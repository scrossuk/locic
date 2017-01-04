#include <Qt/qstring.h>

extern "C" {
	
	size_t MT1N7QStringF1N11__alignmask() {
		return alignof(QString) - 1;
	}
	
	size_t MT1N7QStringF1N8__sizeof() {
		return sizeof(QString);
	}
	
	void MT1N7QStringF1N13createCString(void* dest, const char* const cstring) {
		new(dest) QString(cstring);
	}
	
	void MT1N7QStringF1N9__destroy(void* string) {
		static_cast<QString*>(string)->~QString();
	}
	
	void MT1N7QStringF1N12implicitCopy(void* dest, void* string) {
		new(dest) QString(*(static_cast<const QString*>(string)));
	}
	
	void MT1N7QStringF1N4copy(void* dest, void* string) {
		new(dest) QString(*(static_cast<const QString*>(string)));
	}
	
	void MT1N7QStringF1N8__move(void* dest, void* string) {
		new(dest) QString(std::move(*(static_cast<QString*>(string))));
	}
	
}
