cmake_minimum_required (VERSION 2.8)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed.")
endif("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

enable_testing()

project(Locic)
include_directories ("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

add_subdirectory(docs)

add_subdirectory(examples)

add_subdirectory(Locic)

add_subdirectory(tests)
