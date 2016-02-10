include_directories(SYSTEM ${LLVMABI_INCLUDE_DIRS})

add_library(locic-codegen
	ArgInfo.cpp
	CodeGenerator.cpp
	ConstantGenerator.cpp
	Context.cpp
	ControlFlow.cpp
	Debug.cpp
	DefaultMethodEmitter.cpp
	Destructor.cpp
	Exception.cpp
	Function.cpp
	FunctionCallInfo.cpp
	FunctionTranslationStub.cpp
	GenABIType.cpp
	GenFunctionCall.cpp
	GenStatement.cpp
	GenType.cpp
	GenValue.cpp
	GenVar.cpp
	GenVTable.cpp
	Interface.cpp
	InternalContext.cpp
	Interpreter.cpp
	IREmitter.cpp
	Linker.cpp
	Liveness.cpp
	LivenessIndicator.cpp
	Mangling.cpp
	md5.cpp
	Memory.cpp
	MethodInfo.cpp
	Module.cpp
	ModulePtr.cpp
	Move.cpp
	PendingResult.cpp
	PrimitiveFunctionEmitter.cpp
	PrimitiveMap.cpp
	Primitives/BoolPrimitive.cpp
	Primitives/CompareResultPrimitive.cpp
	Primitives/FinalLvalPrimitive.cpp
	Primitives/FloatPrimitive.cpp
	Primitives/FunctionPtrPrimitive.cpp
	Primitives/NullPrimitive.cpp
	Primitives/PrimitiveDestructor.cpp
	Primitives/PrimitiveMethod.cpp
	Primitives/PrimitiveMove.cpp
	Primitives/PrimitiveSizeOf.cpp
	Primitives/PrimitiveStoreLval.cpp
	Primitives/PrimitiveTypes.cpp
	Primitives/PtrLvalPrimitive.cpp
	Primitives/PtrPrimitive.cpp
	Primitives/RangePrimitive.cpp
	Primitives/RefPrimitive.cpp
	Primitives/SignedIntegerPrimitive.cpp
	Primitives/StaticArrayPrimitive.cpp
	Primitives/TypenamePrimitive.cpp
	Primitives/UnsignedIntegerPrimitive.cpp
	Primitives/ValueLvalPrimitive.cpp
	Primitives/VoidPrimitive.cpp
	Routines.cpp
	ScopeExitActions.cpp
	SEMCodeEmitter.cpp
	SEMFunctionGenerator.cpp
	SizeOf.cpp
	Support.cpp
	Template.cpp
	TemplateBuilder.cpp
	TemplatedObject.cpp
	TypeGenerator.cpp
	TypeInfo.cpp
	UnwindAction.cpp
	VTable.cpp
	VirtualCall/GenericVirtualCallABI.cpp
	VirtualCall/NestVirtualCallABI.cpp
)

add_dependencies(locic-codegen ${LLVMABI_DEPENDENCIES})

include(PrecompiledHeader)

add_precompiled_header(locic-codegen LLVMIncludes.hpp
	FORCEINCLUDE
	SOURCE_CXX LLVMIncludes.cpp
)


