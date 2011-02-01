#include "Allocate.h"
#include "Float.h"

void * _Float_(unsigned int val){
	LOCI_NEW(floatVal, Loci_Float, _vtable_Float);
	floatVal->value = val;
	return (void *) floatVal;
}

