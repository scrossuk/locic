# Utility function (copied from LLVM).
function (add_sphinx_target builder)
	set(SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${builder}")
	set(SPHINX_DOC_TREE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
	set(SPHINX_TARGET_NAME docs-${builder})
	
	set(SPHINX_BUILDER_TARGET_NAME docs-${builder})
	
	file(GLOB SPHINX_INPUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.rst")
	list(APPEND SPHINX_INPUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/conf.py")
	
	if ("${builder}" STREQUAL "pdf")
		# PDF needs special handling since Sphinx will generate
		# LaTeX documentation which then needs to be converted
		# into a PDF document.
		add_custom_command(
			OUTPUT "${SPHINX_BUILD_DIR}"
			COMMAND ${CMAKE_COMMAND}
				-E remove_directory
				"${SPHINX_BUILD_DIR}"
			# Build Latex documentation.
			COMMAND ${SPHINX_EXECUTABLE}
				-b "latex"
				-d "${SPHINX_DOC_TREE_DIR}"
				-q # Quiet: no output other than errors and warnings.
				"${CMAKE_CURRENT_SOURCE_DIR}" # Source
				"${SPHINX_BUILD_DIR}" # Output
			# Run make to generate the PDF documentation.
			# (Suppressing Makefile output since it is very verbose,
			# and we can probably assume that Sphinx will produce
			# valid LaTex).
			COMMAND make -C "${CMAKE_CURRENT_BINARY_DIR}/pdf" all-pdf > /dev/null
			DEPENDS ${SPHINX_INPUT_FILES}
			COMMENT
				"Generating PDF (from LaTeX) Sphinx documentation")
	else()
		add_custom_command(
			OUTPUT "${SPHINX_BUILD_DIR}"
			COMMAND ${CMAKE_COMMAND}
				-E remove_directory
				"${SPHINX_BUILD_DIR}"
			COMMAND ${SPHINX_EXECUTABLE}
				-b ${builder}
				-d "${SPHINX_DOC_TREE_DIR}"
				-q # Quiet: no output other than errors and warnings.
				"${CMAKE_CURRENT_SOURCE_DIR}" # Source
				"${SPHINX_BUILD_DIR}" # Output
			DEPENDS ${SPHINX_INPUT_FILES}
			COMMENT
				"Generating ${builder} Sphinx documentation")
	endif()
	
	add_custom_target(${SPHINX_BUILDER_TARGET_NAME} ALL DEPENDS "${SPHINX_BUILD_DIR}")
	
	# When "clean" target is run, remove the Sphinx build directory
	set_property(DIRECTORY APPEND PROPERTY
		ADDITIONAL_MAKE_CLEAN_FILES
		"${SPHINX_BUILD_DIR}")

	# We need to remove ${SPHINX_DOC_TREE_DIR} when make clean is run
	# but we should only add this path once
	get_property(_CURRENT_MAKE_CLEAN_FILES
		DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES)
	list(FIND _CURRENT_MAKE_CLEAN_FILES "${SPHINX_DOC_TREE_DIR}" _INDEX)
	if (_INDEX EQUAL -1)
		set_property(DIRECTORY APPEND PROPERTY
			ADDITIONAL_MAKE_CLEAN_FILES
			"${SPHINX_DOC_TREE_DIR}")
	endif()

	add_dependencies(sphinx ${SPHINX_TARGET_NAME})
endfunction()

find_package(Sphinx REQUIRED)

add_custom_target(sphinx ALL)

if (${SPHINX_OUTPUT_HTML})
	add_sphinx_target(html)
endif()

if (${SPHINX_OUTPUT_MAN})
	add_sphinx_target(man)
endif()

if (${SPHINX_OUTPUT_PDF})
	add_sphinx_target(pdf)
endif()

