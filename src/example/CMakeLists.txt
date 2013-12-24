set(LOCIC ${CMAKE_CURRENT_BINARY_DIR}/../Locic/locic)

set(EXAMPLE_SOURCES
	Example1.loci
	Example2.loci
	)

macro(copy_file name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
endmacro(copy_file)

set(EXAMPLE_BUILD_SOURCES "")

foreach(f ${EXAMPLE_SOURCES})
	copy_file(${f})
	list(APPEND EXAMPLE_BUILD_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${f})
endforeach(f)

add_custom_command(OUTPUT exampleProgram ast.txt sem.txt codegen.ll opt.ll clangopt.ll
		COMMAND # Run compiler.
			${LOCIC} -O3 -o exampleProgram.bc --ast-debug-file=ast.txt --sem-debug-file=sem.txt --codegen-debug-file=codegen.ll --opt-debug-file=opt.ll ${EXAMPLE_BUILD_SOURCES} &&
			
			# Use clang to optimise, since it currently does
			# much better than locic.
			clang -S -emit-llvm -O3 -o clangopt.ll opt.ll &&
			
			# Use clang to generate platform binary.
			clang -o exampleProgram clangopt.ll -lm
		DEPENDS ${LOCIC} ${EXAMPLE_BUILD_SOURCES})

add_custom_target(ExampleProgram ALL DEPENDS exampleProgram)

