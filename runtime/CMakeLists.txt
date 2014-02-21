set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

add_library(loci-runtime-exception STATIC
	exception.cpp
	)

