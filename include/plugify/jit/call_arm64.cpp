#include <asmjit/a64.h>
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

MemAddr JitCall::GetJitFunc(const FuncSignature& sig, MemAddr target, WaitType waitType, bool hidden) {
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
	a64::Compiler cc(&code);
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
#endif // PLUGIFY_IS_RELEASE

	a64::Gp paramImm = cc.newGpx();
	func->setArg(0, paramImm);

	a64::Gp returnImm = cc.newGpx();
	func->setArg(1, returnImm);

	// paramMem = ((char*)paramImm) + i (char* size walk, uint64_t size r/w)
	a64::Gp i = cc.newGpx();
	a64::Mem paramMem = ptr(paramImm, i);

	// i = 0
	cc.mov(i, 0);

	if (hidden) {
		// load first arg and store its address to ret struct
		a64::Gp tmp = cc.newGpx();
		cc.ldr(tmp, paramMem);
		cc.str(tmp, ptr(returnImm));

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, i, sizeof(uint64_t));
	}

	std::vector<a64::Reg> argRegisters;
	argRegisters.reserve(sig.argCount());

	// map argument slots to registers, following abi. (We can have multiple register per arg slot such as high and low 32bits of a 64bit slot)
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];

		a64::Reg arg;
		if (TypeUtils::isInt(argType)) {
			arg = cc.newGp(argType);
			cc.ldr(arg.as<a64::Gp>(), paramMem);
		} else if (TypeUtils::isFloat(argType)) {
			arg = cc.newVec(argType);
			cc.ldr(arg.as<a64::Vec>(), paramMem);
		} else {
			// ex: void example(__m128i xmmreg) is invalid: https://github.com/asmjit/asmjit/issues/83
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		argRegisters.emplace_back(std::move(arg));

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, i, sizeof(uint64_t));
	}

	// allows debuggers to trap
	if (waitType == WaitType::Breakpoint) {
		cc.brk(0x1);
	} else if (waitType == WaitType::Wait_Keypress) {
		a64::Gp dest = cc.newGpx();
		cc.mov(dest, (uint64_t) &getchar);
		InvokeNode* invokeNode;
		cc.invoke(&invokeNode, dest, FuncSignature::build<int>());
	}

	a64::Gp dest = cc.newGpx();
	cc.mov(dest, (uint64_t) target.GetPtr());

	if (hidden) {
		a64::Gp tmp = cc.newGpx();
		cc.ldr(tmp, ptr(returnImm));
		cc.mov(a64::x8, tmp);

		func->frame().addUnavailableRegs(a64::x8);
	}

	// Gen the call
	InvokeNode* invokeNode;
	cc.invoke(&invokeNode, dest, sig);

	// Map call params to the args
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		invokeNode->setArg(argIdx, argRegisters.at(argIdx));
	}

	if (sig.hasRet()) {
		if (TypeUtils::isInt(sig.ret())) {
			a64::Gp tmp = cc.newGp(sig.ret());
			invokeNode->setRet(0, tmp);
			cc.str(tmp, ptr(returnImm));
		} else if (TypeUtils::isBetween(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
			cc.str(a64::x0, ptr(returnImm));
			cc.str(a64::x1, ptr(returnImm, sizeof(uint64_t)));
		} else if (sig.ret() == TypeId::kFloat32x2) { // Vector2
			cc.str(a64::s0, ptr(returnImm));
			cc.str(a64::s1, ptr(returnImm, sizeof(float)));
		} else if (sig.ret() == TypeId::kFloat64x2) { // Vector3
			cc.str(a64::s0, ptr(returnImm));
			cc.str(a64::s1, ptr(returnImm, sizeof(float)));
			cc.str(a64::s2, ptr(returnImm, sizeof(float) * 2));
		}  else if (sig.ret() == TypeId::kFloat32x4) { // Vector4
			cc.str(a64::s0, ptr(returnImm));
			cc.str(a64::s1, ptr(returnImm, sizeof(float)));
			cc.str(a64::s2, ptr(returnImm, sizeof(float) * 2));
			cc.str(a64::s3, ptr(returnImm, sizeof(float) * 3));
		} else {
			a64::Vec ret = cc.newVec(sig.ret());
			invokeNode->setRet(0, ret);
			cc.str(ret, ptr(returnImm));
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
	FuncSignature sig(JitUtils::GetCallConv(method.GetCallingConvention()), method.GetVarIndex(), JitUtils::GetRetTypeId(retHidden ? ValueType::Void : retType));
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, target, waitType, retHidden);
}
