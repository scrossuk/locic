find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

if("${LLVM_VERSION_SIMPLE_STRING}" VERSION_LESS "3.8")
add_executable(FileCheck FileCheck_up_to_llvm_3_7.cpp)
elseif("${LLVM_VERSION_SIMPLE_STRING}" VERSION_EQUAL "3.8")
add_executable(FileCheck FileCheck_llvm_3_8.cpp)
else()
add_executable(FileCheck FileCheck_llvm_3_9.cpp)
endif()

target_link_libraries(FileCheck
	${LLVM_LIBRARIES}
	tinfo
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)
