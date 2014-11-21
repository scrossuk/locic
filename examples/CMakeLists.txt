find_package(LLVMBinaries REQUIRED)
find_package(ClangBinary REQUIRED)

set(LOCIC "${CMAKE_BINARY_DIR}/tools/locic")

macro(copy_source_file output_name name)
	get_filename_component(name_file "${name}" NAME)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name_file}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name_file}"
		MAIN_DEPENDENCY "${name}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
	set(${output_name} "${CMAKE_CURRENT_BINARY_DIR}/${name_file}")
endmacro(copy_source_file)

function(loci_module name output_name flags)
	set(${name}_Sources ${ARGN})
	set(${name}_BuildSources "")
	
	foreach(file_source_name ${${name}_Sources})
		copy_source_file(file_build_source_name ${file_source_name})
		list(APPEND ${name}_BuildSources ${file_build_source_name})
	endforeach(file_source_name)
	
	separate_arguments(flags)
	
	add_custom_command(OUTPUT ${output_name} ${output_name}.ast.txt ${output_name}.sem.txt ${output_name}.codegen.ll ${output_name}.opt.ll
		COMMAND # Run compiler.
			${LOCIC} ${flags} -o ${output_name} --ast-debug-file=${output_name}.ast.txt --sem-debug-file=${output_name}.sem.txt --codegen-debug-file=${output_name}.codegen.ll --opt-debug-file=${output_name}.opt.ll ${${name}_BuildSources}
		DEPENDS ${LOCIC} ${${name}_BuildSources}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name} ALL DEPENDS ${output_name})
endfunction(loci_module)

set(RUNTIME_MODULE "${CMAKE_BINARY_DIR}/runtime/loci-runtime.bc")

function(loci_link name binary_name flags primary_module)
	set(${name}_Sources ${ARGN})
	
	separate_arguments(flags)
	
	add_custom_command(OUTPUT ${binary_name} ${binary_name}_optimised
		COMMAND
			# Create a file of all symbols in the primary module.
			${LLVMBINARIES_NM_EXECUTABLE} -extern-only -just-symbol-name -defined-only "${primary_module}" > "${primary_module}.symbols.txt"
		COMMAND
			# Link bitcode files together.
			${LLVMBINARIES_LINK_EXECUTABLE} "${primary_module}" ${${name}_Sources} "${RUNTIME_MODULE}" -o "${binary_name}_linked_raw.bc"
		COMMAND
			# Internalize all dependency symbols in linked module.
			${LLVMBINARIES_OPT_EXECUTABLE} -internalize -internalize-public-api-file="${primary_module}.symbols.txt" -o "${binary_name}_linked.bc" "${binary_name}_linked_raw.bc"
		COMMAND
			# Run 'opt' multiple times for maximum optimisation.
			${LLVMBINARIES_OPT_EXECUTABLE} -O3 ${binary_name}_linked.bc -o ${binary_name}_opt0.bc &&
			${LLVMBINARIES_OPT_EXECUTABLE} -O3 ${binary_name}_opt0.bc -o ${binary_name}_opt1.bc &&
			${LLVMBINARIES_OPT_EXECUTABLE} -O3 ${binary_name}_opt1.bc -o ${binary_name}_final.bc &&
			
			# Disassemble for debugging.
			${LLVMBINARIES_DIS_EXECUTABLE} ${binary_name}_final.bc -o ${binary_name}_final.ll &&
			
			# Use 'llc' to generate non-optimised platform object file.
			${LLVMBINARIES_LLC_EXECUTABLE} -filetype=obj -o ${binary_name}.o ${binary_name}_linked.bc &&
			
			# Use 'clang' to generate non-optimised platform binary.
			${CLANGBINARY_EXECUTABLE} -o ${binary_name} ${binary_name}.o -lm -L${CMAKE_BINARY_DIR}/runtime -lloci-runtime ${flags} &&
			
			# Use 'llc' to generate optimised platform object file.
			${LLVMBINARIES_LLC_EXECUTABLE} -filetype=obj -o ${binary_name}_optimised.o ${binary_name}_final.bc &&
			
			# Use 'clang' to generate optimised platform binary.
			${CLANGBINARY_EXECUTABLE} -s -o ${binary_name}_optimised ${binary_name}_optimised.o -lm ${flags}
		DEPENDS ${primary_module} ${${name}_Sources} loci-runtime-bc
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	
	add_custom_target(${name} ALL DEPENDS ${binary_name})
endfunction(loci_link)

function(loci_example name binary_name flags)
	loci_module("${name}_Module" ${binary_name}.bc "" ${ARGN})
	loci_link("${name}" "${binary_name}" "${flags}" "${binary_name}.bc")
endfunction(loci_example)

add_subdirectory(Calculator)
add_subdirectory(ChainReversi)
add_subdirectory(CommandLine)
add_subdirectory(Events)
add_subdirectory(General)
add_subdirectory(Looper)
add_subdirectory(Network)
add_subdirectory(SplitModule)

