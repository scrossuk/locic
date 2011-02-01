#include "Call.h"

#define LOCI_VAR(type, name, value) type name; name = ((type) value)

unsigned int Loci_Hash(const char * name){
	unsigned int v = 0;
	unsigned int i;
	for(i = 0; name[i] != 0; ++i){
		v += name[i] * (i + 1);
	}
	return i % 32;
}

typedef void * (*Method0)(void *);
typedef void * (*Method1)(void *, void *);
typedef void * (*Method2)(void *, void *, void *);
typedef void * (*Method3)(void *, void *, void *, void *);
typedef void * (*Method4)(void *, void *, void *, void *, void *);
typedef void * (*Method5)(void *, void *, void *, void *, void *, void *);
typedef void * (*Method6)(void *, void *, void *, void *, void *, void *, void *);

void * Loci_CallHashed0(void * object, unsigned int hash){
	void ** vtable = *(((void ***) object) - 1);
	LOCI_VAR(Method0, method, vtable[hash]);
	return method(object);
}

void * Loci_CallHashed1(void * object, unsigned int hash, void * p0){
	void ** vtable = *(((void ***) object) - 1);
	LOCI_VAR(Method1, method, vtable[hash]);
	return method(object, p0);
}

void * Loci_CallHashed2(void * object, unsigned int hash, void * p0, void * p1){
	void ** vtable = *(((void ***) object) - 1);
	LOCI_VAR(Method2, method, vtable[hash]);
	return method(object, p0, p1);
}

void * Loci_Call0(void * object, const char * name){
	return Loci_CallHashed0(object, Loci_Hash(name));
}

void * Loci_Call1(void * object, const char * name, void * p0){
	return Loci_CallHashed1(object, Loci_Hash(name), p0);
}

void * Loci_Call2(void * object, const char * name, void * p0, void * p1){
	return Loci_CallHashed2(object, Loci_Hash(name), p0, p1);
}

