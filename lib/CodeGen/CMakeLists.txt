add_subdirectory(llvm-abi)

include_directories(llvm-abi/include)

add_library(locic-codegen
	CodeGen.cpp
	Debug.cpp
	DefaultMethods.cpp
	Destructor.cpp
	Exception.cpp
	GenABIType.cpp
	GenFunction.cpp
	GenFunctionCall.cpp
	GenStatement.cpp
	GenType.cpp
	GenTypeInstance.cpp
	GenValue.cpp
	GenVar.cpp
	GenVTable.cpp
	Interpreter.cpp
	Mangling.cpp
	md5.cpp
	Memory.cpp
	Module.cpp
	Primitives.cpp
	SizeOf.cpp
	Support.cpp
	TargetInfo.cpp
	TypeSizeKnowledge.cpp
	UnwindAction.cpp
	VTable.cpp
	VirtualCall/VirtualCall-Default.cpp
	)

