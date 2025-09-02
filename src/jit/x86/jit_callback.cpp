#include "plugify/jit_callback.hpp"
#include "../helpers.hpp"

#include <asmjit/x86.h>

using namespace plugify;
using namespace asmjit;

static JitRuntime rt;

struct JitCallback::Impl {
	Impl() : _function(nullptr), _userData(nullptr) {}

	~Impl() {
		if (_function) {
		    rt.release(_function);
		}
	}

	MemAddr GetJitFunc(const Signature& signature, const Method& method, CallbackHandler callback, MemAddr data, bool hidden) {
		if (_function)
			return _function;

	    _userData = data;

	    auto sig = ConvertSignature(signature);

	    SimpleErrorHandler eh;
	    CodeHolder code;
	    code.init(rt.environment(), rt.cpuFeatures());
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

			    if (HasHiArgSlot(argType)) {
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

		    argRegSlots.push_back(std::move(argSlot));
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
			    cc.movq(argsStackIdx, argSlot.low.as<x86::Vec>());
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
	    cc.mov(methodPtrParam, &method);

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
			    cc.movq(argSlot.low.as<x86::Vec>(), argsStackIdx);
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
			    x86::Vec tmp = cc.newXmm();
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

	    rt.add(&_function, &code);

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

    MemAddr _function;
    union {
        MemAddr _userData;
        const char* _errorCode{};
    };
};

using namespace plugify;

JitCallback::JitCallback() : _impl(std::make_unique<Impl>()) {}

JitCallback::JitCallback(JitCallback&& other) noexcept = default;

JitCallback::~JitCallback() = default;

JitCallback& JitCallback::operator=(JitCallback&& other) noexcept = default;

MemAddr JitCallback::GetJitFunc(const Signature& signature, const Method& method, CallbackHandler callback, MemAddr data, bool hidden) {
    return _impl->GetJitFunc(signature, method, callback, data, hidden);
}

MemAddr JitCallback::GetJitFunc(const Method& method, CallbackHandler callback, MemAddr data, HiddenParam hidden) {
    ValueType retType = method.GetRetType().GetType();
    bool retHidden = hidden(retType);

    Signature signature(method.GetCallConv(), method.GetVarIndex(), retHidden ? ValueType::Pointer : retType);
    if (retHidden) {
        signature.addArg(retType);
    }
    for (const auto& type : method.GetParamTypes()) {
        signature.addArg(type.IsRef() ? ValueType::Pointer : type.GetType());
    }

    return GetJitFunc(signature, method, callback, data, hidden);
}

MemAddr JitCallback::GetFunction() const noexcept {
    return _impl->_function;
}

MemAddr JitCallback::GetUserData() const noexcept {
    return _impl->_userData;
}

std::string_view JitCallback::GetError() noexcept {
    return !_impl->_function && _impl->_errorCode ? _impl->_errorCode : "";
}