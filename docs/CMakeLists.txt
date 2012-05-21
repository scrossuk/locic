macro(add_latex name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.tex"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}.tex" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tex"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}.tex")
	add_custom_command(OUTPUT "${name}.pdf" "${name}.aux" "${name}.log" "${name}.lof" "${name}.toc" "${name}.bbl" "${name}.blg" "${name}.out" "${name}.nav" "${name}.snm" COMMAND rubber "-d" "${name}.tex"
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.tex")
endmacro(add_latex)

add_latex("Language Design")
add_latex("Module Loading")

add_custom_target(Documentation ALL DEPENDS
	"Language Design.pdf"
	"Module Loading.pdf")
