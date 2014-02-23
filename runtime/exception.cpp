#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <unwind.h>

typedef const char* __loci_exception_name_t;

typedef struct __loci_catch_type_t {
	uint32_t offset;
	__loci_exception_name_t name;
} __loci_catch_type_t;

typedef struct __loci_throw_type_t {
	uint32_t length;
	__loci_exception_name_t names[8];
} __loci_throw_type_t;

typedef struct __loci_exception_t {
	// Loci data.
	const __loci_throw_type_t* type;
	
	// Unwind exception.
	struct _Unwind_Exception unwindException;
} __loci_exception_t;

const _Unwind_Exception_Class __loci_exception_class
	= (
		(
			(
				(
					(
						(_Unwind_Exception_Class) 'L'
					) << 8 |
					(_Unwind_Exception_Class) 'O'
				) << 8 |
				(_Unwind_Exception_Class) 'C'
			) << 8 |
			(_Unwind_Exception_Class) 'I'
		) << 8 |
		(_Unwind_Exception_Class) '\0'
	);

static uint64_t EXCEPTION_UNWIND_OFFSET() {
	return offsetof(struct __loci_exception_t, unwindException);
}

static __loci_exception_t* GET_EXCEPTION(struct _Unwind_Exception* unwindException) {
	return ((__loci_exception_t*) (((uint8_t*) unwindException) - EXCEPTION_UNWIND_OFFSET()));
}

static __loci_exception_t* GET_EXCEPTION_HEADER(void* exception) {
	return ((__loci_exception_t*) exception) - 1;
}

static void* GET_EXCEPTION_DATA(__loci_exception_t* exception) {
	return (uint8_t*) (exception + 1);
}

static bool canCatch(const __loci_catch_type_t* catchType, const __loci_throw_type_t* throwType) {
	assert(throwType != NULL);
	
	// Treat null pointer as a catch-all.
	if (catchType == NULL) {
		return true;
	}
	
	if (catchType->offset >= throwType->length) {
		return false;
	}
	
	return strcmp(catchType->name, throwType->names[catchType->offset]) == 0;
}

enum {
	DW_EH_PE_absptr = 0x00,
	DW_EH_PE_omit = 0xff,
	DW_EH_PE_uleb128 = 0x01,
	DW_EH_PE_udata2 = 0x02,
	DW_EH_PE_udata4 = 0x03,
	DW_EH_PE_udata8 = 0x04,
	DW_EH_PE_sleb128 = 0x09,
	DW_EH_PE_sdata2 = 0x0A,
	DW_EH_PE_sdata4 = 0x0B,
	DW_EH_PE_sdata8 = 0x0C,
	DW_EH_PE_signed = 0x08,
	DW_EH_PE_programCounterrel = 0x10,
	DW_EH_PE_textrel = 0x20,
	DW_EH_PE_datarel = 0x30,
	DW_EH_PE_funcrel = 0x40,
	DW_EH_PE_aligned = 0x50,
	DW_EH_PE_indirect = 0x80
};

extern "C" void* __loci_allocate_exception(size_t value) {
	return ((uint8_t*) malloc(sizeof(__loci_exception_t) + value)) + sizeof(__loci_exception_t);
}

extern "C" void __loci_free_exception(void* ptr) {
	free(ptr);
}

extern "C" void __loci_throw(void* exceptionPtr, void* exceptionType, void* destructor) {
	(void) destructor;
	
	uint32_t* ptr = (uint32_t*) exceptionPtr;
	for (size_t i = 0; i < 3; i++) {
		printf("At %llu: %llu\n", (unsigned long long) i, (unsigned long long) ptr[i]);
	}
	
	__loci_exception_t* const header = GET_EXCEPTION_HEADER(exceptionPtr);
	
	header->type = (const __loci_throw_type_t*) exceptionType;
	assert(header->type->length > 0);
	
	header->unwindException.exception_class = __loci_exception_class;
	
	const _Unwind_Reason_Code result = _Unwind_RaiseException(&(header->unwindException));
	
	// 'RaiseException' ONLY returns if there is an error;
	// abort the process if this happens.
	if (result == _URC_END_OF_STACK) {
		printf("Length: %llu\n", (unsigned long long) header->type->length);
		for (uint32_t i = 0; i < header->type->length; i++) {
			printf("At %llu: %s\n", (unsigned long long) i, header->type->names[i]);
		}
		printf("Unhandled exception of type '%s'; aborting...\n", header->type->names[header->type->length - 1]);
	} else {
		printf("Unwind failed with result %d; calling abort().\n", (int) result);
	}
	
	abort();
}

