#include <asmjit/a64.h>
#include <plugify/jit/callback.h>
#include <plugify/jit/helpers.h>

using namespace plugify;

JitCallback::JitCallback(std::weak_ptr<asmjit::JitRuntime> rt) : _rt{std::move(rt)} {
}

JitCallback::JitCallback(JitCallback&& other) noexcept : _rt{std::move(other._rt)}, _function{other._function}, _userData{other._userData} {
	other._function = nullptr;
	other._userData = nullptr;
}

JitCallback::~JitCallback() {
	if (_function) {
		if (auto rt = _rt.lock()) {
			rt->release(_function);
		}
	}
}

MemAddr JitCallback::GetJitFunc(const asmjit::FuncSignature& sig, MethodRef method, CallbackHandler callback, MemAddr data) {
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
	asmjit::a64::Compiler cc(&code);
	asmjit::FuncNode* func = cc.addFunc(sig);

	/*StringLogger log;
	auto kFormatFlags = FormatFlags::kMachineCode | FormatFlags::kExplainImms | FormatFlags::kRegCasts | FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kPositions;

	log.addFlags(kFormatFlags);
	code.setLogger(&log);*/

	// too small to really need it
	func->frame().resetPreservedFP();

	// map argument slots to registers, following abi.
	std::vector<asmjit::a64::Reg> argRegisters;
	argRegisters.reserve(sig.argCount());

	for (uint32_t argIdx = 0; argIdx < sig.argCount(); argIdx++) {
		const auto& argType = sig.args()[argIdx];

		asmjit::a64::Reg arg;
		if (asmjit::TypeUtils::isInt(argType)) {
			arg = cc.newGpx();
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			arg = cc.newVec(argType);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		func->setArg(argIdx, arg);
		argRegisters.push_back(std::move(arg));
	}

	const uint32_t alignment = 16;

	// setup the stack structure to hold arguments for user callback
	auto stackSize = static_cast<uint32_t>(sizeof(int64_t) * sig.argCount());
	asmjit::a64::Mem argsStack = cc.newStack(stackSize, alignment);
	asmjit::a64::Mem argsStackIdx(argsStack);

	// assigns some register as index reg
	asmjit::a64::Gp i = cc.newGpx();

	// stackIdx <- stack[i].
	argsStackIdx.setIndex(i);

	// r/w are sizeof(int64_t) width now
	argsStackIdx.setSize(sizeof(int64_t));

	// set i = 0
	cc.mov(i, 0);

	//// mov from arguments registers into the stack structure
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];

		// have to cast back to explicit register types to gen right mov type
		if (asmjit::TypeUtils::isInt(argType)) {
			cc.str(argRegisters.at(argIdx).as<asmjit::a64::Gp>(), argsStackIdx);
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			cc.str(argRegisters.at(argIdx).as<asmjit::a64::Vec>(), argsStackIdx);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(int64_t))
		cc.add(i, i, sizeof(int64_t));
	}

	union {
		MethodRef method;
		int64_t ptr;
	} cast{ method };

	// fill reg to pass method ptr to callback
	asmjit::a64::Gp methodPtrParam = cc.newGpx("methodPtrParam");
	cc.mov(methodPtrParam, cast.ptr);

	// fill reg to pass data ptr to callback
	asmjit::a64::Gp dataPtrParam = cc.newGpx("dataPtrParam");
	cc.mov(dataPtrParam, data.CCast<int64_t>());

	// get pointer to stack structure and pass it to the user callback
	asmjit::a64::Gp argStruct = cc.newGpx("argStruct");
	cc.lea(argStruct, argsStack);

	// fill reg to pass struct arg count to callback
	asmjit::a64::Gp argCountParam = cc.newGp(asmjit::TypeId::kUInt8, "argCountParam");
	cc.mov(argCountParam, static_cast<uint8_t>(sig.argCount()));

	bool isPod = asmjit::TypeUtils::isVec128(sig.ret());
	//bool isIntPod = asmjit::TypeUtils::isBetween(sig.ret(), asmjit::TypeId::kInt8x16, asmjit::TypeId::kUInt64x2);
	//bool isFloatPod = asmjit::TypeUtils::isBetween(sig.ret(), asmjit::TypeId::kFloat32x4, asmjit::TypeId::kFloat64x2);
	auto retSize = static_cast<uint32_t>(sizeof(int64_t) * (isPod ? 2 : 1));

	// create buffer for ret val
	asmjit::a64::Mem retStack = cc.newStack(retSize, alignment);
	asmjit::a64::Gp retStruct = cc.newGpx("retStruct");
	cc.lea(retStruct, retStack);

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
			cc.ldr(argRegisters.at(argIdx).as<asmjit::a64::Gp>(), argsStackIdx);
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			cc.ldr(argRegisters.at(argIdx).as<asmjit::a64::Vec>(), argsStackIdx);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(int64_t))
		cc.add(i, i, sizeof(int64_t));
	}

	if (sig.hasRet()) {
		asmjit::a64::Mem retStackIdx(retStack);
		retStackIdx.setSize(sizeof(int64_t));
		if (asmjit::TypeUtils::isInt(sig.ret())) {
			asmjit::a64::Gp tmp = cc.newGpx();
			cc.ldr(tmp, retStackIdx);
			cc.ret(tmp);
		} else {
			asmjit::a64::Vec tmp = cc.newVec(sig.ret());
			cc.ldr(tmp, retStackIdx);
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
	bool isHiddenParam = hidden(retType);
	asmjit::FuncSignature sig(JitUtils::GetCallConv(method.GetCallingConvention()), method.GetVarIndex(), JitUtils::GetRetTypeId(isHiddenParam ? ValueType::Pointer : retType));
	if (isHiddenParam) {
		sig.addArg(JitUtils::GetValueTypeId(retType));
	}
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, method, callback, data);
}