#ifndef LOCIC_ARRAY_H
#define LOCIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

void * Locic_Array_Alloc();

void Locic_Array_Free(void * stack);

void Locic_Array_PushBack(void * stack, void * data);

void Locic_Array_PopBack(void * stack);

void * Locic_Array_Front(void * stack);

void * Locic_Array_Back(void * stack);

void * Locic_Array_Get(void * stack, size_t level);

void Locic_Array_Set(void * stack, size_t level, void * data);

size_t Locic_Array_Size(void * stack);

#ifdef __cplusplus
}
#endif

#endif
