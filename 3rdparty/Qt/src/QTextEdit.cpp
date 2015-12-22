#include <Qt/qtextedit.h>

extern "C" {
	
	size_t MT1N9QTextEditF1N11__alignmask() {
		return alignof(QTextEdit) - 1;
	}
	
	size_t MT1N9QTextEditF1N8__sizeof() {
		return sizeof(QTextEdit);
	}
	
	void MT1N9QTextEditF1N6create(void* ptr) {
		new(ptr) QTextEdit();
	}
	
	void MT1N9QTextEditF1N9__destroy(void* ptr) {
		static_cast<QTextEdit*>(ptr)->~QTextEdit();
	}
	
	void MT1N9QTextEditF1N4show(void* ptr) {
		static_cast<QTextEdit*>(ptr)->show();
	}
	
	void MT1N9QTextEditF1N7setText(void* ptr, void* string) {
		static_cast<QTextEdit*>(ptr)->setText(*(static_cast<const QString*>(string)));
	}
	
}
