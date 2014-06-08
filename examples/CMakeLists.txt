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
			opt -O3 ${binary_name}.bc -o ${binary_name}_opt0.bc &&
			opt -O3 ${binary_name}_opt0.bc -o ${binary_name}_opt1.bc &&
			opt -O3 ${binary_name}_opt1.bc -o ${binary_name}_opt_final.bc &&
			
			# Use clang to generate non-optimised platform binary.
			clang -o ${binary_name} ${binary_name}.bc -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception ${flags} &&
			
			# Use clang to generate optimised platform binary.
			clang -o ${binary_name}_optimised ${binary_name}_opt_final.bc -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception ${flags}
		DEPENDS ${LOCIC} ${${name}BuildSources} loci-runtime-exception
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name}Example ALL DEPENDS ${binary_name})
endfunction(loci_example)

add_subdirectory(Calculator)
add_subdirectory(ChainReversi)
add_subdirectory(General)

