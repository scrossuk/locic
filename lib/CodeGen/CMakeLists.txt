include_directories(${LLVMABI_INCLUDE_DIRS})

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
	Primitives/Bool.cpp
	Primitives/CompareResult.cpp
	Primitives/FinalLval.cpp
	Primitives/Float.cpp
	Primitives/FunctionPtr.cpp
	Primitives/Null.cpp
	Primitives/PrimitiveDestructor.cpp
	Primitives/PrimitiveMethod.cpp
	Primitives/PrimitiveMove.cpp
	Primitives/PrimitiveSizeOf.cpp
	Primitives/PrimitiveStoreLval.cpp
	Primitives/PrimitiveTypes.cpp
	Primitives/PrimitiveTypeSizeKnowledge.cpp
	Primitives/Ptr.cpp
	Primitives/PtrLval.cpp
	Primitives/Ref.cpp
	Primitives/SignedInteger.cpp
	Primitives/Typename.cpp
	Primitives/UnsignedInteger.cpp
	Primitives/ValueLval.cpp
	Primitives/Void.cpp
	Routines.cpp
	ScopeExitActions.cpp
	SEMCodeEmitter.cpp
	SizeOf.cpp
	Support.cpp
	Template.cpp
	TemplateBuilder.cpp
	TemplatedObject.cpp
	TypeGenerator.cpp
	TypeSizeKnowledge.cpp
	UnwindAction.cpp
	VTable.cpp
	VirtualCall/VirtualCall-Default.cpp
)

add_dependencies(locic-codegen ${LLVMABI_DEPENDENCIES})

include(PrecompiledHeader)

add_precompiled_header(locic-codegen LLVMIncludes.hpp
	FORCEINCLUDE
	SOURCE_CXX LLVMIncludes.cpp
)


