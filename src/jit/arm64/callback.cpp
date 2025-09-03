#include "plugify/callback.hpp"
#include "../helpers.hpp"

#include <asmjit/a64.h>

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

	MemAddr GetJitFunc(const Signature& signature, const Method* method, CallbackHandler callback, MemAddr data, bool hidden) {
		if (_function)
			return _function;

	    _userData = data;

	    auto sig = ConvertSignature(signature);

	    SimpleErrorHandler eh;
	    CodeHolder code;
	    code.init(rt.environment(), rt.cpuFeatures());
	    code.setErrorHandler(&eh);

	    // initialize function
	    a64::Compiler cc(&code);
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
    #endif // PLUGIFY_IS_RELEASE

	    // map argument slots to registers, following abi.
	    std::vector<a64::Reg> argRegisters;
	    argRegisters.reserve(sig.argCount());

	    for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		    const auto& argType = sig.args()[argIdx];

		    a64::Reg arg;
		    if (TypeUtils::isInt(argType)) {
			    arg = cc.newGp(argType);
		    } else if (TypeUtils::isFloat(argType)) {
			    arg = cc.newVec(argType);
		    } else {
			    _errorCode = "Parameters wider than 64bits not supported";
			    return nullptr;
		    }

		    func->setArg(argIdx, arg);
		    argRegisters.push_back(std::move(arg));
	    }

	    a64::Gp retStruct = cc.newGpx("retStruct");

	    // store x8 in advance
	    if (hidden) {
		    cc.mov(retStruct, a64::x8);
	    }

	    const uint32_t alignment = 16;

	    // setup the stack structure to hold arguments for user callback
	    const auto stackSize = static_cast<uint32_t>(sizeof(uint64_t) * sig.argCount());
	    a64::Mem argsStack;
	    if (stackSize > 0) {
		    argsStack = cc.newStack(stackSize, alignment);
	    }
	    a64::Mem argsStackIdx(argsStack);

	    // assigns some register as index reg
	    a64::Gp i = cc.newGpx();

	    // stackIdx <- stack[i].
	    argsStackIdx.setIndex(i);

	    // set i = 0
	    cc.mov(i, 0);

	    //// mov from arguments registers into the stack structure
	    for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		    const auto& argType = sig.args()[argIdx];

		    // have to cast back to explicit register types to gen right mov type
		    if (TypeUtils::isInt(argType)) {
			    cc.str(argRegisters.at(argIdx).as<a64::Gp>(), argsStackIdx);
		    } else if (TypeUtils::isFloat(argType)) {
			    cc.str(argRegisters.at(argIdx).as<a64::Vec>(), argsStackIdx);
		    } else {
			    _errorCode = "Parameters wider than 64bits not supported";
			    return nullptr;
		    }

		    // next structure slot (+= sizeof(uint64_t))
		    cc.add(i, i, sizeof(uint64_t));
	    }

	    // fill reg to pass method ptr to callback
	    a64::Gp methodPtrParam = cc.newGpx("methodPtrParam");
	    cc.mov(methodPtrParam, method);

	    // fill reg to pass data ptr to callback
	    a64::Gp dataPtrParam = cc.newGpx("dataPtrParam");
	    cc.mov(dataPtrParam, static_cast<uintptr_t>(data));

	    // get pointer to stack structure and pass it to the user callback
	    a64::Gp argStruct = cc.newGpx("argStruct");
	    cc.loadAddressOf(argStruct, argsStack);

	    // fill reg to pass struct arg count to callback
	    a64::Gp argCountParam = cc.newGpx("argCountParam");
	    cc.mov(argCountParam, static_cast<size_t>(sig.argCount()));

	    // create buffer for ret val
	    a64::Mem retStack;
	    if (hidden) {
		    // already cached
	    } else {
		    const auto retSize = static_cast<uint32_t>(sizeof(uint64_t) * (TypeUtils::isVec128(sig.ret()) ? 2 : 1));
		    retStack = cc.newStack(retSize, alignment);
		    cc.loadAddressOf(retStruct, retStack);
	    }

	    a64::Gp dest = cc.newGpx();
	    cc.mov(dest, (uint64_t) callback);

	    InvokeNode* invokeNode;
	    cc.invoke(&invokeNode,
			      dest,
			      FuncSignature::build<void, void*, void*, uint64_t*, size_t, void*>()
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
		    if (TypeUtils::isInt(argType)) {
			    cc.ldr(argRegisters.at(argIdx).as<a64::Gp>(), argsStackIdx);
		    } else if (TypeUtils::isFloat(argType)) {
			    cc.ldr(argRegisters.at(argIdx).as<a64::Vec>(), argsStackIdx);
		    } else {
			    _errorCode = "Parameters wider than 64bits not supported";
			    return nullptr;
		    }

		    // next structure slot (+= sizeof(uint64_t))
		    cc.add(i, i, sizeof(uint64_t));
	    }

	    if (hidden) {
		    cc.mov(a64::x8, retStruct);
		    cc.ret();
	    } else if (sig.hasRet()) {
		    a64::Mem retStackIdx0(retStack);
		    if (TypeUtils::isInt(sig.ret())) {
			    a64::Gp tmp = cc.newGp(sig.ret());
			    cc.ldr(tmp, retStackIdx0);
			    cc.ret(tmp);
		    } else if (TypeUtils::isBetween(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.addOffset(sizeof(uint64_t));
			    cc.ldr(a64::x0, retStackIdx0);
			    cc.ldr(a64::x1, retStackIdx1);
			    cc.ret();
		    } else if (sig.ret() == TypeId::kFloat32x2) { // Vector2
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.addOffset(sizeof(float));
			    cc.ldr(a64::s0, retStackIdx0);
			    cc.ldr(a64::s1, retStackIdx1);
			    cc.ret();
		    } else if (sig.ret() == TypeId::kFloat64x2) { // Vector3
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.addOffset(sizeof(float));
			    a64::Mem retStackIdx2(retStack);
			    retStackIdx2.addOffset(sizeof(float) * 2);
			    cc.ldr(a64::s0, retStackIdx0);
			    cc.ldr(a64::s1, retStackIdx1);
			    cc.ldr(a64::s2, retStackIdx2);
			    cc.ret();
		    } else if (sig.ret() == TypeId::kFloat32x4) { // Vector4
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.addOffset(sizeof(float));
			    a64::Mem retStackIdx2(retStack);
			    retStackIdx2.addOffset(sizeof(float) * 2);
			    a64::Mem retStackIdx3(retStack);
			    retStackIdx3.addOffset(sizeof(float) * 3);
			    cc.ldr(a64::s0, retStackIdx0);
			    cc.ldr(a64::s1, retStackIdx1);
			    cc.ldr(a64::s2, retStackIdx2);
			    cc.ldr(a64::s3, retStackIdx3);
			    cc.ret();
		    } else {
			    a64::Vec tmp = cc.newVec(sig.ret());
			    cc.ldr(tmp, retStackIdx0);
			    cc.ret(tmp);
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

MemAddr JitCallback::GetJitFunc(const Signature& signature, const Method* method, CallbackHandler callback, MemAddr data, bool hidden) {
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

    return GetJitFunc(signature, &method, callback, data, hidden);
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