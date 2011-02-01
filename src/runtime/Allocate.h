#ifndef LOCIRUNTIME_ALLOCATE_H
#define LOCIRUNTIME_ALLOCATE_H

#include <stdlib.h>

#define LOCI_NEW(name, type, vtable) type * name; { void ** ___tmp; ___tmp = ((void **) Loci_Allocate(sizeof(void *) + sizeof(type))); ___tmp[0] = (void *) vtable; name = (type *) (___tmp + 1); }

void * Loci_Allocate(size_t);

#endif
