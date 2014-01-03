macro(add_latex name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.tex"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}.tex" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tex"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}.tex")
	add_custom_command(OUTPUT "${name}.pdf"
		COMMAND rubber "-d" "${name}.tex"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_BINARY_DIR}/${name}.tex")
endmacro(add_latex)

add_latex("LanguageDesign")
add_latex("ModuleLoading")

add_custom_target(Documentation ALL DEPENDS
	"LanguageDesign.pdf"
	"ModuleLoading.pdf")
