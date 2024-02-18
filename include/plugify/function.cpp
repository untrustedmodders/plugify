#include <plugify/function.h>

using namespace plugify;
using namespace asmjit;

Function::Function(std::weak_ptr<asmjit::JitRuntime> rt) : _rt{std::move(rt)} {
}

Function::Function(Function&& other) noexcept : _rt{std::move(other._rt)}, _doneCallback{other._doneCallback}, _function{other._function}, _userData{other._userData}, _error{std::move(other._error)} {
	other._doneCallback = nullptr;
	other._function = nullptr;
	other._userData = nullptr;
}

Function::~Function() {
	if (auto rt = _rt.lock()) {
		if (_function)
			rt->release(_function);
	}
	if (_doneCallback) {
		_doneCallback(_userData);
	}
}

void* Function::GetJitFunc(const asmjit::FuncSignature& sig, const Method& method, FuncCallback callback, void* data) {
	if (_function)
		return _function;

	auto rt = _rt.lock();
	if (!rt) {
		_error = "JitRuntime invalid";
		return nullptr;
	}

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

	CodeHolder code;
	code.init(rt->environment(), rt->cpuFeatures());

	// initialize function
	x86::Compiler cc(&code);
	FuncNode* func = cc.addFunc(sig);

	/*StringLogger log;
	auto kFormatFlags = FormatFlags::kMachineCode | FormatFlags::kExplainImms | FormatFlags::kRegCasts | FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kPositions;

	log.addFlags(kFormatFlags);
	code.setLogger(&log);*/

	// too small to really need it
	func->frame().resetPreservedFP();

	// map argument slots to registers, following abi.
	std::vector<x86::Reg> argRegisters;
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); argIdx++) {
		const auto& argType = sig.args()[argIdx];

		x86::Reg arg;
		if (IsGeneralReg(argType)) {
			arg = cc.newUIntPtr();
		} else if (IsXmmReg(argType)) {
			arg = cc.newXmm();
		} else {
			_error = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		func->setArg(argIdx, arg);
		argRegisters.push_back(arg);
	}

	const uint32_t alignment = 16;

	// setup the stack structure to hold arguments for user callback
	uint32_t stackSize = static_cast<uint32_t>(sizeof(uintptr_t) * sig.argCount());
	x86::Mem argsStack = cc.newStack(stackSize, alignment);
	x86::Mem argsStackIdx{argsStack};

	// assigns some register as index reg
	x86::Gp i = cc.newUIntPtr();

	// stackIdx <- stack[i].
	argsStackIdx.setIndex(i);

	// r/w are sizeof(uintptr_t) width now
	argsStackIdx.setSize(sizeof(uintptr_t));

	// set i = 0
	cc.mov(i, 0);
	//// mov from arguments registers into the stack structure
	for (uint32_t argIdx = 0; argIdx < sig.argCount(); ++argIdx) {
		const auto& argType = sig.args()[argIdx];

		// have to cast back to explicit register types to gen right mov type
		if (IsGeneralReg(argType)) {
			cc.mov(argsStackIdx, argRegisters.at(argIdx).as<x86::Gp>());
		} else if(IsXmmReg(argType)) {
			cc.movq(argsStackIdx, argRegisters.at(argIdx).as<x86::Xmm>());
		} else {
			_error = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uintptr_t))
		cc.add(i, sizeof(uintptr_t));
	}

	// fill reg to pass method ptr to callback
	x86::Gp methodPtrParam = cc.newUIntPtr("methodPtrParam");
	cc.mov(methodPtrParam, reinterpret_cast<uintptr_t>(&method));

	// fill reg to pass data ptr to callback
	x86::Gp dataPtrParam = cc.newUIntPtr("dataPtrParam");
	cc.mov(dataPtrParam, reinterpret_cast<uintptr_t>(data));

	// get pointer to stack structure and pass it to the user callback
	x86::Gp argStruct = cc.newUIntPtr("argStruct");
	cc.lea(argStruct, argsStack);

	// fill reg to pass struct arg count to callback
	x86::Gp argCountParam = cc.newUInt8("argCountParam");
	cc.mov(argCountParam, static_cast<uint8_t>(sig.argCount()));

	// create buffer for ret val
	x86::Mem retStack = cc.newStack(sizeof(uintptr_t), alignment);
	x86::Gp retStruct = cc.newUIntPtr("retStruct");
	cc.lea(retStruct, retStack);

	InvokeNode* invokeNode;
	cc.invoke(&invokeNode,
			  reinterpret_cast<uintptr_t>(callback),
			  FuncSignature::build<void, Method*, void*, Parameters*, uint8_t, ReturnValue*>()
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
		if (IsGeneralReg(argType)) {
			cc.mov(argRegisters.at(argIdx).as<x86::Gp>(), argsStackIdx);
		}else if (IsXmmReg(argType)) {
			cc.movq(argRegisters.at(argIdx).as<x86::Xmm>(), argsStackIdx);
		}else {
			_error = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uintptr_t))
		cc.add(i, sizeof(uintptr_t));
	}

	if (sig.hasRet()) {
		x86::Mem retStackIdx{retStack};
		retStackIdx.setSize(sizeof(uintptr_t));
		if (IsGeneralReg(sig.ret())) {
			x86::Gp tmp2 = cc.newUIntPtr();
			cc.mov(tmp2, retStackIdx);
			cc.ret(tmp2);
		} else {
			x86::Xmm tmp2 = cc.newXmm();
			cc.movq(tmp2, retStackIdx);
			cc.ret(tmp2);
		}
	}

	cc.endFunc();

	// write to buffer
	cc.finalize();

	Error err = rt->add(&_function, &code);
	if (err) {
		_error = DebugUtils::errorAsString(err);
		return nullptr;
	}

	_userData = data;

	//PL_LOG_VERBOSE("JIT Stub:\n{}", log.data());

	return _function;
}

