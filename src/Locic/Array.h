#ifndef LOCIC_ARRAY_H
#define LOCIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void * Locic_Array;

Locic_Array Locic_Array_Alloc();

void Locic_Array_Free(Locic_Array array);

void Locic_Array_PushBack(Locic_Array array, void * data);

void Locic_Array_PopBack(Locic_Array array);

void * Locic_Array_Front(Locic_Array array);

void * Locic_Array_Back(Locic_Array array);

void * Locic_Array_Get(Locic_Array array, size_t index);

void Locic_Array_Set(Locic_Array array, size_t index, void * data);

size_t Locic_Array_Size(Locic_Array array);

#ifdef __cplusplus
}
#endif

#endif
