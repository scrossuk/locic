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
	imports/string.loci
	)

set(STDLIB "${PROJECT_BINARY_DIR}/stdlib")

loci_link(ChainReversi chainReversi "-lcsfml-graphics -lcsfml-window -lcsfml-system"
	chainReversi.bc
	${STDLIB}/std/memory/std_memory.bc
	${STDLIB}/std/string/std_string.bc
	)

add_dependencies(ChainReversi std.memory std.string)

