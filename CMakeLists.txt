cmake_minimum_required (VERSION 2.8)

message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

enable_testing()

project(Locic)

# Find LLVM (for code generation).
find_package(LLVM REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(LLVM_REQ_LIBS bitreader bitwriter core jit native ipo)

# Clang (for target information).
set(CLANG_REQ_LIBS clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization)

# Compiler flags.
add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings -Werror")

# Use C++11.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(CMAKE_BUILD_TYPE Debug)

include_directories (
	"${CMAKE_SOURCE_DIR}/include"
	)

add_subdirectory(docs)

add_subdirectory(examples)

add_subdirectory(lib)

add_subdirectory(tests)
