find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

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
	set(_sources ${ARGN})
	set(_c_build_sources "")
	set(_loci_build_sources "")
	
	foreach(_file_source_name ${_sources})
		get_filename_component(_absolute_source_name "${_file_source_name}" REALPATH)
		if("${_file_source_name}" MATCHES "^(.+)\\.c(pp)?$")
			list(APPEND _c_build_sources "${_absolute_source_name}")
		elseif("${_file_source_name}" MATCHES "^(.+)\\.loci$")
			list(APPEND _loci_build_sources "${_absolute_source_name}")
		else()
			message (FATAL_ERROR "Unknown source file type for file '${_file_source_name}'.")
		endif()
	endforeach(_file_source_name)
	
	separate_arguments(flags)
	
	if(_loci_build_sources)
		set(_locimodule_output_name "${CMAKE_CURRENT_BINARY_DIR}/LociModule_${output_name}")
		
		add_custom_command(OUTPUT "${_locimodule_output_name}" ${output_name}.ast.txt ${output_name}.sem.txt ${output_name}.codegen.ll ${output_name}.opt.ll
			COMMAND # Run compiler.
				${LOCIC} ${flags} -o "${_locimodule_output_name}" --ast-debug-file=${output_name}.ast.txt --sem-debug-file=${output_name}.sem.txt --codegen-debug-file=${output_name}.codegen.ll --opt-debug-file=${output_name}.opt.ll ${_loci_build_sources}
			DEPENDS ${LOCIC} ${_loci_build_sources}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
	endif()
	
	# Use clang to build C source files into LLVM bitcode.
	if(_c_build_sources)
		set (CFLAGS ${LOCIC_C_WARNINGS})
		separate_arguments(CFLAGS)
		
		set(_cmodule_output_name "${CMAKE_CURRENT_BINARY_DIR}/CModule_${output_name}")
		set(_c_build_objects "")
		
		foreach(_file_source_path ${_c_build_sources})
			get_filename_component(_file_source_name "${_file_source_path}" NAME)
			set(_file_object_name "${CMAKE_CURRENT_BINARY_DIR}/CModule_${_file_source_name}.bc")
			list(APPEND _c_build_objects "${_file_object_name}")
			
			set(cmd_flags "${CFLAGS}" "${flags}")
			if("${_file_source_path}" MATCHES "^(.+)\\.cpp$")
				list(APPEND cmd_flags "-std=c++11")
			endif()
			add_custom_command(OUTPUT "${_file_object_name}"
				COMMAND
					${CLANG_EXECUTABLE} -o "${_file_object_name}" ${cmd_flags} -c -emit-llvm "${_file_source_path}"
				DEPENDS "${_file_source_path}"
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			)
		endforeach(_file_source_path)
		
		add_custom_command(
			OUTPUT "${_cmodule_output_name}"
			COMMAND ${LLVM_LINK_EXECUTABLE} -o "${_cmodule_output_name}" ${_c_build_objects}
			DEPENDS ${_c_build_objects}
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		)
	endif()
	
	if(_loci_build_sources AND _c_build_sources)
		# Link Loci and C modules together.
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
			COMMAND ${LLVM_LINK_EXECUTABLE} -o "${CMAKE_CURRENT_BINARY_DIR}/${output_name}" "${_cmodule_output_name}" "${_locimodule_output_name}"
			DEPENDS "${_cmodule_output_name}" "${_locimodule_output_name}"
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		)
	else()
		if(_loci_build_sources)
			add_custom_command(
				OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_locimodule_output_name}" "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				DEPENDS "${_locimodule_output_name}"
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
			)
		else()
			add_custom_command(
				OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_cmodule_output_name}" "${CMAKE_CURRENT_BINARY_DIR}/${output_name}"
				DEPENDS "${_cmodule_output_name}"
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
			)
		endif()
	endif()
	
	add_custom_target(${name} ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${output_name}")
endmacro(loci_module)
