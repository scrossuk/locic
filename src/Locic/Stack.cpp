#include <assert.h>
#include <vector>
#include <Locic/Stack.h>

typedef std::vector<void*> StackType;

extern "C" {

	void* Locic_Stack_Alloc() {
		return new StackType();
	}
	
	void Locic_Stack_Free(void* stackPtr) {
		assert(stackPtr != NULL);
		delete reinterpret_cast<StackType*>(stackPtr);
	}
	
	void Locic_Stack_Push(void* stackPtr, void* data) {
		assert(stackPtr != NULL);
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		stack->push_back(data);
	}
	
	void Locic_Stack_Pop(void* stackPtr) {
		assert(stackPtr != NULL);
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		assert(!stack->empty());
		stack->pop_back();
	}
	
	void* Locic_Stack_Top(void* stackPtr) {
		assert(stackPtr != NULL);
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		assert(!stack->empty());
		return stack->back();
	}
	
	void* Locic_Stack_Get(void* stackPtr, size_t level) {
		assert(stackPtr != NULL);
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		assert(level < stack->size());
		return (*stack)[level];
	}
	
	size_t Locic_Stack_Size(void* stackPtr) {
		assert(stackPtr != NULL);
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		return stack->size();
	}
	
}