extern "C" void* __loci_begin_catch(_Unwind_Exception* unwindException) {
	__loci_exception_t* const exception = GET_EXCEPTION(unwindException);
	return GET_EXCEPTION_DATA(exception);
}

extern "C" void __loci_end_catch() {
	// TODO
}

// Decode uleb128 value.
static uintptr_t readULEB128(const uint8_t** data) {
	uintptr_t result = 0;
	uintptr_t shift = 0;
	const uint8_t* nextPtr = *data;
	
	while (true) {
		const uint8_t byte = *nextPtr++;
		result |= (byte & 0x7f) << shift;
		shift += 7;
		if ((byte & 0x80) == 0) break;
	}
	
	*data = nextPtr;
	
	return result;
}

// Decode sleb128 value.
static uintptr_t readSLEB128(const uint8_t** data) {
	uintptr_t result = 0;
	uintptr_t shift = 0;
	const uint8_t* nextPtr = *data;
	
	while (true) {
		const uint8_t byte = *nextPtr++;
		result |= (byte & 0x7f) << shift;
		shift += 7;
		if ((byte & 0x80) == 0) {
			if ((byte & 0x40) != 0 && (shift < (sizeof(result) << 3))) {
				result |= (~0 << shift);
			}
			break;
		}
	}
	
	*data = nextPtr;
	
	return result;
}

unsigned int getEncodingSize(uint8_t encoding) {
	if (encoding == DW_EH_PE_omit) {
		return 0;
	}
	
	switch (encoding & 0x0F) {
		case DW_EH_PE_absptr:
			return sizeof(uintptr_t);
			
		case DW_EH_PE_udata2:
			return sizeof(uint16_t);
			
		case DW_EH_PE_udata4:
			return sizeof(uint32_t);
			
		case DW_EH_PE_udata8:
			return sizeof(uint64_t);
			
		case DW_EH_PE_sdata2:
			return sizeof(int16_t);
			
		case DW_EH_PE_sdata4:
			return sizeof(int32_t);
			
		case DW_EH_PE_sdata8:
			return sizeof(int64_t);
			
		default:
			// Not supported.
			abort();
	}
}

// Read DWARF encoded pointer value.
static uintptr_t readEncodedPointer(const uint8_t** data, uint8_t encoding) {
	if (encoding == DW_EH_PE_omit) {
		return 0;
	}
	
	uintptr_t result = 0;
	const uint8_t* nextPtr = *data;
	
	// Read value.
	switch (encoding & 0x0F) {
		case DW_EH_PE_absptr:
			result = *((uintptr_t*)nextPtr);
			nextPtr += sizeof(uintptr_t);
			break;
			
		case DW_EH_PE_uleb128:
			result = readULEB128(&nextPtr);
			break;
			
		case DW_EH_PE_sleb128:
			result = readSLEB128(&nextPtr);
			break;
			
		case DW_EH_PE_udata2:
			result = *((uint16_t*)nextPtr);
			nextPtr += sizeof(uint16_t);
			break;
			
		case DW_EH_PE_udata4:
			result = *((uint32_t*)nextPtr);
			nextPtr += sizeof(uint32_t);
			break;
			
		case DW_EH_PE_udata8:
			result = *((uint64_t*)nextPtr);
			nextPtr += sizeof(uint64_t);
			break;
			
		case DW_EH_PE_sdata2:
			result = *((int16_t*)nextPtr);
			nextPtr += sizeof(int16_t);
			break;
			
		case DW_EH_PE_sdata4:
			result = *((int32_t*)nextPtr);
			nextPtr += sizeof(int32_t);
			break;
			
		case DW_EH_PE_sdata8:
			result = *((int64_t*)nextPtr);
			nextPtr += sizeof(int64_t);
			break;
			
		default:
			// Not supported.
			abort();
			break;
	}
	
	// Add relative offset.
	switch (encoding & 0x70) {
		case DW_EH_PE_absptr:
			break;
			
		case DW_EH_PE_programCounterrel:
			result += (uintptr_t)(*data);
			break;
			
		case DW_EH_PE_textrel:
		case DW_EH_PE_datarel:
		case DW_EH_PE_funcrel:
		case DW_EH_PE_aligned:
		default:
			// Not supported.
			abort();
			break;
	}
	
	// Apply indirection.
	if ((encoding & DW_EH_PE_indirect) != 0) {
		result = *((uintptr_t*)result);
	}
	
	*data = nextPtr;
	
	return result;
}

