#ifndef LOCIC_STRINGMAP_H
#define LOCIC_STRINGMAP_H

#ifdef __cplusplus
extern "C" {
#endif

void * Locic_StringMap_Alloc();

void Locic_StringMap_Free(void * stringMap);

void Locic_StringMap_Clear(void * stringMap);

void * Locic_StringMap_Find(void * stringMap, const char * str);

// Return value is existing value in map, which if not NULL indicates the insert failed.
void * Locic_StringMap_Insert(void * stringMap, const char * str, void * data);

void * Locic_StringMap_Erase(void * stringMap, const char * str);

#ifdef __cplusplus
}
#endif

#endif
