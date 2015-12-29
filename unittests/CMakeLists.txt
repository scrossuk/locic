# Unit tests.
# Run 'make test' or 'ctest' to execute these.

find_package(Threads REQUIRED)

include_directories(SYSTEM
	"${PROJECT_SOURCE_DIR}/utils/googletest/include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCIC_BUILD_WARNINGS}")

add_library(locic-unittestmain STATIC
	TestMain.cpp
)

function(add_unit_test name)
	add_executable(locic-unittest-${name}
		${ARGN}
	)
	
	target_link_libraries(locic-unittest-${name}
		locic-unittestmain
		locic-util-gtest
		locic-newparser
		locic-ast
		locic-lex
		locic-support
		${CMAKE_DL_LIBS}
		${CMAKE_THREAD_LIBS_INIT}
	)
	
	add_test(NAME UnitTest${name} COMMAND locic-unittest-${name})
endfunction()

add_subdirectory(Lex)
add_subdirectory(Parser)
add_subdirectory(Support)

