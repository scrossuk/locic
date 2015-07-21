find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCIC_BUILD_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

set(RUNTIME_FILES
	bool_t.loci
	byte_t.loci
	callable.loci
	comparable.loci
	compare_result_t.loci
	copyable.loci
	default_constructible.loci
	dissolvable.loci
	double_t.loci
	final_lval.loci
	float_t.loci
	function_ptr_t.loci
	int16_t.loci
	int32_t.loci
	int64_t.loci
	int8_t.loci
	interfacemethod_t.loci
	int_t.loci
	longdouble_t.loci
	longlong_t.loci
	long_t.loci
	method_t.loci
	methodfunction_ptr_t.loci
	movable.loci
	null_t.loci
	ptrdiff_t.loci
	ptr.loci
	ptr_lval.loci
	ref.loci
	short_t.loci
	size_t.loci
	sized_type.loci
	ssize_t.loci
	staticinterfacemethod_t.loci
	templatedfunction_ptr_t.loci
	templatedmethodfunction_ptr_t.loci
	templatedmethod_t.loci
	typename_t.loci
	ubyte_t.loci
	uint16_t.loci
	uint32_t.loci
	uint64_t.loci
	uint8_t.loci
	uint_t.loci
	ulonglong_t.loci
	ulong_t.loci
	ushort_t.loci
	value_lval.loci
	varargfunction_ptr_t.loci
	void_t.loci
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci"
	COMMAND ${CMAKE_COMMAND}
		-D INPUT_FILES="${RUNTIME_FILES}"
		-D OUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci"
		-P "${CMAKE_SOURCE_DIR}/cmake/ConcatFiles.cmake"
	DEPENDS ${RUNTIME_FILES}
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c"
	COMMAND ${CMAKE_COMMAND}
		-D INPUT_FILE="BuiltInTypes.loci"
		-D OUTPUT_FILE="BuiltInTypes.loci.c"
		-D WORKING_DIR="${CMAKE_CURRENT_BINARY_DIR}"
		-P "${CMAKE_SOURCE_DIR}/cmake/MakeCString.cmake"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set(RUNTIME_FILES
	assert.cpp
	exception.cpp
)

add_library(loci-builtintypes STATIC
	"${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c"
)

add_library(loci-runtime STATIC
	${RUNTIME_FILES}
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime.bc"
	COMMAND
		${CLANG_EXECUTABLE} -fno-exceptions -c -emit-llvm -o "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime-assert.bc" "assert.cpp"
	COMMAND
		${CLANG_EXECUTABLE} -fno-exceptions -c -emit-llvm -o "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime-exception.bc" "exception.cpp"
	COMMAND
		${LLVM_LINK_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime-assert.bc" "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime-exception.bc" -o "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime.bc"
	DEPENDS ${RUNTIME_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(loci-runtime-bc ALL
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/loci-runtime.bc"
)
