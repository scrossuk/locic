#include <stdlib.h>

#include <Locic/AST/List.h>

AST_ListElement * AST_ListBegin(AST_List * list){
	return (list->head).next;
}

AST_ListElement * AST_ListEnd(){
	return NULL;
}

size_t AST_ListSize(AST_List * list){
	size_t n = 0;
	AST_ListElement * it;
	for(it = AST_ListBegin(list); it != AST_ListEnd(); it = it->next){
		n++;
	}
	return n;
}

AST_List * AST_ListCreate(){
	AST_List * list = malloc(sizeof(AST_List));
	list->head.next = NULL;
	list->head.data = NULL;
	list->tail = &(list->head);
	return list;
}

AST_List * AST_ListAppend(AST_List * list, void * ptr){
	AST_ListElement * listElement = malloc(sizeof(AST_ListElement));
	
	AST_ListElement * const tail = list->tail;
	tail->next = listElement;
	list->tail = listElement;
	listElement->next = NULL;
	
	listElement->data = ptr;
	
	return list;
}

