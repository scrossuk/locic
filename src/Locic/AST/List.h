#ifndef LOCIC_AST_LIST_H
#define LOCIC_AST_LIST_H

typedef struct AST_ListElement{
	struct AST_ListElement * next;
	void * data;
} AST_ListElement;

typedef struct AST_List{
	AST_ListElement head;
	AST_ListElement * tail;
} AST_List;

AST_ListElement * AST_ListBegin(AST_List * list);

AST_ListElement * AST_ListEnd();

AST_List * AST_ListCreate();

AST_List * AST_ListAppend(AST_List * list, void * ptr);

#endif
