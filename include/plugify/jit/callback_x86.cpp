#include <plugify/jit/callback.hpp>
#include <plugify/jit/helpers.hpp>

using namespace plugify;

JitCallback::JitCallback(std::weak_ptr<asmjit::JitRuntime> rt) : _rt{std::move(rt)} {
}

JitCallback::JitCallback(JitCallback&& other) noexcept
	: _rt{std::move(other._rt)},
	  _function{std::exchange(other._function, nullptr)},
	  _userData{std::exchange(other._userData, nullptr)} {
}

JitCallback::~JitCallback() {
	if (_function) {
		if (auto rt = _rt.lock()) {
			rt->release(_function);
		}
	}
}

MemAddr JitCallback::GetJitFunc(const asmjit::FuncSignature& sig, MethodRef method, CallbackHandler callback, MemAddr data, bool hidden) {
	if (_function) 
		return _function;

	auto rt = _rt.lock();
	if (!rt) {
		_errorCode = "JitRuntime invalid";
		return nullptr;
	}

	_userData = data;

	/*
	  AsmJit is smart enough to track register allocations and will forward
	  the proper registers the right values and fixup any it dirtied earlier.
	  This can only be done if it knows the signature, and ABI, so we give it
	  them. It also only does this mapping for calls, so we need to generate
	  calls on our boundaries of transfers when we want argument order correct
	  (ABI stuff is managed for us when calling C code within this project via host mode).
	  It also does stack operations for us including alignment, shadow space, and
	  arguments, everything really. Manual stack push/pop is not supported using
	  the AsmJit compiler, so we must create those nodes, and insert them into
	  the Node list manually to not corrupt the compiler's tracking of things.

	  Inside the compiler, before endFunc only virtual registers may be used. Any
	  concrete physical registers will not have their liveness tracked, so will
	  be spoiled and must be manually marked dirty. After endFunc ONLY concrete
	  physical registers may be inserted as nodes.
	*/

	asmjit::CodeHolder code;
	code.init(rt->environment(), rt->cpuFeatures());

	// initialize function
	asmjit::x86::Compiler cc(&code);
	asmjit::FuncNode* func = cc.addFunc(sig);

	/*StringLogger log;
	auto kFormatFlags = FormatFlags::kMachineCode | FormatFlags::kExplainImms | FormatFlags::kRegCasts | FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kPositions;

	log.addFlags(kFormatFlags);
	code.setLogger(&log);*/

	// too small to really need it
	func->frame().resetPreservedFP();

	// map argument slots to registers, following abi.
	std::vector<asmjit::x86::Reg> argRegisters;
	argRegisters.reserve(sig.argCount());

	for (uint32_t argIdx = 0; argIdx < sig.argCount(); argIdx++) {
		const auto& argType = sig.args()[argIdx];

		asmjit::x86::Reg arg;
		if (asmjit::TypeUtils::isInt(argType)) {
			arg = cc.newUIntPtr();
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			arg = cc.newXmm();
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		func->setArg(argIdx, arg);
		argRegisters.push_back(std::move(arg));
	}

	const uint32_t alignment = 16;

	// setup the stack structure to hold arguments for user callback
	const auto stackSize = static_cast<uint32_t>(sizeof(uint64_t) * sig.argCount());
	asmjit::x86::Mem argsStack = cc.newStack(stackSize, alignment);
	asmjit::x86::Mem argsStackIdx(argsStack);

	// assigns some register as index reg
	asmjit::x86::Gp i = cc.newUIntPtr();

	// stackIdx <- stack[i].
	argsStackIdx.setIndex(i);

	// r/w are sizeof(uint64_t) width now
	argsStackIdx.setSize(sizeof(uint64_t));

	// set i = 0
	cc.mov(i, 0);

	//// mov from arguments registers into the stack structure
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];

		// have to cast back to explicit register types to gen right mov type
		if (asmjit::TypeUtils::isInt(argType)) {
			cc.mov(argsStackIdx, argRegisters.at(argIdx).as<asmjit::x86::Gp>());
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			cc.movq(argsStackIdx, argRegisters.at(argIdx).as<asmjit::x86::Xmm>());
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, sizeof(uint64_t));
	}

	union {
		MethodRef method;
		uintptr_t ptr;
	} cast{ method };

	// fill reg to pass method ptr to callback
	asmjit::x86::Gp methodPtrParam = cc.newUIntPtr("methodPtrParam");
	cc.mov(methodPtrParam, cast.ptr);

	// fill reg to pass data ptr to callback
	asmjit::x86::Gp dataPtrParam = cc.newUIntPtr("dataPtrParam");
	cc.mov(dataPtrParam, data.CCast<uintptr_t>());

	// get pointer to stack structure and pass it to the user callback
	asmjit::x86::Gp argStruct = cc.newUIntPtr("argStruct");
	auto argCount = static_cast<uint8_t>(sig.argCount());
	if (hidden) {
		// if hidden param, then we need to offset it
		if (--argCount != 0) {
			asmjit::x86::Mem argsStackIdxNoRet(argsStack);
			argsStackIdxNoRet.setSize(sizeof(uint64_t));
			argsStackIdxNoRet.addOffset(sizeof(uint64_t));
			cc.lea(argStruct, argsStackIdxNoRet);
		}
	} else {
		cc.lea(argStruct, argsStack);
	}

	// fill reg to pass struct arg count to callback
	asmjit::x86::Gp argCountParam = cc.newUInt8("argCountParam");
	cc.mov(argCountParam, argCount);

	// create buffer for ret val
	std::optional<asmjit::x86::Mem> retStack;
	asmjit::x86::Gp retStruct = cc.newUIntPtr("retStruct");
	if (hidden) {
		cc.mov(retStruct, argsStack);
	} else {
		const auto retSize = static_cast<uint32_t>(sizeof(uint64_t) * (asmjit::TypeUtils::isVec128(sig.ret()) ? 2 : 1));
		retStack = cc.newStack(retSize, alignment);
		cc.lea(retStruct, *retStack);
	}

	asmjit::InvokeNode* invokeNode;
	cc.invoke(&invokeNode,
			  (uint64_t) callback,
			  asmjit::FuncSignature::build<void, void*, void*, Parameters*, uint8_t, Return*>()
	);

	// call to user provided function (use ABI of host compiler)
	invokeNode->setArg(0, methodPtrParam);
	invokeNode->setArg(1, dataPtrParam);
	invokeNode->setArg(2, argStruct);
	invokeNode->setArg(3, argCountParam);
	invokeNode->setArg(4, retStruct);

	// mov from arguments stack structure into regs
	cc.mov(i, 0); // reset idx
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];
		if (asmjit::TypeUtils::isInt(argType)) {
			cc.mov(argRegisters.at(argIdx).as<asmjit::x86::Gp>(), argsStackIdx);
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			cc.movq(argRegisters.at(argIdx).as<asmjit::x86::Xmm>(), argsStackIdx);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, sizeof(uint64_t));
	}

	if (hidden) {
		cc.ret(retStruct);
	} else if (sig.hasRet()) {
		asmjit::x86::Mem retStackIdx0(*retStack);
		retStackIdx0.setSize(sizeof(uint64_t));
		if (asmjit::TypeUtils::isInt(sig.ret())) {
			asmjit::x86::Gp tmp = cc.newUIntPtr();
			cc.mov(tmp, retStackIdx0);
			cc.ret(tmp);
		}
#if !PLUGIFY_PLATFORM_WINDOWS
		else if (asmjit::TypeUtils::isBetween(sig.ret(), asmjit::TypeId::kInt8x16, asmjit::TypeId::kUInt64x2)) {
			asmjit::x86::Mem retStackIdx1(*retStack);
			retStackIdx1.setSize(sizeof(uint64_t));
			retStackIdx1.addOffset(sizeof(uint64_t));

			cc.mov(asmjit::x86::rax, retStackIdx0);
			cc.mov(asmjit::x86::rdx, retStackIdx1);
			cc.ret();
		} else if (asmjit::TypeUtils::isBetween(sig.ret(), asmjit::TypeId::kFloat32x4, asmjit::TypeId::kFloat64x2)) {
			asmjit::x86::Mem retStackIdx1(*retStack);
			retStackIdx1.setSize(sizeof(uint64_t));
			retStackIdx1.addOffset(sizeof(uint64_t));

			cc.movq(asmjit::x86::xmm0, retStackIdx0);
			cc.movq(asmjit::x86::xmm1, retStackIdx1);
			cc.ret();
		}
#endif
		else {
			asmjit::x86::Xmm tmp = cc.newXmm();
			cc.movq(tmp, retStackIdx0);
			cc.ret(tmp);
		}
	}

	cc.endFunc();

	// write to buffer
	cc.finalize();

	asmjit::Error err = rt->add(&_function, &code);
	if (err) {
		_function = nullptr;
		_errorCode = asmjit::DebugUtils::errorAsString(err);
		return nullptr;
	}

	//PL_LOG_VERBOSE("JIT Stub:\n{}", log.data());

	return _function;
}

MemAddr JitCallback::GetJitFunc(MethodRef method, CallbackHandler callback, MemAddr data, HiddenParam hidden) {
	ValueType retType = method.GetReturnType().GetType();
	bool retHidden = hidden(retType);
	asmjit::FuncSignature sig(JitUtils::GetCallConv(method.GetCallingConvention()), method.GetVarIndex(), JitUtils::GetRetTypeId(retHidden ? ValueType::Pointer : retType));
	if (retHidden) {
		sig.addArg(JitUtils::GetValueTypeId(retType));
	}
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, method, callback, data, retHidden);
}
