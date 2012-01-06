#include <stdio.h>
#include <stdlib.h>

#include <Locic/List.h>

Locic_List * Locic_List_Alloc(){
	Locic_List * list = malloc(sizeof(Locic_List));
	list->head.next = NULL;
	list->head.data = NULL;
	list->tail = &(list->head);
	if(list->tail == NULL){
		printf("Internal compiler error: Locic_List_Alloc(): list has NULL tail.\n");
		return NULL;
	}
	return list;
}

void Locic_List_Free(Locic_List * list){
	if(list == NULL){
		printf("Internal compiler error: Locic_List_Free() called on NULL list.\n");
		return;
	}
	
	printf("Free list\n");

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
	if(list == NULL){
		printf("Internal compiler error: Locic_List_Append() called on NULL list.\n");
		return NULL;
	}

	Locic_ListElement * listElement = malloc(sizeof(Locic_ListElement));
	
	Locic_ListElement * const tail = list->tail;
	
	if(tail == NULL){
		printf("Internal compiler error: Locic_List_Append(): list has NULL tail.\n");
		return NULL;
	}
	
	tail->next = listElement;
	list->tail = listElement;
	listElement->next = NULL;
	
	listElement->data = data;
	
	list->size++;
	
	return list;
}

Locic_ListElement * Locic_List_Begin(Locic_List * list){
	if(list == NULL){
		printf("Internal compiler error: Locic_List_Begin() called on NULL list.\n");
		return NULL;
	}
	return (list->head).next;
}

Locic_ListElement * Locic_List_End(Locic_List * list){
	if(list == NULL){
		printf("Internal compiler error: Locic_List_End() called on NULL list.\n");
		return NULL;
	}
	return NULL;
}

size_t Locic_List_Size(Locic_List * list){
	if(list == NULL){
		printf("Internal compiler error: Locic_List_Size() called on NULL list.\n");
		return;
	}
	return list->size;
}

