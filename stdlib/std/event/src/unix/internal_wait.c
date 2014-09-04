#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef int EventHandle;

#define MAX_EVENTS 10

// epoll_*() implementation.
EventHandle std_event_wait_create() {
	const int epollFd = epoll_create(1);
	return epollFd >= 0 ? epollFd : -errno;
}

int std_event_wait_destroy(EventHandle waitHandle) {
	const int closeResult = close(waitHandle);
	return closeResult >= 0 ? closeResult : -errno;
}

int std_event_wait_add_handle(EventHandle waitHandle, EventHandle newHandle, int isEdgeTriggered) {
	struct epoll_event ev;
	ev.events = (isEdgeTriggered != 0) ? (EPOLLIN | EPOLLET) : EPOLLIN;
	ev.data.u64 = 0;
	ev.data.fd = newHandle;
	const int ctlResult = epoll_ctl(waitHandle, EPOLL_CTL_ADD, newHandle, &ev);
	return ctlResult >= 0 ? ctlResult : -errno;
}

int std_event_wait_internal(EventHandle waitHandle, int doPoll) {
	struct epoll_event events[MAX_EVENTS];
	
	while (1) {
		const int epollWaitResult = epoll_wait(waitHandle, events, MAX_EVENTS, doPoll != 0 ? 0 : -1);
		if (epollWaitResult == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				return -errno;
			}
		}
		
		return epollWaitResult;
	}
}

/*
// select() implementation.
int std_event_wait_internal(const EventHandle* handleArray, size_t numHandles, int doPoll) {
	assert(numHandles > 0);
	
	while (1) {
		fd_set readSet, writeSet, excepSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		FD_ZERO(&excepSet);
		
		EventHandle maxHandle = -1;
		
		for (size_t i = 0; i < numHandles; i++) {
			const EventHandle handle = handleArray[i];
			
			if (handle == -1) {
				continue;
			}
			
			FD_SET(handle, &readSet);
			FD_SET(handle, &writeSet);
			FD_SET(handle, &excepSet);
			
			if (handle > maxHandle) {
				maxHandle = handle;
			}
		}
		
		assert(maxHandle != -1);
		
		struct timeval zeroTimeout = {0, 0};
		struct timeval* timeoutPtr = (doPoll != 0) ? &zeroTimeout : NULL;
		
		const int result = select(maxHandle + 1, &readSet, &writeSet, &excepSet, timeoutPtr);
		assert(result >= -1);
		
		if (result == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			}
			return -errno;
		} else {
			return result;
		}
	}
}
*/

