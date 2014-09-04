#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

typedef int EventHandle;

struct HandlePair {
	EventHandle read, write;
};

struct HandlePair std_event_generator_makepipe() {
	int handles[2] = {0, 0};
	const int result = pipe(handles);
	
	struct HandlePair handlePair;
	handlePair.read = -1;
	handlePair.write = -1;
	
	if (result < 0) {
		handlePair.read = handlePair.write = -errno;
	} else {
		handlePair.read = handles[0];
		handlePair.write = handles[1];
		
		// Make read-end non-blocking.
		const int flags = fcntl(handlePair.read, F_GETFL, 0);
		fcntl(handlePair.read, F_SETFL, flags | O_NONBLOCK);
	}
	
	return handlePair;
}

int std_event_generator_closepipe(struct HandlePair handlePair) {
	const int readCloseResult = close(handlePair.read);
	if (readCloseResult < 0) {
		return -errno;
	}
	
	const int writeCloseResult = close(handlePair.write);
	if (writeCloseResult < 0) {
		return -errno;
	}
	
	return 0;
}

int std_event_generator_readpipe(struct HandlePair handlePair) {
	uint8_t data[1] = { 0xFF };
	const ssize_t result = read(handlePair.read, data, sizeof(data)/sizeof(data[0]));
	
	if (result < 0) {
		return -errno;
	}
	
	return (int) result;
}

int std_event_generator_reset(struct HandlePair handlePair) {
	// Read any data already in the pipe.
	while (1) {
		uint8_t data[1] = { 0xFF };
		const ssize_t result = read(handlePair.read, data, sizeof(data)/sizeof(data[0]));
		
		if (result <= 0) {
			return 0;
		}
	}
}

int std_event_generator_notify(struct HandlePair handlePair) {
	// Read any data already in the pipe.
	(void) std_event_generator_reset(handlePair);
	
	// Write a byte to generate an event.
	const uint8_t data[1] = { 0xFF };
	const ssize_t result = write(handlePair.write, data, sizeof(data)/sizeof(data[0]));
	
	if (result < 0) {
		return -errno;
	}
	
	return (int) result;
}



