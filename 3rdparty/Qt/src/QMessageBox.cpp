#include <Qt/qmessagebox.h>

extern "C" {
	
	size_t MT1N11QMessageBoxF1N11__alignmask() {
		return alignof(QMessageBox) - 1;
	}
	
	size_t MT1N11QMessageBoxF1N8__sizeof() {
		return sizeof(QMessageBox);
	}
	
	void MT1N11QMessageBoxF1N6create(void* ptr) {
		new(ptr) QMessageBox();
	}
	
	void MT1N11QMessageBoxF1N9__destroy(void* ptr) {
		static_cast<QMessageBox*>(ptr)->~QMessageBox();
	}
	
	void MT1N11QMessageBoxF1N4show(void* ptr) {
		static_cast<QMessageBox*>(ptr)->show();
	}
	
	void MT1N11QMessageBoxF1N7setText(void* ptr, void* string) {
		static_cast<QMessageBox*>(ptr)->setText(*(static_cast<const QString*>(string)));
	}
	
}
