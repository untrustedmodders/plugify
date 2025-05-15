#include <asmjit/a64.h>
#include <plugify/jit/callback.hpp>
#include <plugify/jit/helpers.hpp>

using namespace plugify;

JitCallback::JitCallback(std::weak_ptr<asmjit::JitRuntime> rt) : _rt{std::move(rt)} {
}

JitCallback::JitCallback(JitCallback&& other) noexcept {
	*this = std::move(other);
}

JitCallback::~JitCallback() {
	if (_function) {
		if (auto rt = _rt.lock()) {
			rt->release(_function);
		}
	}
}

JitCallback& JitCallback::operator=(JitCallback&& other) noexcept {
	_rt = std::move(other._rt);
	_function = std::exchange(other._function, nullptr);
	_userData = std::exchange(other._userData, nullptr);
	return *this;
}

MemAddr JitCallback::GetJitFunc(const asmjit::FuncSignature& sig, MethodHandle method, CallbackHandler callback, MemAddr data, bool hidden) {
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

#if PLUGIFY_IS_RELEASE
	// too small to really need it
	func->frame().resetPreservedFP();
#endif

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
		argRegisters.emplace_back(std::move(arg));
	}

	const uint32_t alignment = 16;

	// setup the stack structure to hold arguments for user callback
	const auto stackSize = static_cast<uint32_t>(sizeof(uint64_t) * sig.argCount());
	asmjit::a64::Mem argsStack = cc.newStack(stackSize, alignment);
	asmjit::a64::Mem argsStackIdx(argsStack);

	// assigns some register as index reg
	asmjit::a64::Gp i = cc.newGpx();

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
			cc.str(argRegisters.at(argIdx).as<asmjit::a64::Gp>(), argsStackIdx);
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			cc.str(argRegisters.at(argIdx).as<asmjit::a64::Vec>(), argsStackIdx);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, i, sizeof(uint64_t));
	}

	// fill reg to pass method ptr to callback
	asmjit::a64::Gp methodPtrParam = cc.newGpx("methodPtrParam");
	cc.mov(methodPtrParam, static_cast<uintptr_t>(method));

	// fill reg to pass data ptr to callback
	asmjit::a64::Gp dataPtrParam = cc.newGpx("dataPtrParam");
	cc.mov(dataPtrParam, static_cast<uintptr_t>(data));

	// get pointer to stack structure and pass it to the user callback
	asmjit::a64::Gp argStruct = cc.newGpx("argStruct");
	cc.add(argStruct, asmjit::a64::sp, argsStack.offset());

	// fill reg to pass struct arg count to callback
	asmjit::a64::Gp argCountParam = cc.newGpx("argCountParam");
	cc.mov(argCountParam, static_cast<size_t>(sig.argCount()));

	// create buffer for ret val
	std::optional<asmjit::a64::Mem> retStack;
	asmjit::a64::Gp retStruct = cc.newGpx("retStruct");
	if (hidden) {
		cc.mov(retStruct, asmjit::a64::x8);
	} else {
		const auto retSize = static_cast<uint32_t>(sizeof(uint64_t) * (asmjit::TypeUtils::isVec128(sig.ret()) ? 2 : 1));
		retStack = cc.newStack(retSize, alignment);
		cc.add(retStruct, asmjit::a64::sp, retStack->offset());
	}

	asmjit::InvokeNode* invokeNode;
	cc.invoke(&invokeNode,
			  (uint64_t) callback,
			  asmjit::FuncSignature::build<void, void*, void*, Parameters*, size_t, Return*>()
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

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, i, sizeof(uint64_t));
	}

	if (hidden) {
		cc.mov(asmjit::a64::x8, retStruct);
		cc.ret();
	} else if (sig.hasRet()) {
		asmjit::a64::Mem retStackIdx0(*retStack);
		retStackIdx0.setSize(sizeof(uint64_t));
		if (asmjit::TypeUtils::isInt(sig.ret())) {
			asmjit::a64::Gp tmp = cc.newGpx();
			cc.ldr(tmp, retStackIdx0);
			cc.ret(tmp);
		} else if (asmjit::TypeUtils::isBetween(sig.ret(), asmjit::TypeId::kInt8x16, asmjit::TypeId::kUInt64x2)) {
			asmjit::a64::Mem retStackIdx1(*retStack);
			retStackIdx1.setSize(sizeof(uint64_t));
			retStackIdx1.addOffset(sizeof(uint64_t));
			cc.ldr(asmjit::a64::x0, retStackIdx0);
			cc.ldr(asmjit::a64::x1, retStackIdx1);
			cc.ret();
		} else if (sig.ret() == asmjit::TypeId::kFloat32x2) { // Vector2
			retStackIdx0.setSize(sizeof(float));
			asmjit::a64::Mem retStackIdx1(*retStack);
			retStackIdx1.setSize(sizeof(float));
			retStackIdx1.addOffset(sizeof(float));
			cc.ldr(asmjit::a64::s0, retStackIdx0);
			cc.ldr(asmjit::a64::s1, retStackIdx1);
			cc.ret();
		} else if (sig.ret() == asmjit::TypeId::kFloat64x2) { // Vector3
			retStackIdx0.setSize(sizeof(float));
			asmjit::a64::Mem retStackIdx1(*retStack);
			retStackIdx1.setSize(sizeof(float));
			retStackIdx1.addOffset(sizeof(float));
			asmjit::a64::Mem retStackIdx2(*retStack);
			retStackIdx2.setSize(sizeof(float));
			retStackIdx2.addOffset(sizeof(float) * 2);
			cc.ldr(asmjit::a64::s0, retStackIdx0);
			cc.ldr(asmjit::a64::s1, retStackIdx1);
			cc.ldr(asmjit::a64::s2, retStackIdx2);
			cc.ret();
		} else if (sig.ret() == asmjit::TypeId::kFloat32x4) { // Vector4
			retStackIdx0.setSize(sizeof(float));
			asmjit::a64::Mem retStackIdx1(*retStack);
			retStackIdx1.setSize(sizeof(float));
			retStackIdx1.addOffset(sizeof(float));
			asmjit::a64::Mem retStackIdx2(*retStack);
			retStackIdx2.setSize(sizeof(float));
			retStackIdx2.addOffset(sizeof(float) * 2);
			asmjit::a64::Mem retStackIdx3(*retStack);
			retStackIdx3.setSize(sizeof(float));
			retStackIdx3.addOffset(sizeof(float) * 3);
			cc.ldr(asmjit::a64::s0, retStackIdx0);
			cc.ldr(asmjit::a64::s1, retStackIdx1);
			cc.ldr(asmjit::a64::s2, retStackIdx2);
			cc.ldr(asmjit::a64::s3, retStackIdx3);
			cc.ret();
		} else {
			asmjit::a64::Vec tmp = cc.newVec(sig.ret());
			cc.ldr(tmp, retStackIdx0);
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

MemAddr JitCallback::GetJitFunc(MethodHandle method, CallbackHandler callback, MemAddr data, HiddenParam hidden) {
	ValueType retType = method.GetReturnType().GetType();
	bool retHidden = hidden(retType);
	asmjit::FuncSignature sig(asmjit::CallConvId::kHost, method.GetVarIndex(), JitUtils::GetRetTypeId(retHidden ? ValueType::Void : retType));
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, method, callback, data, retHidden);
}
