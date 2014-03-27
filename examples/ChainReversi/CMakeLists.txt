file(COPY Data DESTINATION .)

loci_example(ChainReversi chainReversi "-lcsfml-graphics -lcsfml-window -lcsfml-system"
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
	SFML/RectangleShape.loci
	SFML/RenderWindow.loci
	SFML/Text.loci
	SFML/VideoMode.loci
	)

