#include <plugify/function.h>

using namespace plugify;
using namespace asmjit;

template<typename T>
constexpr TypeId GetTypeIdx() {
	return static_cast<TypeId>(TypeUtils::TypeIdOfT<T>::kTypeId);
}

TypeId GetValueTypeId(ValueType valueType) {
	switch (valueType) {
		case ValueType::Invalid:
		case ValueType::Void:
			return GetTypeIdx<void>();
		case ValueType::Bool:
			return GetTypeIdx<bool>();
		case ValueType::Char8:
			return GetTypeIdx<char>();
		case ValueType::Char16:
			return GetTypeIdx<char16_t>();
		case ValueType::Int8:
			return GetTypeIdx<int8_t>();
		case ValueType::Int16:
			return GetTypeIdx<int16_t>();
		case ValueType::Int32:
			return GetTypeIdx<int32_t>();
		case ValueType::Int64:
			return GetTypeIdx<int64_t>();
		case ValueType::UInt8:
			return GetTypeIdx<uint8_t>();
		case ValueType::UInt16:
			return GetTypeIdx<uint16_t>();
		case ValueType::UInt32:
			return GetTypeIdx<uint32_t>();
		case ValueType::UInt64:
			return GetTypeIdx<uint64_t>();
		case ValueType::Float:
			return GetTypeIdx<float>();
		case ValueType::Double:
			return GetTypeIdx<double>();
		case ValueType::Pointer:
		case ValueType::String:
		case ValueType::Function:
		case ValueType::ArrayBool:
		case ValueType::ArrayChar8:
		case ValueType::ArrayChar16:
		case ValueType::ArrayInt8:
		case ValueType::ArrayInt16:
		case ValueType::ArrayInt32:
		case ValueType::ArrayInt64:
		case ValueType::ArrayUInt8:
		case ValueType::ArrayUInt16:
		case ValueType::ArrayUInt32:
		case ValueType::ArrayUInt64:
		case ValueType::ArrayPointer:
		case ValueType::ArrayFloat:
		case ValueType::ArrayDouble:
		case ValueType::ArrayString:
		case ValueType::Vector2:
		case ValueType::Vector3:
		case ValueType::Vector4:
		case ValueType::Matrix4x4:
			return TypeId::kUIntPtr;
	}
	return TypeId::kVoid;
}

TypeId GetRetTypeId(ValueType valueType) {
	switch (valueType) {
		case ValueType::Invalid:
		case ValueType::Void:
			return GetTypeIdx<void>();
		case ValueType::Bool:
			return GetTypeIdx<bool>();
		case ValueType::Char8:
			return GetTypeIdx<char>();
		case ValueType::Char16:
			return GetTypeIdx<char16_t>();
		case ValueType::Int8:
			return GetTypeIdx<int8_t>();
		case ValueType::Int16:
			return GetTypeIdx<int16_t>();
		case ValueType::Int32:
			return GetTypeIdx<int32_t>();
		case ValueType::Int64:
			return GetTypeIdx<int64_t>();
		case ValueType::UInt8:
			return GetTypeIdx<uint8_t>();
		case ValueType::UInt16:
			return GetTypeIdx<uint16_t>();
		case ValueType::UInt32:
			return GetTypeIdx<uint32_t>();
		case ValueType::UInt64:
			return GetTypeIdx<uint64_t>();
		case ValueType::Float:
			return GetTypeIdx<float>();
		case ValueType::Double:
			return GetTypeIdx<double>();
		case ValueType::Pointer:
		case ValueType::String:
		case ValueType::Function:
		case ValueType::ArrayBool:
		case ValueType::ArrayChar8:
		case ValueType::ArrayChar16:
		case ValueType::ArrayInt8:
		case ValueType::ArrayInt16:
		case ValueType::ArrayInt32:
		case ValueType::ArrayInt64:
		case ValueType::ArrayUInt8:
		case ValueType::ArrayUInt16:
		case ValueType::ArrayUInt32:
		case ValueType::ArrayUInt64:
		case ValueType::ArrayPointer:
		case ValueType::ArrayFloat:
		case ValueType::ArrayDouble:
		case ValueType::ArrayString:
		case ValueType::Matrix4x4:
			return TypeId::kUIntPtr;
		case ValueType::Vector2:
#if PLUGIFY_PLATFORM_WINDOWS
			return TypeId::kInt64;
#else
			return TypeId::kFloat64;
#endif
		case ValueType::Vector3:
		case ValueType::Vector4:
#if PLUGIFY_PLATFORM_WINDOWS
			return TypeId::kUIntPtr;
#else
			return TypeId::kFloat32x4;
#endif
	}
	return TypeId::kVoid;
}

