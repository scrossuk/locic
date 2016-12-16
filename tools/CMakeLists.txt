find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCIC_BUILD_WARNINGS}")

set(FILE_LISTS "")
set(FILE_LISTS "${FILE_LISTS}struct module_info {\n")
set(FILE_LISTS "${FILE_LISTS}    const char* name;\n")
set(FILE_LISTS "${FILE_LISTS}    const char* version;\n")
set(FILE_LISTS "${FILE_LISTS}    const char* const * api_files;\n")
set(FILE_LISTS "${FILE_LISTS}    const char* const * binary_files;\n")
set(FILE_LISTS "${FILE_LISTS}    const char* const * dependencies;\n")
set(FILE_LISTS "${FILE_LISTS}};\n\n")

foreach(std_module ${STD_MODULES})
	string(TOUPPER "${std_module}" std_module_upper)
	
	set(FILE_LISTS "${FILE_LISTS}const char* const std_${std_module}_api_files[] = {")
	foreach(std_module_api_file ${STD_${std_module_upper}_API_FILES})
		set(FILE_LISTS "${FILE_LISTS}\"${std_module_api_file}\", ")
	endforeach()
	set(FILE_LISTS "${FILE_LISTS}0, ")
	set(FILE_LISTS "${FILE_LISTS} };\n\n")
	
	set(FILE_LISTS "${FILE_LISTS}const char* const std_${std_module}_binary_files[] = {")
	foreach(std_module_binary_file ${STD_${std_module_upper}_BINARY_FILES})
		set(FILE_LISTS "${FILE_LISTS}\"${std_module_binary_file}\", ")
	endforeach()
	set(FILE_LISTS "${FILE_LISTS}0, ")
	set(FILE_LISTS "${FILE_LISTS} };\n\n")
	
	set(FILE_LISTS "${FILE_LISTS}const char* const std_${std_module}_dependencies[] = {")
	foreach(std_module_dependency ${STD_${std_module_upper}_DEPENDENCIES})
		set(FILE_LISTS "${FILE_LISTS}\"${std_module_dependency}\", ")
	endforeach()
	set(FILE_LISTS "${FILE_LISTS}0, ")
	set(FILE_LISTS "${FILE_LISTS} };\n\n")
	
	set(FILE_LISTS "${FILE_LISTS}const struct module_info std_${std_module}_info = {\n")
	set(FILE_LISTS "${FILE_LISTS}    .name = \"std.${std_module}\",\n")
	set(FILE_LISTS "${FILE_LISTS}    .version = \"0.1.0\",\n")
	set(FILE_LISTS "${FILE_LISTS}    .api_files = std_${std_module}_api_files,\n")
	set(FILE_LISTS "${FILE_LISTS}    .binary_files = std_${std_module}_binary_files,\n")
	set(FILE_LISTS "${FILE_LISTS}    .dependencies = std_${std_module}_dependencies,\n")
	set(FILE_LISTS "${FILE_LISTS} };\n\n")
endforeach()

set(FILE_LISTS "${FILE_LISTS}const struct module_info* const std_modules[] = {")
foreach(std_module ${STD_MODULES})
	set(FILE_LISTS "${FILE_LISTS}&std_${std_module}_info, ")
endforeach()
set(FILE_LISTS "${FILE_LISTS}0, ")
set(FILE_LISTS "${FILE_LISTS} };\n")

set(FILE_LISTS_PATH "${CMAKE_CURRENT_BINARY_DIR}/filelists.c")

file(WRITE "${FILE_LISTS_PATH}" "${FILE_LISTS}")

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
	"${FILE_LISTS_PATH}"
)

target_link_libraries(locic
	locic-semanalysis
	locic-codegen
	locic-parser
	locic-ast
	locic-lex
	locic-frontend
	locic-support
	loci-builtintypes
	loci-runtime
	${REQ_LIBS}
)

install(TARGETS locic
	DESTINATION bin
)

# Build vtable generator.
add_executable(loci-vtable
	loci-vtable.cpp
)

target_link_libraries(loci-vtable
	locic-semanalysis
	locic-codegen
	locic-parser
	locic-ast
	locic-lex
	locic-support
	${REQ_LIBS}
)

