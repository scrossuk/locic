# --- Script Setup

cmake_minimum_required (VERSION 2.8)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Disable in-source builds and modifications
# to the source tree.
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Enable compiler tests.
enable_testing()

project(LOCIC C CXX)

if(NOT CMAKE_BUILD_TYPE)
	# Build debug by default.
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type (options are None, Debug, Release, RelWithDebInfo and MinSizeRel)." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# --- Compiler Properties

set(LOCIC_VERSION_MAJOR 1)
set(LOCIC_VERSION_MINOR 0)
set(LOCIC_VERSION_PATCH 0)
set(LOCIC_VERSION_BUILD 0)
set(LOCIC_VERSION ${LOCIC_VERSION_MAJOR}.${LOCIC_VERSION_MINOR}.${LOCIC_VERSION_PATCH}.${LOCIC_VERSION_BUILD})

message(STATUS "Building Loci Compiler Tools version ${LOCIC_VERSION} using build type '${CMAKE_BUILD_TYPE}'.")
message(STATUS "    Source directory is '${PROJECT_SOURCE_DIR}'.")
message(STATUS "    Build directory is '${PROJECT_BINARY_DIR}'.")

# --- Compiler Flags

add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

# Enable most warnings.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings")

# Use C++11.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Enable/disable optimisation depending on build type.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Add version as preprocessor defines.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_VERSION=${LOCIC_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_VERSION_MAJOR=${LOCIC_VERSION_MAJOR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_VERSION_MINOR=${LOCIC_VERSION_MINOR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_VERSION_PATCH=${LOCIC_VERSION_PATCH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_VERSION_BUILD=${LOCIC_VERSION_BUILD}")

# --- Dependencies

# Find LLVM (for code generation).
find_package(LLVM REQUIRED)

# Check LLVM version.
set(LLVM_VERSION_STRING "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}")
set(LLVM_VERSION_NAME "")
if("${LLVM_VERSION_STRING}" STREQUAL "3.3")
	set(LLVM_VERSION_NAME "3_3")
else("${LLVM_VERSION_STRING}" STREQUAL "3.4")
	set(LLVM_VERSION_NAME "3_4")
endif("${LLVM_VERSION_STRING}" STREQUAL "3.3")

if("${LLVM_VERSION_NAME}" STREQUAL "")
	message(FATAL_ERROR "LLVM version ${LLVM_VERSION_STRING} is not supported.")
endif("${LLVM_VERSION_NAME}" STREQUAL "")

message(STATUS "Using LLVM ${LLVM_VERSION_STRING} (name is '${LLVM_VERSION_NAME}').")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLLVM_${LLVM_VERSION_NAME}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_LLVM_VERSION_MINOR=${LLVM_VERSION_MINOR}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(LLVM_REQ_LIBS bitreader bitwriter core jit mcjit nativecodegen native ipo option)

# Clang (for target information).
set(CLANG_REQ_LIBS clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization)

# --- Subdirectories

# All headers are in the /include directory.
include_directories (
	"${CMAKE_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/lib/CodeGen/llvm-abi/include"
	)

# Documentation.
add_subdirectory(docs)

# Loci examples that are build by the compiler.
add_subdirectory(examples)

# Compiler libraries (e.g. SemanticAnalysis).
add_subdirectory(lib)

# Language runtime.
add_subdirectory(runtime)

# Compiler tests.
add_subdirectory(test)

# Compiler tools (such as the 'locic' command line tool).
add_subdirectory(tools)

