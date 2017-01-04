#include <pthread.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

size_t MT1N9pthread_tF1N11__alignmask() {
	return alignof(pthread_t) - 1;
}

size_t MT1N9pthread_tF1N8__sizeof() {
	return sizeof(pthread_t);
}

void MT1N9pthread_tF1N6__move(void* dest, pthread_t* thread) {
	memcpy(dest, thread, sizeof(*thread));
}

void MT1N9pthread_tF1N4null(pthread_t* thread) {
	// Don't do anything until create().
	(void) thread;
	return;
}

void MT1N9pthread_tF1N9__destroy(pthread_t* thread) {
	// Should already have joined...
	(void) thread;
}

void MT1N9pthread_tF1N9__setdead(pthread_t* thread) {
	// Nothing to do.
	(void) thread;
}

int MT1N9pthread_tF1N6create(pthread_t* thread,
                             void* attr,
                             void* (*start_routine)(void*),
                             void* arg) {
	return pthread_create(thread,
	                      attr,
	                      start_routine,
	                      arg);
}

int MT1N9pthread_tF1N4join(pthread_t* thread, void** retval) {
	return pthread_join(*thread, retval);
}
