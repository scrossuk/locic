include_directories(SYSTEM ${LLVMABI_INCLUDE_DIRS})

add_library(locic-codegen
	ArgInfo.cpp
	ASTCodeEmitter.cpp
	ASTFunctionGenerator.cpp
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
	GenType.cpp
	GenVTable.cpp
	Interface.cpp
	InternalContext.cpp
	Interpreter.cpp
	IREmitter.cpp
	Linker.cpp
	LivenessEmitter.cpp
	LivenessIndicator.cpp
	LivenessInfo.cpp
	Mangling.cpp
	md5.cpp
	MethodInfo.cpp
	Module.cpp
	ModulePtr.cpp
	PendingResult.cpp
	PrimitiveFunctionEmitter.cpp
	PrimitiveMap.cpp
	Primitives/BoolPrimitive.cpp
	Primitives/CompareResultPrimitive.cpp
	Primitives/FloatPrimitive.cpp
	Primitives/FunctionPtrPrimitive.cpp
	Primitives/NullPrimitive.cpp
	Primitives/PrimitiveDestructor.cpp
	Primitives/PrimitiveMethod.cpp
	Primitives/PrimitiveSizeOf.cpp
	Primitives/PrimitiveTypes.cpp
	Primitives/PtrLvalPrimitive.cpp
	Primitives/PtrPrimitive.cpp
	Primitives/RangePrimitive.cpp
	Primitives/RefPrimitive.cpp
	Primitives/SignedIntegerPrimitive.cpp
	Primitives/StaticArrayPrimitive.cpp
	Primitives/TypenamePrimitive.cpp
	Primitives/UnicharPrimitive.cpp
	Primitives/UnsignedIntegerPrimitive.cpp
	Primitives/ValueLvalPrimitive.cpp
	Primitives/VoidPrimitive.cpp
	Routines.cpp
	ScopeEmitter.cpp
	ScopeExitActions.cpp
	SizeOf.cpp
	StatementEmitter.cpp
	Support.cpp
	Template.cpp
	TemplateBuilder.cpp
	TemplatedObject.cpp
	TypeGenerator.cpp
	TypeInfo.cpp
	UnwindAction.cpp
	ValueEmitter.cpp
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


