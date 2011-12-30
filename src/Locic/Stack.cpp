#include <vector>
#include <Locic/Stack.h>

typedef std::vector<void*> StackType;

extern "C" {

	void* Locic_Stack_Alloc() {
		return new StackType();
	}
	
	void Locic_Stack_Free(void* stackPtr) {
		delete reinterpret_cast<StackType*>(stackPtr);
	}
	
	void Locic_Stack_Push(void* stackPtr, void* data) {
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		stack->push_back(data);
	}
	
	void Locic_Stack_Pop(void* stackPtr) {
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		stack->pop_back();
	}
	
	void* Locic_Stack_Top(void* stackPtr) {
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		return stack->back();
	}
	
	void* Locic_Stack_Get(void* stackPtr, size_t level) {
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		return (*stack)[level];
	}
	
	size_t Locic_Stack_Size(void* stackPtr) {
		StackType* stack = reinterpret_cast<StackType*>(stackPtr);
		return stack->size();
	}
	
}

