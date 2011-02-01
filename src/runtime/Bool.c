#include "Allocate.h"
#include "Bool.h"

void * _Bool_(unsigned int val){
	LOCI_NEW(boolVal, Loci_Bool, _vtable_Bool);
	boolVal->value = val;
	return (void *) boolVal;
}

Loci_Bool * _Bool_not(Loci_Bool * this){
	LOCI_NEW(boolVal, Loci_Bool, _vtable_Bool);
	boolVal->value = this->value ? 0 : 1;
	return boolVal;
}

