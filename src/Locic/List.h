#ifndef LOCIC_LIST_H
#define LOCIC_LIST_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Locic_ListElement{
	struct Locic_ListElement * next;
	void * data;
} Locic_ListElement;

typedef struct Locic_List{
	size_t size;
	Locic_ListElement head;
	Locic_ListElement * tail;
} Locic_List;

Locic_List * Locic_List_Alloc();

void Locic_List_Free(Locic_List * list);

Locic_List * Locic_List_Append(Locic_List * list, void * data);

Locic_ListElement * Locic_List_Begin(Locic_List * list);

Locic_ListElement * Locic_List_End(Locic_List * list);

size_t Locic_List_Size(Locic_List * list);

#ifdef __cplusplus
}
#endif

#endif
