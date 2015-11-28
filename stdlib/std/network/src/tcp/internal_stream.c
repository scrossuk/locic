#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static int getDontWait() {
#if defined(MSG_DONTWAIT)
	return MSG_DONTWAIT;
#else
	return 0;
#endif
}

static int getNonBlock() {
#if defined(SOCK_NONBLOCK)
	return SOCK_NONBLOCK;
#else
	return 0;
#endif
}

static int getNoSignal() {
#if defined(MSG_NOSIGNAL)
	return MSG_NOSIGNAL;
#else
	return 0;
#endif
}

static int getReadFlags() {
	return getDontWait();
}

static int getWriteFlags() {
	return getDontWait() | getNoSignal();
}

static int getAcceptFlags() {
	return getNonBlock();
}

int std_tcp_iswouldblock(const int error) {
	return error == EAGAIN || error == EWOULDBLOCK;
}

int std_tcp_isinprogress(const int error) {
	return error == EINPROGRESS;
}

int std_tcp_socket() {
	// Specify IPv6 since we can use this for both
	// IPv4 and IPv6 by disabling IPV6_V6ONLY.
	const int result = socket(AF_INET6, SOCK_STREAM, 0);
	return result >= 0 ? result : -errno;
}

struct std_ipv6_address {
	uint8_t value[16];
};

static struct in6_addr encodeAddress(const struct std_ipv6_address* const address) {
	struct in6_addr addr;
	for (size_t i = 0; i < 16; i++) {
		addr.s6_addr[i] = address->value[i];
	}
	return addr;
}

static struct std_ipv6_address decodeAddress(const struct in6_addr * const address) {
	struct std_ipv6_address addr;
	for (size_t i = 0; i < 16; i++) {
		addr.value[i] = address->s6_addr[i];
	}
	return addr;
}

int std_tcp_connect(const int handle, const struct std_ipv6_address* const address, const uint16_t port) {
	struct sockaddr_in6 peerAddress;
	memset(&peerAddress, 0, sizeof(peerAddress));
	peerAddress.sin6_family = AF_INET6;
	peerAddress.sin6_addr = encodeAddress(address);
	peerAddress.sin6_port = htons(port);
	
	while (1) {
		const int result = connect(handle, (const struct sockaddr*) &peerAddress, sizeof(peerAddress));
		if (result >= 0) {
			return result;
		} else {
			if (errno == EINTR) {
				continue;
			}
			
			return -errno;
		}
	}
}

int std_tcp_setnosigpipe(const int handle, const int value) {
	(void) handle;
	(void) value;
#if defined(MSG_NOSIGNAL)
	// We'll pass MSG_NOSIGNAL for each write.
	return 0;
#elif defined(SO_NOSIGPIPE)
	const int result = setsockopt(handle, SOL_SOCKET, SO_NOSIGPIPE, (const void*) &value, sizeof(value));
	return result >= 0 ? result : -errno;
#else
#warning No option is supported on this platform for preventing SIGPIPE.
	// Give up...
	return 0;
#endif
}

int std_tcp_setnonblocking(const int handle, const int value) {
	const int flags = fcntl(handle, F_GETFL, 0);
	if (flags < 0) {
		return -errno;
	}
	const int newFlags = (value != 0) ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	const int result = fcntl(handle, F_SETFL, newFlags);
	return result >= 0 ? result : -errno;
}

int std_tcp_setipv6only(const int handle, const int value) {
	const int result = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const void *)&value, sizeof(value));
	return result >= 0 ? result : -errno;
}

int std_tcp_setreuseaddress(const int handle, const int value) {
	const int result = setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const void *)&value, sizeof(value));
	return result >= 0 ? result : -errno;
}

int std_tcp_geterror(const int handle) {
	int value = 0;
	socklen_t valueSize = sizeof(value);
	const int result = getsockopt(handle, SOL_SOCKET, SO_ERROR, (void *)&value, &valueSize);
	return result >= 0 ? value : -errno;
}

int std_tcp_bind(const int handle, const uint16_t port) {
	struct sockaddr_in6 localAddress;
	memset(&localAddress, 0, sizeof(localAddress));
	localAddress.sin6_family = AF_INET6;
	localAddress.sin6_addr = in6addr_any;
	localAddress.sin6_port = htons(port);
	
	const int result = bind(handle, (const struct sockaddr*) &localAddress, sizeof(localAddress));
	if (result < 0) {
		printf("bind() failed: %s (%d)\n", strerror(errno), errno);
	}
	return result >= 0 ? result : -errno;
}

int std_tcp_listen(const int handle, const int backlog) {
	const int result = listen(handle, backlog);
	return result >= 0 ? result : -errno;
}

int std_tcp_accept(const int handle, struct std_ipv6_address* const peerAddressValue, uint16_t* const peerPort) {
	struct sockaddr_in6 peerAddress;
	socklen_t peerAddressLength = sizeof(peerAddress);
	const int result = accept(handle, (struct sockaddr*) &peerAddress, &peerAddressLength);
	
	if (result >= 0) {
		if (peerAddressValue != NULL) {
			*peerAddressValue = decodeAddress(&(peerAddress.sin6_addr));
		}
		if (peerPort != NULL) {
			*peerPort = peerAddress.sin6_port;
		}
		return result;
	} else {
		return -errno;
	}
}

ssize_t std_tcp_read(const int eventHandle, uint8_t* const data, const size_t size) {
	while (1) {
		const ssize_t result = recv(eventHandle, data, size, getReadFlags());
		if (result >= 0) {
			return result;
		} else {
			if (errno == EINTR) {
				continue;
			}
			
			return -errno;
		}
	}
}

ssize_t std_tcp_write(const int eventHandle, const uint8_t* const data, const size_t size) {
	while (1) {
		const ssize_t result = send(eventHandle, data, size, getWriteFlags());
		if (result >= 0) {
			return result;
		} else {
			if (errno == EINTR) {
				continue;
			}
			
			return -errno;
		}
	}
}

void std_tcp_close(const int eventHandle) {
	const int result = close(eventHandle);
	if (result < 0) {
		printf("close() failed: %s (%d)\n", strerror(errno), errno);
	}
}

