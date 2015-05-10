# --- Script Setup

cmake_minimum_required (VERSION 2.8)

# Use Debug build by default.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Select build type. Options are: 'Debug', 'Release', 'RelWithDebInfo' and 'MinSizeRel'."
		FORCE
	)
	message(STATUS "Set CMAKE_BUILD_TYPE to default '${CMAKE_BUILD_TYPE}'.")
endif(NOT CMAKE_BUILD_TYPE)

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

# Add path for custom modules
set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

# --- Compiler Properties

set(LOCIC_VERSION_MAJOR 1)
set(LOCIC_VERSION_MINOR 3)
set(LOCIC_VERSION_PATCH 0)
set(LOCIC_VERSION_BUILD 0)
set(LOCIC_VERSION ${LOCIC_VERSION_MAJOR}.${LOCIC_VERSION_MINOR}.${LOCIC_VERSION_PATCH}.${LOCIC_VERSION_BUILD})

message(STATUS "Building Loci Compiler Tools version ${LOCIC_VERSION} using build type '${CMAKE_BUILD_TYPE}'.")
message(STATUS "    Source directory is '${PROJECT_SOURCE_DIR}'.")
message(STATUS "    Build directory is '${PROJECT_BINARY_DIR}'.")

# --- Options

set(LOCIC_BUILD_EXAMPLES ON CACHE BOOL "Build Loci examples")

if(LOCIC_BUILD_EXAMPLES)
	message(STATUS "Building examples is ENABLED (use -DLOCIC_BUILD_EXAMPLES=OFF to disable).")
else()
	message(STATUS "Building examples is DISABLED (use -DLOCIC_BUILD_EXAMPLES=ON to enable).")
endif()

# --- Compiler Flags

add_definitions(
	-D__STDC_LIMIT_MACROS
	-D__STDC_CONSTANT_MACROS
)

# Enable most warnings.
set(LOCIC_BUILD_WARNINGS "-pedantic -Wall -Wextra -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings")

# Use C++11.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Add version as preprocessor defines.
add_definitions(
	"-DLOCIC_VERSION=${LOCIC_VERSION}"
	"-DLOCIC_VERSION_MAJOR=${LOCIC_VERSION_MAJOR}"
	"-DLOCIC_VERSION_MINOR=${LOCIC_VERSION_MINOR}"
	"-DLOCIC_VERSION_PATCH=${LOCIC_VERSION_PATCH}"
	"-DLOCIC_VERSION_BUILD=${LOCIC_VERSION_BUILD}"
)

# --- User config

set(LLVM_ROOT_DIR "" CACHE STRING "Set LLVM root directory.")
set(CLANG_BINARY_NAME "" CACHE STRING "Set Clang binary name.")
set(CLANG_ROOT_DIR "" CACHE STRING "Set Clang root directory.")

if(NOT "${LLVM_ROOT_DIR}" STREQUAL "")
	message(STATUS "Using LLVM_ROOT_DIR=${LLVM_ROOT_DIR}")
else(NOT "${LLVM_ROOT_DIR}" STREQUAL "")
	message(STATUS "LLVM_ROOT_DIR wasn't specified (you can use this to search for LLVM in a particular path).")
endif(NOT "${LLVM_ROOT_DIR}" STREQUAL "")

if(NOT "${CLANG_BINARY_NAME}" STREQUAL "")
	message(STATUS "Using CLANG_BINARY_NAME=${CLANG_BINARY_NAME}")
else(NOT "${CLANG_BINARY_NAME}" STREQUAL "")
	message(STATUS "CLANG_BINARY_NAME wasn't specified (you can use this to search for Clang by a particular name).")
endif(NOT "${CLANG_BINARY_NAME}" STREQUAL "")

if(NOT "${CLANG_ROOT_DIR}" STREQUAL "")
	message(STATUS "Using CLANG_ROOT_DIR=${CLANG_ROOT_DIR}")
else(NOT "${CLANG_ROOT_DIR}" STREQUAL "")
	message(STATUS "CLANG_ROOT_DIR wasn't specified (you can use this to search for Clang in a particular path).")
