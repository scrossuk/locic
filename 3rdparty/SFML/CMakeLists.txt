
function(loci_get_sfml_api_files SFML_API_FILES)
	set(${SFML_API_FILES}
		api/CircleShape.loci
		api/Clock.loci
		api/Color.loci
		api/Event.loci
		api/Font.loci
		api/Mouse.loci
		api/Rect.loci
		api/RectangleShape.loci
		api/RenderWindow.loci
		api/Text.loci
		api/Vector.loci
		api/VideoMode.loci
		api/View.loci
		PARENT_SCOPE
	)
endfunction()
