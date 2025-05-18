#include <plugify/jit/call.hpp>
#include <plugify/jit/helpers.hpp>

using namespace plugify;
using namespace asmjit;

JitCall::JitCall(std::weak_ptr<JitRuntime> rt) : _rt{std::move(rt)} {
}

JitCall::JitCall(JitCall&& other) noexcept {
	*this = std::move(other);
}

JitCall::~JitCall() {
	if (_function) {
		if (auto rt = _rt.lock()) {
			rt->release(_function);
		}
	}
}

JitCall& JitCall::operator=(JitCall&& other) noexcept {
	_rt = std::move(other._rt);
	_function = std::exchange(other._function, nullptr);
	_targetFunc = std::exchange(other._targetFunc, nullptr);
	return *this;
}

MemAddr JitCall::GetJitFunc(const FuncSignature& sig, MemAddr target, WaitType waitType, bool) {
	if (_function)
		return _function;

	auto rt = _rt.lock();
	if (!rt) {
		_errorCode = "JitRuntime invalid";
		return nullptr;
	}

	_targetFunc = target;

	JitUtils::SimpleErrorHandler eh;
	CodeHolder code;
	code.init(rt->environment(), rt->cpuFeatures());
	code.setErrorHandler(&eh);

	// initialize function
	x86::Compiler cc(&code);
	FuncNode* func = cc.addFunc(FuncSignature::build<void, Parameters*, Return*>());// Create the wrapper function around call we JIT

#if 0
	StringLogger log;
	auto kFormatFlags = FormatFlags::kMachineCode | FormatFlags::kExplainImms | FormatFlags::kRegCasts | FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kPositions;

	log.addFlags(kFormatFlags);
	code.setLogger(&log);
#endif

#if PLUGIFY_IS_RELEASE
	// too small to really need it
	func->frame().resetPreservedFP();
#endif

	x86::Gp paramImm = cc.newUIntPtr();
	func->setArg(0, paramImm);

	x86::Gp returnImm = cc.newUIntPtr();
	func->setArg(1, returnImm);

	// paramMem = ((char*)paramImm) + i (char* size walk, uint64_t size r/w)
	x86::Gp i = cc.newUIntPtr();
	x86::Mem paramMem = ptr(paramImm, i);
	paramMem.setSize(sizeof(uint64_t));

	// i = 0
	cc.mov(i, 0);

	std::vector<x86::Reg> argRegisters;
	argRegisters.reserve(sig.argCount());

	// map argument slots to registers, following abi. (We can have multiple register per arg slot such as high and low 32bits of a 64bit slot)
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];

		x86::Reg arg;
		if (TypeUtils::isInt(argType)) {
			arg = cc.newUIntPtr();
			cc.mov(arg.as<x86::Gp>(), paramMem);
		} else if (TypeUtils::isFloat(argType)) {
			arg = cc.newXmm();
			cc.movq(arg.as<x86::Xmm>(), paramMem);
		} else {
			// ex: void example(__m128i xmmreg) is invalid: https://github.com/asmjit/asmjit/issues/83
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		argRegisters.emplace_back(std::move(arg));

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, sizeof(uint64_t));
	}

	// allows debuggers to trap
	if (waitType == WaitType::Breakpoint) {
		cc.int3();
	} else if (waitType == WaitType::Wait_Keypress) {
		InvokeNode* invokeNode;
		cc.invoke(&invokeNode,
				(uint64_t) &getchar,
				FuncSignature::build<int>()
		);
	}

	// Gen the call
	InvokeNode* invokeNode;
	cc.invoke(&invokeNode,
			(uint64_t) target.GetPtr(),
			sig
	);

	// Map call params to the args
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		invokeNode->setArg(argIdx, argRegisters.at(argIdx));
	}

	if (sig.hasRet()) {
		if (TypeUtils::isInt(sig.ret())) {
			x86::Gp tmp = cc.newUIntPtr();
			invokeNode->setRet(0, tmp);
			cc.mov(ptr(returnImm), tmp);
		}
#if !PLUGIFY_PLATFORM_WINDOWS
		else if (TypeUtils::isBetween(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
			cc.mov(ptr(returnImm), x86::rax);
			cc.mov(ptr(returnImm, sizeof(uint64_t)), x86::rdx);

		} else if (TypeUtils::isBetween(sig.ret(), TypeId::kFloat32x4, TypeId::kFloat64x2)) {
			cc.movq(ptr(returnImm), x86::xmm0);
			cc.movq(ptr(returnImm, sizeof(uint64_t)), x86::xmm1);
		}
#endif
		else {
			x86::Xmm ret = cc.newXmm();
			invokeNode->setRet(0, ret);
			cc.movq(ptr(returnImm), ret);
		}
	}

	cc.ret();

	// end of the function body
	cc.endFunc();

	// write to buffer
	cc.finalize();

	rt->add(&_function, &code);

	if (eh.error) {
		_function = nullptr;
		_errorCode = eh.code;
		return nullptr;
	}

#if 0
	std::printf("JIT Stub[%p]:\n%s\n", (void*)_function, log.data());
#endif

	return _function;
}

MemAddr JitCall::GetJitFunc(MethodHandle method, MemAddr target, WaitType waitType, HiddenParam hidden) {
	ValueType retType = method.GetReturnType().GetType();
	bool retHidden = hidden(retType);
	FuncSignature sig(JitUtils::GetCallConv(method.GetCallingConvention()), method.GetVarIndex(), JitUtils::GetRetTypeId(retHidden ? ValueType::Pointer : retType));
	if (retHidden) {
		sig.addArg(JitUtils::GetValueTypeId(retType));
	}
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, target, waitType, retHidden);
}
