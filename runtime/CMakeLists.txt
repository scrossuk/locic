set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

add_library(loci-runtime STATIC
	assert.cpp
	exception.cpp
	)

