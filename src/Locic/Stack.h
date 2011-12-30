#ifndef LOCIC_STACK_H
#define LOCIC_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

void * Locic_Stack_Alloc();

void Locic_Stack_Free(void * stack);

void Locic_Stack_Push(void * stack, void * data);

void Locic_Stack_Pop(void * stack);

void * Locic_Stack_Top(void * stack);

void * Locic_Stack_Get(void * stack, size_t level);

size_t Locic_Stack_Size(void * stack);

#ifdef __cplusplus
}
#endif

#endif
