loci_module(Events_Module events.bc ""
	Events.loci
	main.loci
	MessageQueue.loci
	Threads.loci
	${STD_CONCURRENCY_API_FILES}
	${STD_CONTAINER_API_FILES}
	${STD_EVENT_API_FILES}
	${STD_STRING_API_FILES}
	)

loci_link(Events events "-pthread"
	events.bc
	${STD_CONCURRENCY_BINARY_FILES}
	${STD_EVENT_BINARY_FILES}
	${STD_STRING_BINARY_FILES}
	${STD_CONTAINER_BINARY_FILES}
	${STD_MEMORY_BINARY_FILES}
	)

add_dependencies(Events std.concurrency std.event std.string std.container)

