/*
 * Small libc/POSIX compatibility shims for Dongtu/Intewell vm_3588 builds.
 *
 * These only cover symbols that the SDK headers expose incompletely or do not
 * provide at link time. Runtime behavior remains conservative: unsupported
 * operations return an error instead of pretending to work.
 */

#include <errno.h>
#include <sys/stat.h>

#ifndef F_OK
#define F_OK 0
#endif

extern int stat(const char *path, struct stat *buf);

int pipe(int fd[2])
{
	if (fd != 0) {
		fd[0] = -1;
		fd[1] = -1;
	}
	errno = ENOSYS;
	return -1;
}

int access(const char *path, int mode)
{
	struct stat st;

	if (path == 0) {
		errno = EINVAL;
		return -1;
	}

	if (mode != F_OK) {
		errno = ENOSYS;
		return -1;
	}

	return stat(path, &st);
}
