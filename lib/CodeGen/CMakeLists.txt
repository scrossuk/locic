add_subdirectory(llvm-abi)

add_library(locic-codegen
	ArgInfo.cpp
	CodeGenerator.cpp
	ConstantGenerator.cpp
	Context.cpp
	ControlFlow.cpp
	Debug.cpp
	Destructor.cpp
	Exception.cpp
	Function.cpp
	FunctionCallInfo.cpp
	GenABIType.cpp
	GenFunction.cpp
	GenFunctionCall.cpp
	GenStatement.cpp
	GenType.cpp
	GenTypeInstance.cpp
	GenValue.cpp
	GenVar.cpp
	GenVTable.cpp
	Interface.cpp
	InternalContext.cpp
	Interpreter.cpp
	Linker.cpp
	Mangling.cpp
	md5.cpp
	Memory.cpp
	Module.cpp
	ModulePtr.cpp
	Move.cpp
	Primitives/PrimitiveDestructor.cpp
	Primitives/PrimitiveMethod.cpp
	Primitives/PrimitiveMove.cpp
	Primitives/PrimitiveSizeOf.cpp
	Primitives/PrimitiveStoreLval.cpp
	Primitives/PrimitiveTypes.cpp
	Primitives/PrimitiveTypeSizeKnowledge.cpp
	Primitives/PrimitiveValueLval.cpp
	Routines.cpp
	ScopeExitActions.cpp
	SizeOf.cpp
	Support.cpp
	Template.cpp
	TemplateBuilder.cpp
	TemplatedObject.cpp
	TypeSizeKnowledge.cpp
	UnwindAction.cpp
	VTable.cpp
	VirtualCall/VirtualCall-Default.cpp
)

include(PrecompiledHeader)

add_precompiled_header(locic-codegen LLVMIncludes.hpp
	FORCEINCLUDE
	SOURCE_CXX LLVMIncludes.cpp
)


