#include <asmjit/a64.h>
#include <plugify/jit/call.h>
#include <plugify/jit/helpers.h>

using namespace plugify;

JitCall::JitCall(std::weak_ptr<asmjit::JitRuntime> rt) : _rt{std::move(rt)} {
}

JitCall::JitCall(JitCall&& other) noexcept : _rt{std::move(other._rt)}, _function{other._function}, _targetFunc{other._targetFunc} {
	other._function = nullptr;
	other._targetFunc = nullptr;
}

JitCall::~JitCall() {
	if (_function) {
		if (auto rt = _rt.lock()) {
			rt->release(_function);
		}
	}
}

MemAddr JitCall::GetJitFunc(const asmjit::FuncSignature& sig, MemAddr target, WaitType waitType, bool hidden) {
	if (_function)
		return _function;

	auto rt = _rt.lock();
	if (!rt) {
		_errorCode = "JitRuntime invalid";
		return nullptr;
	}

	_targetFunc = target;

	asmjit::CodeHolder code;
	code.init(rt->environment(), rt->cpuFeatures());

	// initialize function
	asmjit::a64::Compiler cc(&code);
	asmjit::FuncNode* func = cc.addFunc(asmjit::FuncSignature::build<void, Parameters*, Return*>());// Create the wrapper function around call we JIT

	/*StringLogger log;
	auto kFormatFlags = FormatFlags::kMachineCode | FormatFlags::kExplainImms | FormatFlags::kRegCasts | FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kPositions;

	log.addFlags(kFormatFlags);
	code.setLogger(&log);*/

	// too small to really need it
	func->frame().resetPreservedFP();

	asmjit::a64::Gp paramImm = cc.newGpx();
	func->setArg(0, paramImm);

	asmjit::a64::Gp returnImm = cc.newGpx();
	func->setArg(1, returnImm);

	// paramMem = ((char*)paramImm) + i (char* size walk, uint64_t size r/w)
	asmjit::a64::Gp i = cc.newGpx();
	asmjit::a64::Mem paramMem = ptr(paramImm, i);
	paramMem.setSize(sizeof(uint64_t));

	// i = 0
	cc.mov(i, 0);

	std::vector<asmjit::a64::Reg> argRegisters;
	argRegisters.reserve(sig.argCount());

	// map argument slots to registers, following abi. (We can have multiple register per arg slot such as high and low 32bits of a 64bit slot)
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); argIdx++) {
		const auto& argType = sig.args()[argIdx];

		asmjit::a64::Reg arg;
		if (asmjit::TypeUtils::isInt(argType)) {
			arg = cc.newGpx();
			cc.ldr(arg.as<asmjit::a64::Gp>(), paramMem);
		} else if (asmjit::TypeUtils::isFloat(argType)) {
			arg = cc.newVec(argType);
			cc.ldr(arg.as<asmjit::a64::Vec>(), paramMem);
		} else {
			// ex: void example(__m128i xmmreg) is invalid: https://github.com/asmjit/asmjit/issues/83
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		argRegisters.push_back(std::move(arg));

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, i, sizeof(uint64_t));
	}

	// allows debuggers to trap
	if (waitType == WaitType::Breakpoint) {
		cc.brk(0x1);
	} else if (waitType == WaitType::Wait_Keypress) {
		asmjit::InvokeNode* invokeNode;
		cc.invoke(&invokeNode,
				  (uint64_t) &getchar,
				  asmjit::FuncSignature::build<int>()
		);
	}

	// Gen the call
	asmjit::InvokeNode* invokeNode;
	cc.invoke(&invokeNode,
			(uint64_t) target.GetPtr(),
			sig
	);

	if (hidden) {
		cc.mov(asmjit::a64::x8, returnImm);
	}

	// Map call params to the args
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		invokeNode->setArg(argIdx, argRegisters.at(argIdx));
	}

	if (sig.hasRet()) {
		if (asmjit::TypeUtils::isInt(sig.ret())) {
			asmjit::a64::Gp tmp = cc.newGpx();
			invokeNode->setRet(0, tmp);
			cc.str(tmp, ptr(returnImm));
		} else if (asmjit::TypeUtils::isBetween(sig.ret(), asmjit::TypeId::kInt8x16, asmjit::TypeId::kUInt64x2)) {
			cc.str(asmjit::a64::x0, ptr(returnImm));
			cc.str(asmjit::a64::x1, ptr(returnImm, sizeof(uint64_t)));
		} else if (sig.ret() == asmjit::TypeId::kFloat32x2) { // Vector2
			cc.str(asmjit::a64::s0, ptr(returnImm));
			cc.str(asmjit::a64::s1, ptr(returnImm, sizeof(float)));
		} else if (sig.ret() == asmjit::TypeId::kFloat64x2) { // Vector3
			cc.str(asmjit::a64::s0, ptr(returnImm));
			cc.str(asmjit::a64::s1, ptr(returnImm, sizeof(float)));
			cc.str(asmjit::a64::s2, ptr(returnImm, sizeof(float) * 2));
		}  else if (sig.ret() == asmjit::TypeId::kFloat32x4) { // Vector4
			cc.str(asmjit::a64::s0, ptr(returnImm));
			cc.str(asmjit::a64::s1, ptr(returnImm, sizeof(float)));
			cc.str(asmjit::a64::s2, ptr(returnImm, sizeof(float) * 2));
			cc.str(asmjit::a64::s3, ptr(returnImm, sizeof(float) * 3));
		} else {
			asmjit::a64::Vec ret = cc.newVec(sig.ret());
			invokeNode->setRet(0, ret);
			cc.str(ret, ptr(returnImm));
		}
	}

	//cc.ret();

	// end of the function body
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

MemAddr JitCall::GetJitFunc(MethodRef method, MemAddr target, WaitType waitType, HiddenParam hidden) {
	ValueType retType = method.GetReturnType().GetType();
	bool retHidden = hidden(retType);
	asmjit::FuncSignature sig(JitUtils::GetCallConv(method.GetCallingConvention()), method.GetVarIndex(), JitUtils::GetRetTypeId(retHidden ? ValueType::Void : retType));
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, target, waitType, retHidden);
}
