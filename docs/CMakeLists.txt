add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Language\ Design.tex COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Language\ Design.tex ${CMAKE_CURRENT_BINARY_DIR}/Language\ Design.tex MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Language\ Design.tex)

add_custom_command(OUTPUT Language\ Design.pdf Language\ Design.aux Language\ Design.log COMMAND pdflatex Language\ Design.tex DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Language\ Design.tex)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Module\ Loading.tex COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Module\ Loading.tex ${CMAKE_CURRENT_BINARY_DIR}/Module\ Loading.tex MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Module\ Loading.tex)

add_custom_command(OUTPUT Module\ Loading.pdf Module\ Loading.aux Module\ Loading.log COMMAND pdflatex Module\ Loading.tex DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Module\ Loading.tex)

add_custom_target(Documentation ALL DEPENDS Language\ Design.pdf Module\ Loading.pdf)
