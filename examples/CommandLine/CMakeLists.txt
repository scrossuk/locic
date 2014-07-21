loci_module(CommandLine_Module commandLine.bc ""
	CommandLine.loci
	${STD_CONTAINER_API_FILES}
	)

loci_link(CommandLine commandLine ""
	commandLine.bc
	${STD_CONTAINER_BINARY_FILES}
	${STD_MEMORY_BINARY_FILES}
	)

add_dependencies(CommandLine std.container)

