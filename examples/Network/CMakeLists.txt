loci_module(Network_Module network.bc ""
	main.loci
	TCP.loci
	${STD_NETWORK_API_FILES}
	${STD_CONTAINER_API_FILES}
	${STD_EVENT_API_FILES}
	${STD_STRING_API_FILES}
	)

loci_link(Network network ""
	network.bc
	${STD_NETWORK_BINARY_FILES}
	${STD_EVENT_BINARY_FILES}
	${STD_STRING_BINARY_FILES}
	${STD_CONTAINER_BINARY_FILES}
	${STD_MEMORY_BINARY_FILES}
	)

add_dependencies(Network std.network std.event std.string std.container)

