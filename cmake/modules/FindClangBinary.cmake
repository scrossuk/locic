# CMake find_package() Module for Clang binary
#
# Example usage:
#
# find_package(ClangBinary)
#
# If successful the following variables will be defined
# CLANGBINARY_FOUND
# CLANGBINARY_EXECUTABLE

set(CLANG_BINARY_NAME "" CACHE STRING "Set Clang binary name.")

if(NOT "${CLANG_BINARY_NAME}" STREQUAL "")
	find_program(CLANGBINARY_EXECUTABLE
		NAMES ${CLANG_BINARY_NAME}
		DOC "Path to clang executable"
	)
else(NOT "${CLANG_BINARY_NAME}" STREQUAL "")
	set(CLANG_BINARY_SEARCH_NAMES
		clang-3.6
		clang-3.5
		clang-3.4
		clang-3.3
		clang
	)
	find_program(CLANGBINARY_EXECUTABLE
		NAMES ${CLANG_BINARY_SEARCH_NAMES}
		DOC "Path to clang executable"
	)
endif(NOT "${CLANG_BINARY_NAME}" STREQUAL "")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangBinary
	"Failed to locate Clang executable"
	CLANGBINARY_EXECUTABLE
)

