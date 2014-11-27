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
	# If user specifies a name only look for clang with
	# that particular binary name.
	set(CLANG_BINARY_SEARCH_NAMES
		${CLANG_BINARY_NAME}
	)
else(NOT "${CLANG_BINARY_NAME}" STREQUAL "")
	# Otherwise search for clang with a
	# set of typical names.
	set(CLANG_BINARY_SEARCH_NAMES
		clang-3.6
		clang-3.5
		clang-3.4
		clang-3.3
		clang
	)
endif(NOT "${CLANG_BINARY_NAME}" STREQUAL "")

if(NOT "${CLANG_ROOT_DIR}" STREQUAL "")
	# If user specifies a directory only look for clang
	# in that particular root directory.
	find_program(CLANGBINARY_EXECUTABLE
		NAMES ${CLANG_BINARY_SEARCH_NAMES}
		PATHS ${CLANG_ROOT_DIR}/bin NO_DEFAULT_PATH
		DOC "Path to clang executable"
	)
else(NOT "${CLANG_ROOT_DIR}" STREQUAL "")
	# Otherwise search for clang in default paths.
	find_program(CLANGBINARY_EXECUTABLE
		NAMES ${CLANG_BINARY_SEARCH_NAMES}
		DOC "Path to clang executable"
	)
endif(NOT "${CLANG_ROOT_DIR}" STREQUAL "")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangBinary
	"Failed to locate Clang executable"
	CLANGBINARY_EXECUTABLE
)

