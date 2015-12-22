
add_subdirectory(Qt)
add_subdirectory(SFML)

set(LOCIC_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING
	"Loci Third Party Library Bindings Directory"
)

function(loci_get_3rdparty_unprefixed_api_files API_FILES MODULE_NAME)
	if ("${MODULE_NAME}" STREQUAL "SFML")
		loci_get_sfml_api_files(SFML_API_FILES)
		set(${API_FILES} "${SFML_API_FILES}" PARENT_SCOPE)
	elseif("${MODULE_NAME}" STREQUAL "Qt")
		loci_get_qt_api_files(QT_API_FILES)
		set(${API_FILES} "${QT_API_FILES}" PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Unknown 3rd-party module '${MODULE_NAME}'.")
	endif()
endfunction()

function(loci_get_3rdparty_api_files API_FILES MODULE_NAME)
	loci_get_3rdparty_unprefixed_api_files(UNPREFIXED_API_FILES "${MODULE_NAME}")
	set(PREFIXED_API_FILES "")
	foreach(UNPREFIX_API_FILE ${UNPREFIXED_API_FILES})
		list(APPEND PREFIXED_API_FILES "${LOCIC_THIRDPARTY_DIR}/${MODULE_NAME}/${UNPREFIX_API_FILE}")
	endforeach()
	set(${API_FILES} "${PREFIXED_API_FILES}" PARENT_SCOPE)
endfunction()
