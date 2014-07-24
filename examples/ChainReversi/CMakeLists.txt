file(COPY Data DESTINATION .)

loci_module(ChainReversi_Module chainReversi.bc ""
	main.loci
	CR/AIPlayer.loci
	CR/Board.loci
	CR/BoardAnalysis.loci
	CR/HumanPlayer.loci
	CR/Player.loci
	CR/Value.loci
	SFML/CircleShape.loci
	SFML/Clock.loci
	SFML/Color.loci
	SFML/Event.loci
	SFML/Font.loci
	SFML/Mouse.loci
	SFML/Rect.loci
	SFML/RectangleShape.loci
	SFML/RenderWindow.loci
	SFML/Text.loci
	SFML/Vector.loci
	SFML/VideoMode.loci
	SFML/View.loci
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

