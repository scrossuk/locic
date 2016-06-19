#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int std_filesystem_is_file(const char* path) {
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		return 0;
	}
	return S_ISREG(path_stat.st_mode);
}

int std_filesystem_is_directory(const char* path) {
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		return 0;
	}
	return S_ISDIR(path_stat.st_mode);
}
