#include <Qt/qstring.h>

extern "C" {
	
	size_t MT1N7QStringF1N11__alignmask() {
		return alignof(QString) - 1;
	}
	
	size_t MT1N7QStringF1N8__sizeof() {
		return sizeof(QString);
	}
	
	void MT1N7QStringF1N13createCString(void* ptr, const char* const cstring) {
		new(ptr) QString(cstring);
	}
	
	void MT1N7QStringF1N9__destroy(void* ptr) {
		static_cast<QString*>(ptr)->~QString();
	}
	
	void  MT1N7QStringF1N12implicitCopy(void* ptr, void* string) {
		new(ptr) QString(*(static_cast<const QString*>(string)));
	}
	
	void  MT1N7QStringF1N4copy(void* ptr, void* string) {
		new(ptr) QString(*(static_cast<const QString*>(string)));
	}
	
	void  MT1N7QStringF1N8__moveto(void* ptr, uint8_t* dest, size_t offset) {
		new(dest + offset) QString(std::move(*(static_cast<QString*>(ptr))));
	}
	
}