static uint64_t handleAction(uint8_t typeTableEncoding, const uint8_t* classInfo,
		uintptr_t actionEntry, uint64_t exceptionClass,
		struct _Unwind_Exception* exceptionObject) {
	assert(exceptionObject != NULL);
	assert(exceptionClass == __loci_exception_class);
	
	// Extract information about exception being thrown.
	const __loci_exception_t* const exception = GET_EXCEPTION(exceptionObject);
	const __loci_throw_type_t* const exceptionThrowType = exception->type;
	
	const uint8_t* actionPos = (uint8_t*) actionEntry;
	
	while (true) {
		// Read offset of exception type (to be caught) in type table.
		const int64_t typeOffset = readSLEB128(&actionPos);
		
		// Read offset to the next action (don't advance pointer
		// since this offset is relevant to the current position).
		const uint8_t* actionPosCopy = actionPos;
		const int64_t actionOffset = readSLEB128(&actionPosCopy);
		
		// Advance pointer to next action.
		actionPos += actionOffset;
		
		assert(typeOffset >= 0 && "Filters are not supported.");
		
		// Type offset equal to 0 means a cleanup action;
		// non-zero position values indicate an exception handler.
		if (typeOffset > 0) {
			const unsigned typeEncodedSize = getEncodingSize(typeTableEncoding);
			
			// Type table is indexed 'backwards'.
			const uint8_t* typeEntryPointer = classInfo - typeOffset * typeEncodedSize;
			const uintptr_t exceptionCatchTypePointer = readEncodedPointer(&typeEntryPointer, typeTableEncoding);
			const __loci_catch_type_t* const exceptionCatchType = (__loci_catch_type_t*) exceptionCatchTypePointer;
			
			if (canCatch(exceptionCatchType, exceptionThrowType)) {
				return typeOffset;
			}
		}
		
		if (actionOffset == 0) {
			// Reached the end of actions for the landing pad.
			return 0;
		}
	}
}

