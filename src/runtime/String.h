#ifndef LOCIRUNTIME_STRING_H
#define LOCIRUNTIME_STRING_H

extern void ** _vtable_String;

typedef struct Loci_String{
	const char * data;
	unsigned int length;
} Loci_String;

void * _String_(unsigned int, const char *);

#endif
