#include <stdlib.h>

#include <Locic/AST/List.h>

AST_ListElement * AST_ListBegin(AST_List * list){
	return (list->head).next;
}

AST_ListElement * AST_ListEnd(){
	return NULL;
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

