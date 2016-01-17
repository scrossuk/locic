find_package(Qt4)

if(NOT QT4_FOUND)
	message(WARNING "Failed to find Qt so can't build GUI example.")
	return()
endif()

loci_get_3rdparty_api_files(QT_API_FILES Qt)

loci_module(GUI_Module gui.bc ""
	main.loci
	${QT_API_FILES}
)

loci_link(GUI gui "-lQtCore -lQtGui -lstdc++"
	gui.bc
	"${PROJECT_BINARY_DIR}/3rdparty/Qt/qt.bc"
)

add_dependencies(GUI
	loci-qt-binding
)
