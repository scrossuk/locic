set(LOCIC ${CMAKE_BINARY_DIR}/tools/locic)

macro(copy_source_file name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
endmacro(copy_source_file)

function(loci_module name output_name flags)
	set(${name}_Sources ${ARGN})
	set(${name}_BuildSources "")
	
	foreach(f ${${name}_Sources})
		copy_source_file(${f})
		list(APPEND ${name}_BuildSources ${CMAKE_CURRENT_BINARY_DIR}/${f})
	endforeach(f)
	
	separate_arguments(flags)
	
	add_custom_command(OUTPUT ${output_name} ${output_name}.ast.txt ${output_name}.sem.txt ${output_name}.codegen.ll ${output_name}.opt.ll
		COMMAND # Run compiler.
			${LOCIC} ${flags} -o ${output_name} --ast-debug-file=${output_name}.ast.txt --sem-debug-file=${output_name}.sem.txt --codegen-debug-file=${output_name}.codegen.ll --opt-debug-file=${output_name}.opt.ll ${${name}_BuildSources}
		DEPENDS ${LOCIC} ${${name}_BuildSources}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name} ALL DEPENDS ${output_name})
endfunction(loci_module)

add_subdirectory(std)

