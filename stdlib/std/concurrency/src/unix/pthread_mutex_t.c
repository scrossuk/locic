#include <pthread.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

size_t MT1N15pthread_mutex_tF1N11__alignmask() {
	return alignof(pthread_mutex_t) - 1;
}

size_t MT1N15pthread_mutex_tF1N8__sizeof() {
	return sizeof(pthread_mutex_t);
}

void MT1N15pthread_mutex_tF1N6__move(void* dest, pthread_mutex_t* mutex) {
	memcpy(dest, mutex, sizeof(*mutex));
}

void MT1N15pthread_mutex_tF1N4null(pthread_mutex_t* mutex) {
	// Don't do anything until init().
	(void) mutex;
	return;
}

void MT1N15pthread_mutex_tF1N9__destroy(pthread_mutex_t* mutex) {
	// Should already have called destroy().
	(void) mutex;
}

void MT1N15pthread_mutex_tF1N9__setdead(pthread_mutex_t* mutex) {
	// Nothing to do.
	(void) mutex;
}

int MT1N15pthread_mutex_tF1N4init(pthread_mutex_t* mutex, void* ptr) {
	return pthread_mutex_init(mutex, ptr);
}

int MT1N15pthread_mutex_tF1N7destroy(pthread_mutex_t* mutex) {
	return pthread_mutex_destroy(mutex);
}

int MT1N15pthread_mutex_tF1N4lock(pthread_mutex_t* mutex) {
	return pthread_mutex_lock(mutex);
}

int MT1N15pthread_mutex_tF1N6unlock(pthread_mutex_t* mutex) {
	return pthread_mutex_unlock(mutex);
}
