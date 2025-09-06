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
	    code.init(rt.environment(), rt.cpu_features());
	    code.set_error_handler(&eh);

	    // initialize function
	    a64::Compiler cc(&code);
	    FuncNode* func = cc.add_func(sig);

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
	    std::vector<Reg> argRegisters;
	    argRegisters.reserve(sig.arg_count());

	    for (uint32_t argIdx = 0; argIdx < sig.arg_count(); ++argIdx) {
		    const auto& argType = sig.args()[argIdx];

		    Reg arg;
		    if (TypeUtils::is_int(argType)) {
			    arg = cc.new_gp(argType);
		    } else if (TypeUtils::is_float(argType)) {
			    arg = cc.new_vec(argType);
		    } else {
			    _errorCode = "Parameters wider than 64bits not supported";
			    return nullptr;
		    }

		    func->set_arg(argIdx, arg);
		    argRegisters.push_back(std::move(arg));
	    }

	    a64::Gp retStruct = cc.new_gpz("retStruct");

	    // store x8 in advance
	    if (hidden) {
		    cc.mov(retStruct, a64::x8);
	    }

	    const uint32_t alignment = 16;

	    // setup the stack structure to hold arguments for user callback
	    const auto stackSize = static_cast<uint32_t>(sizeof(uint64_t) * sig.arg_count());
	    a64::Mem argsStack;
	    if (stackSize > 0) {
		    argsStack = cc.new_stack(stackSize, alignment);
	    }
	    a64::Mem argsStackIdx(argsStack);

	    // assigns some register as index reg
	    a64::Gp i = cc.new_gpz();

	    // stackIdx <- stack[i].
	    argsStackIdx.set_index(i);

	    // set i = 0
	    cc.mov(i, 0);

	    //// mov from arguments registers into the stack structure
	    for (uint32_t argIdx = 0; argIdx < sig.arg_count(); ++argIdx) {
		    const auto& argType = sig.args()[argIdx];

		    // have to cast back to explicit register types to gen right mov type
		    if (TypeUtils::is_int(argType)) {
			    cc.str(argRegisters.at(argIdx).as<a64::Gp>(), argsStackIdx);
		    } else if (TypeUtils::is_float(argType)) {
			    cc.str(argRegisters.at(argIdx).as<a64::Vec>(), argsStackIdx);
		    } else {
			    _errorCode = "Parameters wider than 64bits not supported";
			    return nullptr;
		    }

		    // next structure slot (+= sizeof(uint64_t))
		    cc.add(i, i, sizeof(uint64_t));
	    }

	    // fill reg to pass method ptr to callback
	    a64::Gp methodPtrParam = cc.new_gpz("methodPtrParam");
	    cc.mov(methodPtrParam, method);

	    // fill reg to pass data ptr to callback
	    a64::Gp dataPtrParam = cc.new_gpz("dataPtrParam");
	    cc.mov(dataPtrParam, static_cast<uintptr_t>(data));

	    // get pointer to stack structure and pass it to the user callback
	    a64::Gp argStruct = cc.new_gpz("argStruct");
	    cc.load_address_of(argStruct, argsStack);

	    // fill reg to pass struct arg count to callback
	    a64::Gp arg_countParam = cc.new_gpz("arg_countParam");
	    cc.mov(arg_countParam, static_cast<size_t>(sig.arg_count()));

	    // create buffer for ret val
	    a64::Mem retStack;
	    if (hidden) {
		    // already cached
	    } else {
		    const auto retSize = static_cast<uint32_t>(sizeof(uint64_t) * (TypeUtils::is_vec128(sig.ret()) ? 2 : 1));
		    retStack = cc.new_stack(retSize, alignment);
		    cc.load_address_of(retStruct, retStack);
	    }

	    a64::Gp dest = cc.new_gpz();
	    cc.mov(dest, (uint64_t) callback);

	    InvokeNode* invokeNode;
	    cc.invoke(Out(invokeNode),
			      dest,
			      FuncSignature::build<void, void*, void*, uint64_t*, size_t, void*>()
	    );

	    // call to user provided function (use ABI of host compiler)
	    invokeNode->set_arg(0, methodPtrParam);
	    invokeNode->set_arg(1, dataPtrParam);
	    invokeNode->set_arg(2, argStruct);
	    invokeNode->set_arg(3, arg_countParam);
	    invokeNode->set_arg(4, retStruct);

	    // mov from arguments stack structure into regs
	    cc.mov(i, 0); // reset idx
	    for (uint32_t argIdx = 0; argIdx < sig.arg_count(); ++argIdx) {
		    const auto& argType = sig.args()[argIdx];
		    if (TypeUtils::is_int(argType)) {
			    cc.ldr(argRegisters.at(argIdx).as<a64::Gp>(), argsStackIdx);
		    } else if (TypeUtils::is_float(argType)) {
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
	    } else if (sig.has_ret()) {
		    a64::Mem retStackIdx0(retStack);
		    if (TypeUtils::is_int(sig.ret())) {
			    a64::Gp tmp = cc.new_gp(sig.ret());
			    cc.ldr(tmp, retStackIdx0);
			    cc.ret(tmp);
		    } else if (TypeUtils::is_between(sig.ret(), TypeId::kInt8x16, TypeId::kUInt64x2)) {
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.add_offset(sizeof(uint64_t));
			    cc.ldr(a64::x0, retStackIdx0);
			    cc.ldr(a64::x1, retStackIdx1);
			    cc.ret();
		    } else if (sig.ret() == TypeId::kFloat32x2) { // Vector2
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.add_offset(sizeof(float));
			    cc.ldr(a64::s0, retStackIdx0);
			    cc.ldr(a64::s1, retStackIdx1);
			    cc.ret();
		    } else if (sig.ret() == TypeId::kFloat64x2) { // Vector3
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.add_offset(sizeof(float));
			    a64::Mem retStackIdx2(retStack);
			    retStackIdx2.add_offset(sizeof(float) * 2);
			    cc.ldr(a64::s0, retStackIdx0);
			    cc.ldr(a64::s1, retStackIdx1);
			    cc.ldr(a64::s2, retStackIdx2);
			    cc.ret();
		    } else if (sig.ret() == TypeId::kFloat32x4) { // Vector4
			    a64::Mem retStackIdx1(retStack);
			    retStackIdx1.add_offset(sizeof(float));
			    a64::Mem retStackIdx2(retStack);
			    retStackIdx2.add_offset(sizeof(float) * 2);
			    a64::Mem retStackIdx3(retStack);
			    retStackIdx3.add_offset(sizeof(float) * 3);
			    cc.ldr(a64::s0, retStackIdx0);
			    cc.ldr(a64::s1, retStackIdx1);
			    cc.ldr(a64::s2, retStackIdx2);
			    cc.ldr(a64::s3, retStackIdx3);
			    cc.ret();
		    } else {
			    a64::Vec tmp = cc.new_vec(sig.ret());
			    cc.ldr(tmp, retStackIdx0);
			    cc.ret(tmp);
		    }
	    } else {
	        cc.ret();
	    }

	    cc.end_func();

	    // write to buffer
	    cc.finalize();

	    rt.add(&_function, &code);

	    if (eh.error != Error::kOk) {
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

    Signature signature(method.GetCallConv(), retHidden ? ValueType::Pointer : retType, method.GetVarIndex());
    if (retHidden) {
        signature.AddArg(retType);
    }
    for (const auto& type : method.GetParamTypes()) {
        signature.AddArg(type.IsRef() ? ValueType::Pointer : type.GetType());
    }

    return GetJitFunc(signature, &method, callback, data, retHidden);
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