#ifndef LOCIC_AST_LIST_H
#define LOCIC_AST_LIST_H

typedef struct AST_ListElement{
	AST_ListElement * next;
	void * data;
} AST_ListElement;

typedef struct AST_List{
	AST_ListElement head;
	AST_ListElement * tail;
} AST_List;

inline AST_ListElement * AST_ListBegin(AST_List * list){
	return (list->head).next;
}

inline AST_ListElement * AST_ListEnd(){
	return NULL;
}

inline AST_List * AST_ListCreate(){
	AST_List * list = malloc(sizeof(AST_List));
	list->tail = &(list->head);
	return list;
}

inline AST_List * AST_ListAppend(AST_List * list, void * ptr){
	AST_ListElement * listElement = malloc(sizeof(AST_ListElement));
	
	AST_ListElement * const tail = list->tail;
	tail->next = listElement;
	list->tail = listElement;
	listElement->next = NULL;
	
	return list;
}

#endif
