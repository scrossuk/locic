#include <stdio.h>
#include "Bool.h"

void * _Main_EntryPoint();

void * _Main_run(void *);

int main(){
	void * data = _Main_EntryPoint();
	Loci_Bool * result = (Loci_Bool *) _Main_run(data);
	
	return result->value ? 0 : 1;
}

