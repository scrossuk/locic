find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_executable(FileCheck
	FileCheck.cpp
)

target_link_libraries(FileCheck
	${LLVM_LIBRARIES}
	tinfo
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)
