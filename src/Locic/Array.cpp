#include <vector>
#include <Locic/Array.h>

typedef std::vector<void*> ArrayType;

extern "C" {

	void* Locic_Array_Alloc() {
		return new ArrayType();
	}
	
	void Locic_Array_Free(void* arrayPtr) {
		delete reinterpret_cast<ArrayType*>(arrayPtr);
	}
	
	void Locic_Array_PushBack(void* arrayPtr, void* data) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		array->push_back(data);
	}
	
	void Locic_Array_PopBack(void* arrayPtr) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		array->pop_back();
	}
	
	void* Locic_Array_Front(void* arrayPtr) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		return array->back();
	}
	
	void* Locic_Array_Back(void* arrayPtr) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		return array->back();
	}
	
	void* Locic_Array_Get(void* arrayPtr, size_t level) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		return (*array)[level];
	}
	
	void Locic_Array_Set(void* arrayPtr, size_t level, void * data) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		(*array)[level] = data;
	}
	
	size_t Locic_Array_Size(void* arrayPtr) {
		ArrayType* array = reinterpret_cast<ArrayType*>(arrayPtr);
		return array->size();
	}
	
}

