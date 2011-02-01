#ifndef LOCIRUNTIME_UINT_H
#define LOCIRUNTIME_UINT_H

extern void ** _vtable_Uint;

typedef struct Loci_Uint{
	unsigned int value;
} Loci_Uint;

void * _Uint_(unsigned int);

#endif
