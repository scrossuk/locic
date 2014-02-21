set(LOCIC ${CMAKE_BINARY_DIR}/tools/locic)

macro(copy_example_file name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
endmacro(copy_example_file)

function(loci_example name binary_name)
	set(${name}Sources ${ARGN})
	set(${name}BuildSources "")
	
	foreach(f ${${name}Sources})
		copy_example_file(${f})
		list(APPEND ${name}BuildSources ${CMAKE_CURRENT_BINARY_DIR}/${f})
	endforeach(f)
	
	add_custom_command(OUTPUT ${binary_name} ast.txt sem.txt codegen.ll opt.ll clangopt.ll
		COMMAND # Run compiler.
			${LOCIC} -O3 -o ${binary_name}.bc --ast-debug-file=ast.txt --sem-debug-file=sem.txt --codegen-debug-file=codegen.ll --opt-debug-file=opt.ll ${${name}BuildSources} &&
			
			# Use clang to optimise, since it currently does
			# much better than locic.
			clang -S -emit-llvm -O3 -o clangopt.ll opt.ll &&
			
			# Use clang to generate platform binary.
			clang -o ${binary_name} clangopt.ll -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception
		DEPENDS ${LOCIC} ${${name}BuildSources} loci-runtime-exception
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name}Example ALL DEPENDS ${binary_name})
endfunction(loci_example)

add_subdirectory(Calculator)
add_subdirectory(Exception)
add_subdirectory(General)