CallConvId GetCallConv(const std::string& conv) {
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

Function::Function(std::weak_ptr<asmjit::JitRuntime> rt) : _rt{std::move(rt)} {
}

Function::Function(Function&& other) noexcept : _rt{std::move(other._rt)}, _function{other._function}, _userData{other._userData} {
	other._function = nullptr;
	other._userData = nullptr;
}

Function::~Function() {
	if (auto rt = _rt.lock()) {
		if (_function)
			rt->release(_function);
	}
}

MemAddr Function::GetJitFunc(const asmjit::FuncSignature& sig, const Method& method, FuncCallback callback, MemAddr data) {
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
		if (TypeUtils::isInt(argType)) {
			arg = cc.newUIntPtr();
		} else if (TypeUtils::isFloat(argType)) {
			arg = cc.newXmm();
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		func->setArg(argIdx, arg);
		argRegisters.push_back(arg);
	}

	const uint32_t alignment = 16;

	// setup the stack structure to hold arguments for user callback
	auto stackSize = static_cast<uint32_t>(sizeof(uintptr_t) * sig.argCount());
	x86::Mem argsStack = cc.newStack(stackSize, alignment);
	x86::Mem argsStackIdx(argsStack);

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
		if (TypeUtils::isInt(argType)) {
			cc.mov(argsStackIdx, argRegisters.at(argIdx).as<x86::Gp>());
		} else if(TypeUtils::isFloat(argType)) {
			cc.movq(argsStackIdx, argRegisters.at(argIdx).as<x86::Xmm>());
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
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
	cc.mov(dataPtrParam, data.CCast<uintptr_t>());

	// get pointer to stack structure and pass it to the user callback
	x86::Gp argStruct = cc.newUIntPtr("argStruct");
	cc.lea(argStruct, argsStack);

	// fill reg to pass struct arg count to callback
	x86::Gp argCountParam = cc.newUInt8("argCountParam");
	cc.mov(argCountParam, static_cast<uint8_t>(sig.argCount()));

#if PLUGIFY_PLATFORM_WINDOWS
	auto retSize = static_cast<uint32_t>(sizeof(uintptr_t));
#else
	bool isIntPod = sig.ret() == TypeId::kInt32x4;
	bool isFloatPod = sig.ret() == TypeId::kFloat32x4;
	auto retSize = static_cast<uint32_t>(sizeof(uintptr_t) * ((isFloatPod || isIntPod) ? 2 : 1));
#endif

	// create buffer for ret val
	x86::Mem retStack = cc.newStack(retSize, alignment);
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
		if (TypeUtils::isInt(argType)) {
			cc.mov(argRegisters.at(argIdx).as<x86::Gp>(), argsStackIdx);
		} else if (TypeUtils::isFloat(argType)) {
			cc.movq(argRegisters.at(argIdx).as<x86::Xmm>(), argsStackIdx);
		} else {
			_errorCode = "Parameters wider than 64bits not supported";
			return nullptr;
		}

		// next structure slot (+= sizeof(uintptr_t))
		cc.add(i, sizeof(uintptr_t));
	}

	if (sig.hasRet()) {
		x86::Mem retStackIdx(retStack);
		retStackIdx.setSize(sizeof(uintptr_t));
		if (TypeUtils::isInt(sig.ret())) {
			x86::Gp tmp = cc.newUIntPtr();
			cc.mov(tmp, retStackIdx);
			cc.ret(tmp);
		}
#if !PLUGIFY_PLATFORM_WINDOWS
		else if (isIntPod) {
			x86::Mem retStackIdxUpper(retStack);
			retStackIdxUpper.addOffset(sizeof(uintptr_t));
			retStackIdxUpper.setSize(sizeof(uintptr_t));

			cc.mov(x86::rax, retStackIdx);
			cc.mov(x86::rdx, retStackIdxUpper);
			cc.ret();
		} else if (isFloatPod) {
			x86::Mem retStackIdxUpper(retStack);
			retStackIdxUpper.addOffset(sizeof(uintptr_t));
			retStackIdxUpper.setSize(sizeof(uintptr_t));

			cc.movq(x86::xmm0, retStackIdx);
			cc.movq(x86::xmm1, retStackIdxUpper);
			cc.ret();
		}
#endif
		else {
			x86::Xmm tmp = cc.newXmm();
			cc.movq(tmp, retStackIdx);
			cc.ret(tmp);
		}
	}

	cc.endFunc();

	// write to buffer
	cc.finalize();

	Error err = rt->add(&_function, &code);
	if (err) {
		_function = nullptr;
		_errorCode = DebugUtils::errorAsString(err);
		return nullptr;
	}

	//PL_LOG_VERBOSE("JIT Stub:\n{}", log.data());

	return _function;
}

MemAddr Function::GetJitFunc(const Method& method, FuncCallback callback, MemAddr data, HiddenParam hidden) {
	bool isHiddenParam = hidden(method.retType.type);
	ValueType retType = isHiddenParam ? ValueType::Pointer : (method.retType.ref ? ValueType::Pointer : method.retType.type);
	FuncSignature sig(GetCallConv(method.callConv), method.varIndex, GetRetTypeId(retType));
	if (isHiddenParam) {
		sig.addArg(GetValueTypeId(method.retType.type));
	}
	for (const auto& type : method.paramTypes) {
		sig.addArg(GetValueTypeId(type.ref ? ValueType::Pointer : type.type));
	}
	return GetJitFunc(sig, method, callback, data);
}