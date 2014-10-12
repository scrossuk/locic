#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef int EventHandle;

int std_tcp_connect_v4(uint32_t address, uint16_t port) {
	// TODO!
	(void) address;
	(void) port;
	return -1;
}

ssize_t std_tcp_read(EventHandle eventHandle, uint8_t* data, size_t size) {
	while (1) {
		const ssize_t result = recv(eventHandle, data, size, 0);
		if (result > 0) {
			return result;
		}
		
		if (result == 0) {
			// Return generic error to indicate connection lost.
			return -1;
		}
		
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		
		if (errno == EINTR) {
			continue;
		}
		
		return -errno;
	}
}

ssize_t std_tcp_write(EventHandle eventHandle, const uint8_t* const data, size_t size) {
	while (1) {
		const ssize_t result = send(eventHandle, data, size, 0);
		if (result >= 0) {
			return result;
		}
		
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		
		if (errno == EINTR) {
			continue;
		}
		
		return -errno;
	}
}

void std_tcp_close(EventHandle eventHandle) {
	(void) close(eventHandle);
}