endif(NOT "${CLANG_ROOT_DIR}" STREQUAL "")

# --- Dependencies

# Find LLVM (for code generation).
set(LLVM_REQ_COMPONENTS all-targets bitreader bitwriter core irreader jit linker mcjit nativecodegen native ipo option)
find_package(LLVM REQUIRED ${LLVM_REQ_COMPONENTS})

# Check LLVM version.
set(LLVM_VERSION_SIMPLE_STRING "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}")

if("${LLVM_VERSION_SIMPLE_STRING}" VERSION_LESS "3.3")
	message(FATAL_ERROR "LLVM version ${LLVM_VERSION_SIMPLE_STRING} is not supported (first supported version is 3.3).")
elseif("${LLVM_VERSION_SIMPLE_STRING}" VERSION_GREATER "3.6")
	message(WARNING "LLVM version ${LLVM_VERSION_SIMPLE_STRING} is not officially supported for this release (last supported version is 3.6).")
endif()

math(EXPR LOCIC_LLVM_VERSION ${LLVM_VERSION_MAJOR}*100+${LLVM_VERSION_MINOR})
message(STATUS "Using LLVM ${LLVM_VERSION_SIMPLE_STRING} (version integer is ${LOCIC_LLVM_VERSION}).")

add_definitions(
	-DLOCIC_LLVM_VERSION=${LOCIC_LLVM_VERSION}
)

# Also use LLVM_OBJ_ROOT here for custom builds of LLVM.
set(LOCIC_LLVM_INCLUDE_DIRS "${LLVM_INCLUDE_DIRS}" CACHE STRING "Set LLVM include directories.")
set(LOCIC_LLVM_LIBRARY_DIRS "${LLVM_LIBRARY_DIRS}" CACHE STRING "Set LLVM library directories.")

message(STATUS "    LLVM binary directory: ${LLVM_BINARY_DIR}")
message(STATUS "        llc path: ${LLVM_LLC_EXECUTABLE}")
message(STATUS "        llvm-dis path: ${LLVM_DIS_EXECUTABLE}")
message(STATUS "        llvm-link path: ${LLVM_LINK_EXECUTABLE}")
message(STATUS "        llvm-nm path: ${LLVM_NM_EXECUTABLE}")
message(STATUS "        opt path: ${LLVM_OPT_EXECUTABLE}")

message(STATUS "    LLVM include directories: ${LOCIC_LLVM_INCLUDE_DIRS}")
message(STATUS "        (if this is wrong, specify LOCIC_LLVM_INCLUDE_DIRS)")
include_directories(SYSTEM ${LOCIC_LLVM_INCLUDE_DIRS})

message(STATUS "    LLVM library directories: ${LOCIC_LLVM_LIBRARY_DIRS}")
message(STATUS "        (if this is wrong, specify LOCIC_LLVM_LIBRARY_DIRS)")
link_directories(${LOCIC_LLVM_LIBRARY_DIRS})

function(add_to_paths var extra)
	foreach(oldpath ${ARGN})
		list(APPEND newpaths "${oldpath}/${extra}")
	endforeach()
	set(${var} ${newpaths} PARENT_SCOPE)
endfunction()

# Workaround for Ubuntu LLVM 3.5 package putting DIBuilder.h
# in 'llvm/' directory rather than 'llvm/IR/'.
add_to_paths(LLVM_IR_INCLUDE_DIRS "llvm/IR" ${LOCIC_LLVM_INCLUDE_DIRS})
find_file(LLVM_DIBUILDER_H
	NAMES "DIBuilder.h"
	PATHS ${LLVM_IR_INCLUDE_DIRS}
	NO_DEFAULT_PATH
)

if(LLVM_DIBUILDER_H)
	message(STATUS "    Found DIBuilder.h in llvm/IR.")
	add_definitions(-DLLVM_DIBUILDER_IN_IR)
endif(LLVM_DIBUILDER_H)

