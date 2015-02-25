
include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(locic-util-gtest STATIC
	src/gtest-all.cc
)

