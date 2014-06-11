set(LOCIC ${CMAKE_BINARY_DIR}/tools/locic)

macro(copy_example_file name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
endmacro(copy_example_file)

function(loci_build output_name flags)
	set(${name}Sources ${ARGN})
	set(${name}BuildSources "")
	
	foreach(f ${${name}Sources})
		copy_example_file(${f})
		list(APPEND ${name}BuildSources ${CMAKE_CURRENT_BINARY_DIR}/${f})
	endforeach(f)
	
	separate_arguments(flags)
	
	add_custom_command(OUTPUT ${output_name} ${output_name}.ast.txt ${output_name}.sem.txt ${output_name}.codegen.ll ${output_name}.opt.ll
		COMMAND # Run compiler.
			${LOCIC} ${flags} -o ${output_name} --ast-debug-file=${output_name}.ast.txt --sem-debug-file=${output_name}.sem.txt --codegen-debug-file=${output_name}.codegen.ll --opt-debug-file=${output_name}.opt.ll ${${name}BuildSources}
		DEPENDS ${LOCIC} ${${name}BuildSources}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${output_name}_Build ALL DEPENDS ${output_name})
endfunction(loci_build)

function(loci_make_executable binary_name flags)
	set(${binary_name}_Sources ${ARGN})
	
	add_custom_command(OUTPUT ${binary_name} ${binary_name}_optimised
		COMMAND # Link bitcode files together.
			llvm-link ${${binary_name}_Sources} -o ${binary_name}_linked.bc &&
			
			# Run 'opt' multiple times for maximum optimisation.
			opt -O3 ${binary_name}_linked.bc -o ${binary_name}_opt0.bc &&
			opt -O3 ${binary_name}_opt0.bc -o ${binary_name}_opt1.bc &&
			opt -O3 ${binary_name}_opt1.bc -o ${binary_name}_final.bc &&
			
			# Disassemble for debugging.
			llvm-dis ${binary_name}_final.bc -o ${binary_name}_final.ll &&
			
			# Use clang to generate non-optimised platform binary.
			clang -o ${binary_name} ${binary_name}_linked.bc -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception ${flags} &&
			
			# Use clang to generate optimised platform binary.
			clang -o ${binary_name}_optimised ${binary_name}_final.bc -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime-exception ${flags}
		DEPENDS ${${binary_name}_Sources} loci-runtime-exception
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${binary_name}_Executable ALL DEPENDS ${binary_name})
endfunction(loci_make_executable)

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
add_subdirectory(Looper)
add_subdirectory(SplitModule)

