set(LOCIC ${CMAKE_BINARY_DIR}/tools/locic)

macro(copy_example_file name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
endmacro(copy_example_file)

function(loci_example name binary_name flags)
	set(${name}Sources ${ARGN})
	set(${name}BuildSources "")
	
	foreach(f ${${name}Sources})
		copy_example_file(${f})
		list(APPEND ${name}BuildSources ${CMAKE_CURRENT_BINARY_DIR}/${f})
	endforeach(f)
	
	separate_arguments(flags)
	
	add_custom_command(OUTPUT ${binary_name} ${binary_name}_optimised ast.txt sem.txt codegen.ll opt.ll clangopt.ll
		COMMAND # Run compiler.
			${LOCIC} -o ${binary_name}.bc --ast-debug-file=ast.txt --sem-debug-file=sem.txt --codegen-debug-file=codegen.ll --opt-debug-file=opt.ll ${${name}BuildSources} &&
			
			# Run 'opt' multiple times for maximum optimisation.
			opt -S -O3 codegen.ll -o opt0.ll &&
			opt -S -O3 opt0.ll -o opt1.ll &&
			opt -S -O3 opt1.ll -o opt_final.ll &&
			
			# Use clang to generate non-optimised platform binary.
			clang -o ${binary_name} codegen.ll -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception ${flags} &&
			
			# Use clang to generate optimised platform binary.
			clang -o ${binary_name}_optimised opt0.ll -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception ${flags}
		DEPENDS ${LOCIC} ${${name}BuildSources} loci-runtime-exception
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name}Example ALL DEPENDS ${binary_name})
endfunction(loci_example)

add_subdirectory(Calculator)
add_subdirectory(ChainReversi)
add_subdirectory(General)

