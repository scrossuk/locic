#ifndef LOCIRUNTIME_BOOL_H
#define LOCIRUNTIME_BOOL_H

extern void ** _vtable_Bool;

typedef struct Loci_Bool{
	unsigned int value;
} Loci_Bool;

void * _Bool_(unsigned int);

#endif