// 'Personality' function for Loci exceptions.
// This determines the actions for each call stack frame.
extern "C" _Unwind_Reason_Code __loci_personality_v0(
	int version, _Unwind_Action actions, uint64_t exceptionClass,
	struct _Unwind_Exception* exceptionObject, struct _Unwind_Context* context) {
	(void) version;
	
	const uint8_t* languageSpecificData = (const uint8_t*) _Unwind_GetLanguageSpecificData(context);
	
	if (languageSpecificData == NULL) {
		// No data for this stack frame; keep unwinding.
		return _URC_CONTINUE_UNWIND;
	}
	
	// Get the address of the instruction which threw
	// (which is one before the current instruction).
	const uintptr_t programCounter = _Unwind_GetIP(context) - 1;
	
	// Get offset of throwing instruction in the function.
	const uintptr_t functionStart = _Unwind_GetRegionStart(context);
	const uintptr_t programCounterOffset = programCounter - functionStart;
	
	// Parse language specific data area header.
	const uint8_t lpStartEncoding = *languageSpecificData++;
	
	if (lpStartEncoding != DW_EH_PE_omit) {
		readEncodedPointer(&languageSpecificData, lpStartEncoding);
	}
	
	const uint8_t typeTableEncoding = *languageSpecificData++;
	
	const uint8_t* classInfo = NULL;
	
	if (typeTableEncoding != DW_EH_PE_omit) {
		// Calculate type info locations in emitted dwarf code which
		// were flagged by type info arguments to llvm.eh.selector
		// intrinsic
		const uintptr_t classInfoOffset = readULEB128(&languageSpecificData);
		classInfo = languageSpecificData + classInfoOffset;
	}
	
	const uint8_t callSiteEncoding = *languageSpecificData++;
	const uint32_t callSiteTableLength = readULEB128(&languageSpecificData);
	
	// Get position/size of call site table.
	const uint8_t* const callSiteTableStart = languageSpecificData;
	const uint8_t* const callSiteTableEnd = callSiteTableStart + callSiteTableLength;
	
	// Action table immediately follows call site table.
	const uint8_t* const actionTableStart = callSiteTableEnd;
	
	// Search for call site range that includes the instruction which threw.
	for (const uint8_t* callSitePtr = callSiteTableStart; callSitePtr < callSiteTableEnd; ) {
		const uintptr_t start = readEncodedPointer(&callSitePtr, callSiteEncoding);
		const uintptr_t length = readEncodedPointer(&callSitePtr, callSiteEncoding);
		const uintptr_t landingPad = readEncodedPointer(&callSitePtr, callSiteEncoding);
		const uintptr_t actionEntryOffsetPlusOne = readULEB128(&callSitePtr);
		
		if (landingPad == 0) {
			// No landing pad found for this entry.
			continue;
		}
		
		// Check exception class.
		const uintptr_t useActionEntryOffsetPlusOne = (exceptionClass == __loci_exception_class) ? actionEntryOffsetPlusOne : 0;
		
		// Get action entry position (0 means no action).
		const uintptr_t actionEntryPointer = useActionEntryOffsetPlusOne != 0 ? (((uintptr_t) actionTableStart) + useActionEntryOffsetPlusOne - 1) : 0;
		
		if (start <= programCounterOffset && programCounterOffset < (start + length)) {
			// Found a suitable landing pad.
			
			// Look for a CATCH handler in the action table.
			const uint64_t actionValue = actionEntryPointer != 0 ?
				handleAction(typeTableEncoding, classInfo, actionEntryPointer,
						exceptionClass, exceptionObject) : false;
			
			const bool isSearchPhase = (actions & _UA_SEARCH_PHASE) != 0;
			if (isSearchPhase) {
				if (actionValue > 0) {
					// Only mark CATCH handlers as found; any clean-up
					// actions will be executed in the cleanup phase
					// (assuming a CATCH handler is found).
					return _URC_HANDLER_FOUND;
				}
			} else {
				// Execute the landing pad.
				
				// Set registers that provide information to the landing pad
				// about what action to take.
				
				// Provide a pointer to the exception thrown;
				// must point to the Unwind exception so that
				// resume works correctly (generated code must
				// call __loci_begin_catch to get exception data).
				_Unwind_SetGR(context, __builtin_eh_return_data_regno(0), (uintptr_t) exceptionObject);
				
				if (actionValue > 0) {
					// Set the selector value (index of the exception)
					// to indicate which exception is being caught.
					_Unwind_SetGR(context, __builtin_eh_return_data_regno(1), actionValue);
				} else {
					// Only perform cleanup action.
					_Unwind_SetGR(context, __builtin_eh_return_data_regno(1), 0);
				}
				
				// Set the instruction pointer to execute the landing pad.
				_Unwind_SetIP(context, functionStart + landingPad);
				return _URC_INSTALL_CONTEXT;
			}
		}
	}
	
	return _URC_CONTINUE_UNWIND;
}

