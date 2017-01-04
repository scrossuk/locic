#include <pthread.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

size_t MT1N14pthread_cond_tF1N11__alignmask() {
	return alignof(pthread_cond_t) - 1;
}

size_t MT1N14pthread_cond_tF1N8__sizeof() {
	return sizeof(pthread_cond_t);
}

void MT1N14pthread_cond_tF1N6__move(void* dest, pthread_cond_t* cond) {
	memcpy(dest, cond, sizeof(*cond));
}

void MT1N14pthread_cond_tF1N4null(pthread_cond_t* cond) {
	// Don't do anything until init().
	(void) cond;
	return;
}

void MT1N14pthread_cond_tF1N9__destroy(pthread_cond_t* cond) {
	// Should already have called destroy().
	(void) cond;
}

void MT1N14pthread_cond_tF1N9__setdead(pthread_cond_t* cond) {
	// Nothing to do.
	(void) cond;
}

int MT1N14pthread_cond_tF1N4init(pthread_cond_t* cond, void* ptr) {
	return pthread_cond_init(cond, ptr);
}

int MT1N14pthread_cond_tF1N7destroy(pthread_cond_t* cond) {
	return pthread_cond_destroy(cond);
}

int MT1N14pthread_cond_tF1N6signal(pthread_cond_t* cond) {
	return pthread_cond_signal(cond);
}

int MT1N14pthread_cond_tF1N9broadcast(pthread_cond_t* cond) {
	return pthread_cond_broadcast(cond);
}

int MT1N14pthread_cond_tF1N4wait(pthread_cond_t* cond,
                                 pthread_mutex_t* mutex) {
	return pthread_cond_wait(cond, mutex);
}
