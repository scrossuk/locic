set(LOCIC ${CMAKE_BINARY_DIR}/tools/locic)

macro(copy_source_file output_name name)
	get_filename_component(name_file "${name}" NAME)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name_file}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name_file}"
		MAIN_DEPENDENCY "${name}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
	set(${output_name} "${CMAKE_CURRENT_BINARY_DIR}/${name_file}")
endmacro(copy_source_file)

macro(loci_module name output_name flags)
	set(${name}_Sources ${ARGN})
	set(${name}_BuildSources "")
	
	foreach(file_source_name ${${name}_Sources})
		copy_source_file(file_build_source_name ${file_source_name})
		list(APPEND ${name}_BuildSources ${file_build_source_name})
	endforeach(file_source_name)
	
	separate_arguments(flags)
	
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_name}" ${output_name}.ast.txt ${output_name}.sem.txt ${output_name}.codegen.ll ${output_name}.opt.ll
		COMMAND # Run compiler.
			${LOCIC} ${flags} -o ${output_name} --ast-debug-file=${output_name}.ast.txt --sem-debug-file=${output_name}.sem.txt --codegen-debug-file=${output_name}.codegen.ll --opt-debug-file=${output_name}.opt.ll ${${name}_BuildSources}
		DEPENDS ${LOCIC} ${${name}_BuildSources}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name} ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${output_name}")
endmacro(loci_module)

add_subdirectory(std)

