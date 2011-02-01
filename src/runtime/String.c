#include <stdio.h>

#include "Allocate.h"
#include "String.h"

void * _String_(unsigned int len, const char * str){
	LOCI_NEW(string, Loci_String, _vtable_String);
	string->data = str;
	string->length = len;
	return (void *) string;
}

Loci_String * _String(Loci_String * str){
	return str;
}

Loci_String * _String_print(void * vthis){
	Loci_String * this;
	this = (Loci_String *) vthis;
	printf("String(%u): %s\n", this->length, this->data);
	return this;
}

Loci_String * _String_OpPlus(Loci_String * this, Loci_String * other){
	char * data;
	data = (char *) Loci_Allocate(this->length + other->length + 1);
	
	unsigned int i;
	
	for(i = 0; i < this->length; ++i){
		data[i] = this->data[i];
	}
	
	for(i = 0; i < other->length; ++i){
		data[this->length + i] = other->data[i];
	}
	
	data[this->length + other->length] = 0;
	
	LOCI_NEW(str, Loci_String, _vtable_String);
	str->data = data;
	str->length = this->length + other->length;
	return str;
}

