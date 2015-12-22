find_package(Qt4)

if(NOT QT4_FOUND)
	message(WARNING "Failed to find Qt so can't build GUI example.")
	return()
endif()

include(LociModule)

function(loci_get_qt_api_files QT_API_FILES)
	set(${QT_API_FILES}
		api/QApplication.loci
		api/QMessageBox.loci
		api/QString.loci
		api/QTextEdit.loci
		PARENT_SCOPE
	)
endfunction()

loci_module(loci-qt-binding qt.bc "-I/usr/include/qt4"
	src/QApplication.cpp
	src/QMessageBox.cpp
	src/QString.cpp
	src/QTextEdit.cpp
)
