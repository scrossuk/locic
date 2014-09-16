find_package(LLVMBinaries REQUIRED)
find_package(ClangBinary REQUIRED)

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
	set(${name}_CBuildSources "")
	set(${name}_LociBuildSources "")
	
	foreach (file_source_name ${${name}_Sources})
		copy_source_file(file_build_source_name ${file_source_name})
		if ("${file_source_name}" MATCHES "^(.+)\\.c$")
			list(APPEND ${name}_CBuildSources ${file_build_source_name})
		elseif ("${file_source_name}" MATCHES "^(.+)\\.loci$")
			list(APPEND ${name}_LociBuildSources ${file_build_source_name})
		else ()
			message (FATAL_ERROR "Unknown source file type for file '${file_source_name}'.")
		endif ()
	endforeach (file_source_name)
	
	separate_arguments(flags)
	
	set (LOCIMODULE_OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/LociModule_${output_name}")
	
	add_custom_command(OUTPUT "${LOCIMODULE_OUTPUT_NAME}" ${output_name}.ast.txt ${output_name}.sem.txt ${output_name}.codegen.ll ${output_name}.opt.ll
		COMMAND # Run compiler.
			${LOCIC} ${flags} -o ${LOCIMODULE_OUTPUT_NAME} --ast-debug-file=${output_name}.ast.txt --sem-debug-file=${output_name}.sem.txt --codegen-debug-file=${output_name}.codegen.ll --opt-debug-file=${output_name}.opt.ll ${${name}_LociBuildSources}
		DEPENDS ${LOCIC} ${${name}_LociBuildSources}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	# Use clang to build C source files into LLVM bitcode.
	if (${name}_CBuildSources)
		set (CMODULE_OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/CModule_${output_name}")
		
		set (CFLAGS ${LOCIC_C_WARNINGS})
		separate_arguments(CFLAGS)
		
		set(${name}_CBuildObjects "")
		
		foreach (file_source_name ${${name}_CBuildSources})
			set(file_object_name "${file_source_name}.bc")
			list(APPEND ${name}_CBuildObjects "${file_object_name}")
			add_custom_command(OUTPUT "${file_object_name}"
				COMMAND
					${CLANGBINARY_EXECUTABLE} -o "${file_object_name}" ${CFLAGS} -c -emit-llvm "${file_source_name}"
				DEPENDS ${file_source_name}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
		endforeach (file_source_name)
		
		add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				COMMAND
					${LLVMBINARIES_LINK_EXECUTABLE} -o "${CMAKE_CURRENT_BINARY_DIR}/${output_name}" ${${name}_CBuildObjects} "${LOCIMODULE_OUTPUT_NAME}"
				DEPENDS ${${name}_CBuildObjects} "${LOCIMODULE_OUTPUT_NAME}"
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	else ()
		add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				COMMAND
					${CMAKE_COMMAND} -E copy_if_different "${LOCIMODULE_OUTPUT_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				DEPENDS "${LOCIMODULE_OUTPUT_NAME}"
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	endif ()
	
	add_custom_target(${name} ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${output_name}")
endmacro(loci_module)

add_subdirectory(std)

