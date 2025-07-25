#include <asmjit/x86.h>
#include <plugify/jit/callback.hpp>
#include <plugify/jit/helpers.hpp>

using namespace plugify;
using namespace asmjit;

JitCallback::JitCallback(std::weak_ptr<JitRuntime> rt) : _rt{std::move(rt)} {
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

MemAddr JitCallback::GetJitFunc(const FuncSignature& sig, MethodHandle method, CallbackHandler callback, MemAddr data, bool hidden) {
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

	JitUtils::SimpleErrorHandler eh;
	CodeHolder code;
	code.init(rt->environment(), rt->cpuFeatures());
	code.setErrorHandler(&eh);

	// initialize function
	x86::Compiler cc(&code);
	FuncNode* func = cc.addFunc(sig);

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

	struct ArgRegSlot {
		explicit ArgRegSlot(uint32_t idx) {
			argIdx = idx;
			useHighReg = false;
		}

		x86::Reg low;
		x86::Reg high;
		uint32_t argIdx;
		bool useHighReg;
	};

	// map argument slots to registers, following abi.
	std::vector<ArgRegSlot> argRegSlots;
	argRegSlots.reserve(sig.argCount());

	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];

		ArgRegSlot argSlot(argIdx);

		if (TypeUtils::isInt(argType)) {
			argSlot.low = cc.newUIntPtr();

			if (JitUtils::HasHiArgSlot(argType)) {
				argSlot.high = cc.newUIntPtr();
				argSlot.useHighReg = true;
			}

		} else if (TypeUtils::isFloat(argType)) {
			argSlot.low = cc.newXmm();
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		func->setArg(argSlot.argIdx, 0, argSlot.low);
		if (argSlot.useHighReg) {
			func->setArg(argSlot.argIdx, 1, argSlot.high);
		}

		argRegSlots.emplace_back(std::move(argSlot));
	}

	const uint32_t alignment = 16;
    uint32_t offsetNextSlot = sizeof(uint64_t);

	// setup the stack structure to hold arguments for user callback
	const auto stackSize = static_cast<uint32_t>(sizeof(uint64_t) * sig.argCount());
	x86::Mem argsStack;
	if (stackSize > 0) {
		argsStack = cc.newStack(stackSize, alignment);
	}
	x86::Mem argsStackIdx(argsStack);

	// assigns some register as index reg
	x86::Gp i = cc.newUIntPtr();

	// stackIdx <- stack[i].
	argsStackIdx.setIndex(i);

	// r/w are sizeof(uint64_t) width now
	argsStackIdx.setSize(sizeof(uint64_t));

	// set i = 0
	cc.mov(i, 0);

	//// mov from arguments registers into the stack structure
	for (const auto& argSlot : argRegSlots) {
		const auto& argType = sig.args()[argSlot.argIdx];

		// have to cast back to explicit register types to gen right mov type
		if (TypeUtils::isInt(argType)) {
			cc.mov(argsStackIdx, argSlot.low.as<x86::Gp>());

			if (argSlot.useHighReg) {
				cc.add(i, sizeof(uint32_t));
				offsetNextSlot -= sizeof(uint32_t);

				cc.mov(argsStackIdx, argSlot.high.as<x86::Gp>());
			}
		} else if (TypeUtils::isFloat(argType)) {
			cc.movq(argsStackIdx, argSlot.low.as<x86::Xmm>());
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, offsetNextSlot);
		offsetNextSlot = sizeof(uint64_t);
	}

	// fill reg to pass method ptr to callback
	x86::Gp methodPtrParam = cc.newUIntPtr("methodPtrParam");
	cc.mov(methodPtrParam, static_cast<uintptr_t>(method));

	// fill reg to pass data ptr to callback
	x86::Gp dataPtrParam = cc.newUIntPtr("dataPtrParam");
	cc.mov(dataPtrParam, static_cast<uintptr_t>(data));

	// get pointer to stack structure and pass it to the user callback
	x86::Gp argStruct = cc.newUIntPtr("argStruct");
	auto argCount = static_cast<size_t>(sig.argCount());
	if (hidden) {
		// if hidden param, then we need to offset it
		if (--argCount != 0) {
			x86::Mem argsStackIdxNoRet(argsStack);
			argsStackIdxNoRet.setSize(sizeof(uint64_t));
			argsStackIdxNoRet.addOffset(sizeof(uint64_t));
			cc.lea(argStruct, argsStackIdxNoRet);
		}
	} else {
		cc.lea(argStruct, argsStack);
	}

	// fill reg to pass struct arg count to callback
	x86::Gp argCountParam = cc.newUIntPtr("argCountParam");
	cc.mov(argCountParam, argCount);

	// create buffer for ret val
	x86::Mem retStack;
	x86::Gp retStruct = cc.newUIntPtr("retStruct");
	if (hidden) {
		cc.mov(retStruct, argsStack);
	} else {
		const auto retSize = static_cast<uint32_t>(sizeof(uint64_t) * (TypeUtils::isVec128(sig.ret()) ? 2 : 1));
		retStack = cc.newStack(retSize, alignment);
		cc.lea(retStruct, retStack);
	}

	InvokeNode* invokeNode;
	cc.invoke(&invokeNode,
			  (uint64_t) callback,
			  FuncSignature::build<void, void*, void*, Parameters*, size_t, Return*>()
	);

	// call to user provided function (use ABI of host compiler)
	invokeNode->setArg(0, methodPtrParam);
	invokeNode->setArg(1, dataPtrParam);
	invokeNode->setArg(2, argStruct);
	invokeNode->setArg(3, argCountParam);
	invokeNode->setArg(4, retStruct);

	// mov from arguments stack structure into regs
	cc.mov(i, 0); // reset idx
	for (const auto& argSlot : argRegSlots) {
		const auto& argType = sig.args()[argSlot.argIdx];
		if (TypeUtils::isInt(argType)) {
			cc.mov(argSlot.low.as<x86::Gp>(), argsStackIdx);

			if (argSlot.useHighReg) {
				cc.add(i, sizeof(uint32_t));
				offsetNextSlot -= sizeof(uint32_t);

				cc.mov(argSlot.high.as<x86::Gp>(), argsStackIdx);
			}
		} else if (TypeUtils::isFloat(argType)) {
			cc.movq(argSlot.low.as<x86::Xmm>(), argsStackIdx);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uint64_t))
		cc.add(i, offsetNextSlot);
		offsetNextSlot = sizeof(uint64_t);
	}

	if (hidden) {
		cc.ret(retStruct);
	} else if (sig.hasRet()) {
		x86::Mem retStackIdx0(retStack); //-V1007
		retStackIdx0.setSize(sizeof(uint64_t));
#if PLUGIFY_ARCH_BITS == 32
		if (TypeUtils::isBetween(sig.ret(), TypeId::kInt64, TypeId::kUInt64)) {
			x86::Mem retStackIdx1(retStack);
			retStackIdx1.setSize(sizeof(uint64_t));
			retStackIdx1.addOffset(sizeof(uint32_t));

			cc.mov(x86::eax, retStackIdx0);
			cc.mov(x86::edx, retStackIdx1);
			cc.ret();
		}
		else
#endif // PLUGIFY_ARCH_BITS
		if (TypeUtils::isInt(sig.ret())) {
			x86::Gp tmp = cc.newUIntPtr();
			cc.mov(tmp, retStackIdx0);
			cc.ret(tmp);
		}
#if !PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_ARCH_BITS == 64
		else if (TypeUtils::isBetween(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
			x86::Mem retStackIdx1(retStack);
			retStackIdx1.setSize(sizeof(uint64_t));
			retStackIdx1.addOffset(sizeof(uint64_t));

			cc.mov(x86::rax, retStackIdx0);
			cc.mov(x86::rdx, retStackIdx1);
			cc.ret();
		} else if (TypeUtils::isBetween(sig.ret(), TypeId::kFloat32x4, TypeId::kFloat64x2)) {
			x86::Mem retStackIdx1(retStack);
			retStackIdx1.setSize(sizeof(uint64_t));
			retStackIdx1.addOffset(sizeof(uint64_t));

			cc.movq(x86::xmm0, retStackIdx0);
			cc.movq(x86::xmm1, retStackIdx1);
			cc.ret();
		}
#endif // PLUGIFY_ARCH_BITS
		else if (TypeUtils::isFloat(sig.ret())) {
			x86::Xmm tmp = cc.newXmm();
			cc.movq(tmp, retStackIdx0);
			cc.ret(tmp);
		} else {
			// ex: void example(__m128i xmmreg) is invalid: https://github.com/asmjit/asmjit/issues/83
			_errorCode = "Return wider than 64bits not supported";
			return nullptr;
		}
	}

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

MemAddr JitCallback::GetJitFunc(MethodHandle method, CallbackHandler callback, MemAddr data, HiddenParam hidden) {
	ValueType retType = method.GetReturnType().GetType();
	bool retHidden = hidden(retType);
	FuncSignature sig(JitUtils::GetCallConv(method.GetCallingConvention()), method.GetVarIndex(), JitUtils::GetRetTypeId(retHidden ? ValueType::Pointer : retType));
	if (retHidden) {
		sig.addArg(JitUtils::GetValueTypeId(retType));
	}
	for (const auto& type : method.GetParamTypes()) {
		sig.addArg(JitUtils::GetValueTypeId(type.IsReference() ? ValueType::Pointer : type.GetType()));
	}
	return GetJitFunc(sig, method, callback, data, retHidden);
}
