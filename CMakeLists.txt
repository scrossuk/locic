cmake_minimum_required (VERSION 2.8)

project(Locic)
include_directories ("${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/src")
add_subdirectory(docs)
add_subdirectory(src)
