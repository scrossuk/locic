find_package(CSFML 2.0 COMPONENTS graphics window system)

if(NOT CSFML_FOUND)
	message(WARNING "Failed to find CSFML so can't build ChainReversi example.")
	return()
endif()

file(COPY Data DESTINATION .)

loci_get_3rdparty_api_files(SFML_API_FILES SFML)

loci_module(ChainReversi_Module chainReversi.bc ""
	main.loci
	ANN/Layer.loci
	ANN/Net.loci
	ANN/Neuron.loci
	CR/AIPlayer.loci
	CR/ANNPlayer.loci
	CR/Board.loci
	CR/BoardAnalysis.loci
	CR/HumanPlayer.loci
	CR/Player.loci
	CR/Value.loci
	${SFML_API_FILES}
	${STD_CONTAINER_API_FILES}
	${STD_STRING_API_FILES}
)

loci_link(ChainReversi chainReversi "-lcsfml-graphics -lcsfml-window -lcsfml-system"
	chainReversi.bc
	${STD_CONTAINER_BINARY_FILES}
	${STD_STRING_BINARY_FILES}
	${STD_MEMORY_BINARY_FILES}
)

add_dependencies(ChainReversi std.container std.memory std.string)