void* Function::GetJitFunc(const Method& method, FuncCallback callback, void* data) {
	FuncSignature sig(GetCallConv(method.callConv), method.varIndex, GetTypeId(method.retType.type));
	for (const auto& type : method.paramTypes) {
		sig.addArg(GetTypeId(type.type));
	}
	return GetJitFunc(sig, method, callback, data);
}

template<typename T>
constexpr TypeId GetTypeIdx() {
	return static_cast<TypeId>(TypeUtils::TypeIdOfT<T>::kTypeId);
}

TypeId Function::GetTypeId(ValueType valueType) {
	switch (valueType) {
		case ValueType::Invalid:
		case ValueType::Void:   return GetTypeIdx<void>();
		case ValueType::Bool:   return GetTypeIdx<bool>();
		case ValueType::Char8:  return GetTypeIdx<char>();
		case ValueType::Char16: return GetTypeIdx<wchar_t>();
		case ValueType::Int8:   return GetTypeIdx<int8_t>();
		case ValueType::Int16:  return GetTypeIdx<int16_t>();
		case ValueType::Int32:  return GetTypeIdx<int32_t>();
		case ValueType::Int64:  return GetTypeIdx<int64_t>();
		case ValueType::Uint8:  return GetTypeIdx<uint8_t>();
		case ValueType::Uint16: return GetTypeIdx<uint16_t>();
		case ValueType::Uint32: return GetTypeIdx<uint32_t>();
		case ValueType::Uint64: return GetTypeIdx<uint64_t>();
		case ValueType::Float:  return GetTypeIdx<float>();
		case ValueType::Double: return GetTypeIdx<double>();
		case ValueType::Ptr64:
		case ValueType::String:
		case ValueType::Function:
		case ValueType::ArrayBool:
		case ValueType::ArrayChar8:
		case ValueType::ArrayChar16:
		case ValueType::ArrayInt8:
		case ValueType::ArrayInt16:
		case ValueType::ArrayInt32:
		case ValueType::ArrayInt64:
		case ValueType::ArrayUint8:
		case ValueType::ArrayUint16:
		case ValueType::ArrayUint32:
		case ValueType::ArrayUint64:
		case ValueType::ArrayPtr64:
		case ValueType::ArrayFloat:
		case ValueType::ArrayDouble:
		case ValueType::ArrayString: return TypeId::kUIntPtr;
	}
	return TypeId::kVoid;
}

CallConvId Function::GetCallConv(const std::string& conv) {
#if PLUGIFY_ARCH_X86 == 64
#if PLUGIFY_PLATFORM_WINDOWS
	if (conv == "vectorcall") {
		return CallConvId::kVectorCall;
	}
	return CallConvId::kX64Windows;
#else
	return CallConvId::kX64SystemV;
#endif // PLUGIFY_PLATFORM_WINDOWS
#elif PLUGIFY_ARCH_X86 == 32
	if (conv == "cdecl") {
		return CallConvId::kCDecl;
	} else if (conv == "stdcall") {
		return CallConvId::kStdCall;
	} else if (conv == "fastcall") {
		return CallConvId::kFastCall;
	} else if (conv == "thiscall") {
		return CallConvId::kThisCall;
	} else if (conv == "vectorcall") {
		return CallConvId::kVectorCall;
	}
	return CallConvId::kHost;
#endif // PLUGIFY_ARCH_X86
}

bool Function::IsGeneralReg(TypeId typeId) {
	switch (typeId) {
		case TypeId::kInt8:
		case TypeId::kUInt8:
		case TypeId::kInt16:
		case TypeId::kUInt16:
		case TypeId::kInt32:
		case TypeId::kUInt32:
		case TypeId::kInt64:
		case TypeId::kUInt64:
		case TypeId::kIntPtr:
		case TypeId::kUIntPtr:
			return true;
		default:
			return false;
	}
}

bool Function::IsXmmReg(TypeId typeId) {
	switch (typeId) {
		case TypeId::kFloat32:
		case TypeId::kFloat64:
			return true;
		default:
			return false;
	}
}
