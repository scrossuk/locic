loci_module(std.network std_network.bc ""
	src/ip/address.loci
	src/ip/address_v4.loci
	src/tcp/connector.loci
	src/tcp/endpoint.loci
	src/tcp/internal_stream.c
	src/tcp/stream.loci
	${STD_EVENT_API_FILES}
	)

