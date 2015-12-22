#include <Qt/qapplication.h>

extern "C" {
	
	size_t MT1N12QApplicationF1N11__alignmask() {
		return alignof(QApplication) - 1;
	}
	
	size_t MT1N12QApplicationF1N8__sizeof() {
		return sizeof(QApplication);
	}
	
	void MT1N12QApplicationF1N6create(void* ptr, int* argc, char** argv) {
		new(ptr) QApplication(*argc, argv);
	}
	
	void MT1N12QApplicationF1N9__destroy(void* ptr) {
		static_cast<QApplication*>(ptr)->~QApplication();
	}
	
	int MT1N12QApplicationF1N4exec(void* ptr) {
		return static_cast<QApplication*>(ptr)->exec();
	}
	
}
