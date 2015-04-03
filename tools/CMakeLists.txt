find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCIC_BUILD_WARNINGS}")

set(REQ_LIBS
	${LLVMABI_LIBRARIES}
	${LLVM_LIBRARIES}
	boost_filesystem
	boost_program_options
	boost_regex
	boost_system
	tinfo
	${ZLIB_LIBRARIES}
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

# Build compiler.
add_executable(locic
	locic.cpp
)

target_link_libraries(locic
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	locic-support
	loci-builtintypes
	${REQ_LIBS}
)

# Build test tool.
add_executable(loci-test
	loci-test.cpp
)

target_link_libraries(loci-test
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	locic-support
	loci-builtintypes
	loci-runtime
	${REQ_LIBS}
)

# Build vtable generator.
add_executable(loci-vtable
	loci-vtable.cpp
)

target_link_libraries(loci-vtable
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-support
	${REQ_LIBS}
)

