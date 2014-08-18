add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
	COMMAND sphinx-build -b html -d "${CMAKE_CURRENT_BINARY_DIR}/doctrees" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/html"
	MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/index.rst")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/latex/LociCompilerTools.pdf"
	COMMAND sphinx-build -b latex -d "${CMAKE_CURRENT_BINARY_DIR}/doctrees" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/latex" &&
		make -C "${CMAKE_CURRENT_BINARY_DIR}/latex" all-pdf
	MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/index.rst")

add_custom_target(Documentation ALL DEPENDS
	"${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
	"${CMAKE_CURRENT_BINARY_DIR}/latex/LociCompilerTools.pdf"
	)