# Workaround for Ubuntu LLVM 3.5 package putting Linker.h
# in 'llvm/' directory rather than 'llvm/Linker/'.
add_to_paths(LLVM_LINKER_INCLUDE_DIRS "llvm/Linker" ${LOCIC_LLVM_INCLUDE_DIRS})
find_path(LLVM_LINKER_H
	NAMES "Linker.h"
	PATHS ${LLVM_LINKER_INCLUDE_DIRS}
	NO_DEFAULT_PATH
)

if(LLVM_LINKER_H)
	message(STATUS "    Found Linker.h in llvm/Linker.")
	add_definitions(-DLLVM_LINKER_IN_LINKER)
endif(LLVM_LINKER_H)

# Build LLVM ABI
include(LLVMABI)

# --- Subdirectories

# All headers are in the /include directory.
include_directories (
	"${CMAKE_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/lib/CodeGen/llvm-abi/include"
)

set (STD_CONCURRENCY_API_FILES
	"${CMAKE_SOURCE_DIR}/stdlib/std/concurrency/api/condition_variable.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/concurrency/api/fiber.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/concurrency/api/message_queue.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/concurrency/api/mutex.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/concurrency/api/runnable.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/concurrency/api/thread.loci"
)

set (STD_CONCURRENCY_BINARY_FILES
	"${CMAKE_BINARY_DIR}/stdlib/std/concurrency/std_concurrency.bc"
)

set (STD_CONTAINER_API_FILES
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/circular_queue.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/counter.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/list.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/map.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/range.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/set.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/types.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/container/api/varray.loci"
)

set (STD_CONTAINER_BINARY_FILES
	"${CMAKE_BINARY_DIR}/stdlib/std/container/std_container.bc"
)

set (STD_EVENT_API_FILES
	"${CMAKE_SOURCE_DIR}/stdlib/std/event/api/generator.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/event/api/source.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/event/api/wait_set.loci"
)

set (STD_EVENT_BINARY_FILES
	"${CMAKE_BINARY_DIR}/stdlib/std/event/std_event.bc"
)

set (STD_MEMORY_API_FILES
	"${CMAKE_SOURCE_DIR}/stdlib/std/memory/api/alloc.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/memory/api/raw.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/memory/api/shared.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/memory/api/unique.loci"
)

set (STD_MEMORY_BINARY_FILES
	"${CMAKE_BINARY_DIR}/stdlib/std/memory/std_memory.bc"
)

set (STD_NETWORK_API_FILES
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/ip/address.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/ip/address_v4.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/tcp/connector.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/tcp/endpoint.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/tcp/listener.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/tcp/stream.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/network/api/tcp/sync_stream.loci"
)

set (STD_NETWORK_BINARY_FILES
	"${CMAKE_BINARY_DIR}/stdlib/std/network/std_network.bc"
)

set (STD_STRING_API_FILES
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/string.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/string_builder.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/string_iterator.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/string_range.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/unichar.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/utf8.loci"
	"${CMAKE_SOURCE_DIR}/stdlib/std/string/api/utf8_buffer.loci"
)

set (STD_STRING_BINARY_FILES
	"${CMAKE_BINARY_DIR}/stdlib/std/string/std_string.bc"
)

# 3rd-party library Loci bindings.
add_subdirectory(3rdparty)

# Documentation.
add_subdirectory(docs)

if(LOCIC_BUILD_EXAMPLES)
	# Loci examples that are build by the compiler.
	add_subdirectory(examples)
endif(LOCIC_BUILD_EXAMPLES)

# Compiler libraries (e.g. SemanticAnalysis).
add_subdirectory(lib)

# Language runtime.
add_subdirectory(runtime)

# Standard library.
add_subdirectory(stdlib)

# Compiler tests (e.g. that particular source files compile or don't compile).
add_subdirectory(test)

# Compiler tools (such as the 'locic' command line tool).
add_subdirectory(tools)

# Unit tests.
add_subdirectory(unittests)

# Utilities.
add_subdirectory(utils)

