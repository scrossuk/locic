#include <assert.h>
#include <vector>
#include <Locic/Array.h>

typedef std::vector<void*> ArrayType;

extern "C" {

	void* Locic_Array_Alloc() {
		return new ArrayType();
	}
	
	void Locic_Array_Free(void* arrayPtr) {
		assert(arrayPtr != NULL);
		delete reinterpret_cast<ArrayType*>(arrayPtr);
	}
	
	void Locic_Array_PushBack(void* arrayPtr, void* data) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		array->push_back(data);
	}
	
	void Locic_Array_PopBack(void* arrayPtr) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		assert(!array->empty());
		array->pop_back();
	}
	
	void* Locic_Array_Front(void* arrayPtr) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		assert(!array->empty());
		return array->back();
	}
	
	void* Locic_Array_Back(void* arrayPtr) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		assert(!array->empty());
		return array->back();
	}
	
	void* Locic_Array_Get(void* arrayPtr, size_t index) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		assert(index < array->size());
		return (*array)[index];
	}
	
	void Locic_Array_Set(void* arrayPtr, size_t index, void * data) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		(*array)[index] = data;
	}
	
	size_t Locic_Array_Size(void* arrayPtr) {
		assert(arrayPtr != NULL);
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		return array->size();
	}
	
}

