cmake_minimum_required (VERSION 2.8)

enable_testing()

project(Locic)
include_directories ("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

add_subdirectory(docs)

add_subdirectory(examples)

add_subdirectory(Locic)

add_subdirectory(tests)
