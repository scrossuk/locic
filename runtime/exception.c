#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unwind.h>

typedef struct {
	// Loci data.
	int value;
	
	// Unwind exception.
	struct _Unwind_Exception unwindException;
} __loci_exception_header_t;

void* __loci_allocate_exception(size_t value) {
	printf("__loci: Allocate exception of size %llu.\n", (unsigned long long) value);
	return malloc(sizeof(__loci_exception_header_t) + value) + sizeof(__loci_exception_header_t);
}

void __loci_free_exception(void* ptr) {
	printf("__loci: Free exception.\n");
	free(ptr);
}

void __loci_throw(void* exceptionPtr, void* exceptionType, void* destructor) {
	printf("__loci: Throw.\n");
	__loci_exception_header_t* const header = ((__loci_exception_header_t*) exceptionPtr) - 1;
	_Unwind_RaiseException(&(header->unwindException));
	
	// 'RaiseException' should not return.
	abort();
}

void __loci_begin_catch() {
	printf("__loci: BEGIN catch.\n");
}

void __loci_end_catch() {
	printf("__loci: END catch.\n");
}

_Unwind_Reason_Code __loci_personality_v0(
	int version, _Unwind_Action actions, uint64_t exceptionClass,
	struct _Unwind_Exception* unwind_exception, struct _Unwind_Context* context) {
	printf("__loci: Personality function.\n");
	
	if ((actions & _UA_SEARCH_PHASE) != 0) {
		printf("__loci: Personality function SEARCH phase.\n");
		return _URC_HANDLER_FOUND;
	} else if ((actions & _UA_CLEANUP_PHASE) != 0) {
		printf("__loci: Personality function CLEANUP phase.\n");
		const uint8_t* languageSpecificData = (const uint8_t*) _Unwind_GetLanguageSpecificData(context);
		(void) languageSpecificData;
		return _URC_INSTALL_CONTEXT;
	} else {
		printf("__loci: Personality function [UNKNOWN] phase.\n");
		return _URC_FATAL_PHASE1_ERROR;
	}
}

