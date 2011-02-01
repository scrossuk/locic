#include "Allocate.h"
#include "Uint.h"

void * _Uint_(unsigned int val){
	LOCI_NEW(uint, Loci_Uint, _vtable_Uint);
	uint->value = val;
	return (void *) uint;
}

