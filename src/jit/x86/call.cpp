#include <asmjit/x86.h>

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

	MemAddr GetJitFunc(
		const Signature& signature,
		MemAddr target,
		WaitType waitType,
		[[maybe_unused]] bool hidden
	) {
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
		x86::Compiler cc(&code);
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

		x86::Gp paramImm = cc.new_gpz();
		func->set_arg(0, paramImm);

		x86::Gp returnImm = cc.new_gpz();
		func->set_arg(1, returnImm);

		// paramMem = ((char*)paramImm) + i (char* size walk, uint64_t size r/w)
		x86::Gp i = cc.new_gpz();
		x86::Mem paramMem = ptr(paramImm, i);
		paramMem.set_size(sizeof(uint64_t));

		// i = 0
		cc.mov(i, 0);

		struct ArgRegSlot {
			explicit ArgRegSlot(uint32_t idx) {
				argIdx = idx;
				useHighReg = false;
			}

			Reg low;
			Reg high;
			uint32_t argIdx;
			bool useHighReg;
		};

		std::inplace_vector<ArgRegSlot, Globals::kMaxFuncArgs> argRegSlots;
		size_t offsetNextSlot = sizeof(uint64_t);

		// map argument slots to registers, following abi. (We can have multiple register per arg
		// slot such as high and low 32bits of a 64bit slot)
		for (uint32_t argIdx = 0; argIdx < sig.arg_count(); ++argIdx) {
			const auto& argType = sig.args()[argIdx];

			ArgRegSlot argSlot(argIdx);

			if (TypeUtils::is_int(argType)) {
				argSlot.low = cc.new_gpz();
				cc.mov(argSlot.low.as<x86::Gp>(), paramMem);

				if (HasHiArgSlot(argType)) {
					cc.add(i, sizeof(uint32_t));
					offsetNextSlot -= sizeof(uint32_t);

					argSlot.high = cc.new_gpz();
					argSlot.useHighReg = true;
					cc.mov(argSlot.high.as<x86::Gp>(), paramMem);
				}
			} else if (TypeUtils::is_float(argType)) {
				argSlot.low = cc.new_xmm();
				cc.movq(argSlot.low.as<x86::Vec>(), paramMem);
			} else {
				// ex: void example(__m128i xmmreg) is invalid:
				// https://github.com/asmjit/asmjit/issues/83
				errorCode = "Parameters wider than 64bits not supported";
				return nullptr;
			}

			argRegSlots.push_back(std::move(argSlot));

			// next structure slot (+= sizeof(uint64_t))
			cc.add(i, offsetNextSlot);
			offsetNextSlot = sizeof(uint64_t);
		}

		// allows debuggers to trap
		if (waitType == WaitType::Breakpoint) {
			cc.int3();
		} else if (waitType == WaitType::Wait_Keypress) {
			InvokeNode* invokeNode;
			cc.invoke(Out(invokeNode), reinterpret_cast<uint64_t>(&getchar), FuncSignature::build<int>());
		}

		// Gen the call
		InvokeNode* invokeNode;
		cc.invoke(Out(invokeNode), (uint64_t) target.GetPtr(), sig);

		// Map call params to the args
		for (const auto& argSlot : argRegSlots) {
			invokeNode->set_arg(argSlot.argIdx, 0, argSlot.low);
			if (argSlot.useHighReg) {
				invokeNode->set_arg(argSlot.argIdx, 1, argSlot.high);
			}
		}

		if (sig.has_ret()) {
#if PLUGIFY_ARCH_BITS == 32
			if (TypeUtils::is_between(sig.ret(), TypeId::kInt64, TypeId::kUInt64)) {
				cc.mov(ptr(returnImm), x86::eax);
				cc.mov(ptr(returnImm, sizeof(uint32_t)), x86::edx);
			} else
#endif	// PLUGIFY_ARCH_BITS
				if (TypeUtils::is_int(sig.ret())) {
					x86::Gp tmp = cc.new_gpz();
					invokeNode->set_ret(0, tmp);
					cc.mov(ptr(returnImm), tmp);
				}
#if !PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_ARCH_BITS == 64
				else if (TypeUtils::is_between(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
					cc.mov(ptr(returnImm), x86::rax);
					cc.mov(ptr(returnImm, sizeof(uint64_t)), x86::rdx);
				} else if (TypeUtils::is_between(sig.ret(), TypeId::kFloat32x4, TypeId::kFloat64x2)) {
					cc.movq(ptr(returnImm), x86::xmm0);
					cc.movq(ptr(returnImm, sizeof(uint64_t)), x86::xmm1);
				}
#endif	// PLUGIFY_ARCH_BITS
				else if (TypeUtils::is_float(sig.ret())) {
					x86::Vec ret = cc.new_xmm();
					invokeNode->set_ret(0, ret);
					cc.movq(ptr(returnImm), ret);
				} else {
					// ex: void example(__m128i xmmreg) is invalid:
					// https://github.com/asmjit/asmjit/issues/83
					errorCode = "Return wider than 64bits not supported";
					return nullptr;
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
