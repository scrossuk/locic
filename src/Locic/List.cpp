#include <assert.h>
#include <stdlib.h>

#include <Locic/List.h>

Locic_List * Locic_List_Alloc(){
	Locic_List * list = malloc(sizeof(Locic_List));
	list->head.next = NULL;
	list->head.data = NULL;
	list->tail = &(list->head);
	list->size = 0;
	return list;
}

void Locic_List_Free(Locic_List * list){	
	assert(list != NULL);

	Locic_ListElement * it, * nextElement;
	
	it = Locic_List_Begin(list);
	while(it != Locic_List_End(list)){
		nextElement = it->next;
		free(it);
		it = nextElement;
	}
	free(list);
}

Locic_List * Locic_List_Append(Locic_List * list, void * data){
	assert(list != NULL);

	Locic_ListElement * listElement = malloc(sizeof(Locic_ListElement));
	
	Locic_ListElement * const tail = list->tail;	
	assert(tail != NULL);
	
	tail->next = listElement;
	list->tail = listElement;
	listElement->next = NULL;
	
	listElement->data = data;
	
	list->size++;
	
	return list;
}

Locic_ListElement * Locic_List_Begin(Locic_List * list){
	assert(list != NULL);
	return (list->head).next;
}

Locic_ListElement * Locic_List_End(Locic_List * list){
	assert(list != NULL);
	return NULL;
}

size_t Locic_List_Size(Locic_List * list){
	assert(list != NULL);
	return list->size;
}

