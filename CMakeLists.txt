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

set(LOCIC_MAJOR_VERSION 1)
set(LOCIC_MINOR_VERSION 0)
set(LOCIC_PATCH_VERSION 0)
set(LOCIC_BUILD_VERSION 0)
set(LOCIC_VERSION ${LOCIC_MAJOR_VERSION}.${LOCIC_MINOR_VERSION}.${LOCIC_PATCH_VERSION}.${LOCIC_BUILD_VERSION})

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_MAJOR_VERSION=${LOCIC_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_MINOR_VERSION=${LOCIC_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_PATCH_VERSION=${LOCIC_PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCIC_BUILD_VERSION=${LOCIC_BUILD_VERSION}")

# --- Dependencies

# Find LLVM (for code generation).
find_package(LLVM REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(LLVM_REQ_LIBS bitreader bitwriter core jit native ipo)

# Clang (for target information).
set(CLANG_REQ_LIBS clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization)

# --- Subdirectories

# All headers are in the /include directory.
include_directories (
	"${CMAKE_SOURCE_DIR}/include"
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

