# CMake find_package() Module for LLVM binaries
#
# Example usage:
#
# find_package(LLVMBinaries)
#
# If successful the following variables will be defined
# LLVMBINARIES_FOUND
# LLVMBINARIES_LINK_EXECUTABLE
# LLVMBINARIES_OPT_EXECUTABLE

find_program(LLVMBINARIES_DIS_EXECUTABLE
	NAMES llvm-dis
	DOC "Path to llvm-dis executable")

find_program(LLVMBINARIES_LINK_EXECUTABLE
	NAMES llvm-link
	DOC "Path to llvm-link executable")

find_program(LLVMBINARIES_LLC_EXECUTABLE
	NAMES llc
	DOC "Path to llc executable")

find_program(LLVMBINARIES_OPT_EXECUTABLE
	NAMES opt
	DOC "Path to opt executable")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LLVMBinaries
	"Failed to locate LLVM executable(s)"
	LLVMBINARIES_DIS_EXECUTABLE
	LLVMBINARIES_LINK_EXECUTABLE
	LLVMBINARIES_LLC_EXECUTABLE
	LLVMBINARIES_OPT_EXECUTABLE
	)

