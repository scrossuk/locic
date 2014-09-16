# CMake find_package() Module for Clang binary
#
# Example usage:
#
# find_package(ClangBinary)
#
# If successful the following variables will be defined
# CLANGBINARY_FOUND
# CLANGBINARY_EXECUTABLE

find_program(CLANGBINARY_EXECUTABLE
	NAMES clang
	DOC "Path to clang executable")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangBinary
	"Failed to locate Clang executable"
	CLANGBINARY_EXECUTABLE
	)

