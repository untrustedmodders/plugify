#include <asmjit/a64.h>

#include "plugify/call.hpp"

#include "../helpers.hpp"

using namespace plugify;
using namespace asmjit;

static JitRuntime rt;

struct JitCall::Impl {
	Impl()
		: function(nullptr)
		, targetFunc(nullptr) {
	}

	~Impl() {
		if (function) {
			rt.release(function);
		}
	}

	MemAddr GetJitFunc(const Signature& signature, MemAddr target, WaitType waitType, bool hidden) {
		if (function) {
			return function;
		}

		targetFunc = target;

		auto sig = ConvertSignature(signature);

		SimpleErrorHandler eh;
		CodeHolder code;
		code.init(rt.environment(), rt.cpu_features());
		code.set_error_handler(&eh);

		// initialize function
		a64::Compiler cc(&code);
		FuncNode* func = cc.add_func(FuncSignature::build<void, void*, void*>());

#if 0
		StringLogger log;
		auto kFormatFlags = FormatFlags::kMachineCode | FormatFlags::kExplainImms | FormatFlags::kRegCasts | FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kPositions;

		log.addFlags(kFormatFlags);
		code.setLogger(&log);
#endif

#if PLUGIFY_IS_RELEASE
		// too small to really need it
		// func->frame().reset_preserved_fp();
#endif	// PLUGIFY_IS_RELEASE

		a64::Gp paramImm = cc.new_gpz();
		func->set_arg(0, paramImm);

		a64::Gp returnImm = cc.new_gpz();
		func->set_arg(1, returnImm);

		// paramMem = ((char*)paramImm) + i (char* size walk, uint64_t size r/w)
		a64::Gp i = cc.new_gpz();
		a64::Mem paramMem = ptr(paramImm, i);

		// i = 0
		cc.mov(i, 0);

		if (hidden) {
			// load first arg and store its address to ret struct
			a64::Gp tmp = cc.new_gpz();
			cc.ldr(tmp, paramMem);
			cc.str(tmp, ptr(returnImm));

			// next structure slot (+= sizeof(uint64_t))
			cc.add(i, i, sizeof(uint64_t));
		}

		std::inplace_vector<Reg, Globals::kMaxFuncArgs> argRegisters;

		// map argument slots to registers, following abi. (We can have multiple register per arg
		// slot such as high and low 32bits of a 64bit slot)
		for (uint32_t argIdx = 0; argIdx < sig.arg_count(); ++argIdx) {
			const auto& argType = sig.args()[argIdx];

			Reg arg;
			if (TypeUtils::is_int(argType)) {
				arg = cc.new_gp(argType);
				cc.ldr(arg.as<a64::Gp>(), paramMem);
			} else if (TypeUtils::is_float(argType)) {
				arg = cc.new_vec(argType);
				cc.ldr(arg.as<a64::Vec>(), paramMem);
			} else {
				// ex: void example(__m128i xmmreg) is invalid:
				// https://github.com/asmjit/asmjit/issues/83
				errorCode = "Parameters wider than 64bits not supported";
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
			a64::Gp dest = cc.new_gpz();
			cc.mov(dest, reinterpret_cast<uint64_t>(&getchar));
			InvokeNode* invokeNode;
			cc.invoke(Out(invokeNode), dest, FuncSignature::build<int>());
		}

		a64::Gp dest = cc.new_gpz();
		cc.mov(dest, (uint64_t) target.GetPtr());

		if (hidden) {
			a64::Gp tmp = cc.new_gpz();
			cc.ldr(tmp, ptr(returnImm));
			cc.mov(a64::x8, tmp);

			func->frame().add_unavailable_regs(a64::x8);
		}

		// Gen the call
		InvokeNode* invokeNode;
		cc.invoke(Out(invokeNode), dest, sig);

		// Map call params to the args
		for (uint32_t argIdx = 0; argIdx < sig.arg_count(); ++argIdx) {
			invokeNode->set_arg(argIdx, argRegisters.at(argIdx));
		}

		if (sig.has_ret()) {
			if (TypeUtils::is_int(sig.ret())) {
				a64::Gp tmp = cc.new_gp(sig.ret());
				invokeNode->set_ret(0, tmp);
				cc.str(tmp, ptr(returnImm));
			} else if (TypeUtils::is_between(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
				cc.str(a64::x0, ptr(returnImm));
				cc.str(a64::x1, ptr(returnImm, sizeof(uint64_t)));
			} else if (sig.ret() == TypeId::kFloat32x2) {  // Vector2
				cc.str(a64::s0, ptr(returnImm));
				cc.str(a64::s1, ptr(returnImm, sizeof(float)));
			} else if (sig.ret() == TypeId::kFloat64x2) {  // Vector3
				cc.str(a64::s0, ptr(returnImm));
				cc.str(a64::s1, ptr(returnImm, sizeof(float)));
				cc.str(a64::s2, ptr(returnImm, sizeof(float) * 2));
			} else if (sig.ret() == TypeId::kFloat32x4) {  // Vector4
				cc.str(a64::s0, ptr(returnImm));
				cc.str(a64::s1, ptr(returnImm, sizeof(float)));
				cc.str(a64::s2, ptr(returnImm, sizeof(float) * 2));
				cc.str(a64::s3, ptr(returnImm, sizeof(float) * 3));
			} else {
				a64::Vec ret = cc.new_vec(sig.ret());
				invokeNode->set_ret(0, ret);
				cc.str(ret, ptr(returnImm));
			}
		}

		cc.ret();

		// end of the function body
		cc.end_func();

		// write to buffer
		cc.finalize();

		rt.add(&function, &code);

		if (eh.error != Error::kOk) {
			function = nullptr;
			errorCode = eh.code;
			return nullptr;
		}

#if 0
		std::printf("JIT Stub[%p]:\n%s\n", (void*)function, log.data());
#endif

		return function;
	}

	MemAddr function;

	union {
		MemAddr targetFunc;
		const char* errorCode{};
	};
};

using namespace plugify;

JitCall::JitCall()
	: _impl(std::make_unique<Impl>()) {
}

JitCall::JitCall(JitCall&& other) noexcept = default;

JitCall::~JitCall() = default;

JitCall& JitCall::operator=(JitCall&& other) noexcept = default;

MemAddr JitCall::GetJitFunc(const Signature& signature, MemAddr target, WaitType waitType, bool hidden) {
	return _impl->GetJitFunc(signature, target, waitType, hidden);
}

MemAddr JitCall::GetJitFunc(const Method& method, MemAddr target, WaitType waitType, HiddenParam hidden) {
	ValueType retType = method.GetRetType().GetType();
	bool retHidden = hidden(retType);

	Signature signature(
		method.GetCallConv(),
		retHidden ? ValueType::Pointer : retType,
		method.GetVarIndex()
	);
	if (retHidden) {
		signature.AddArg(retType);
	}
	for (const auto& type : method.GetParamTypes()) {
		signature.AddArg(type.IsRef() ? ValueType::Pointer : type.GetType());
	}

	return GetJitFunc(signature, target, waitType, retHidden);
}

MemAddr JitCall::GetFunction() const noexcept {
	return _impl->function;
}

MemAddr JitCall::GetTargetFunc() const noexcept {
	return _impl->targetFunc;
}

std::string_view JitCall::GetError() noexcept {
	return !_impl->function && _impl->errorCode ? _impl->errorCode : "";
}
