/**
 * @file module_compat.c
 * @brief LoongArch64 OneOS module-load compatibility shims.
 */

#if defined(ONEOS_PLATFORM) && (defined(__loongarch__) || defined(__loongarch64) || defined(__loongarch_lp64))

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

int dirfd(DIR *dirp)
{
    (void)dirp;
    errno = ENOSYS;
    return -1;
}

int statvfs(const char *path, struct statvfs *buf)
{
    (void)path;
    (void)buf;
    errno = ENOSYS;
    return -1;
}

int fchmodat(int dirfd_arg, const char *path, mode_t mode, int flags)
{
    (void)dirfd_arg;
    (void)path;
    (void)mode;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

DIR *fdopendir(int fd)
{
    (void)fd;
    errno = ENOSYS;
    return NULL;
}

int utimensat(int dirfd_arg, const char *path, const struct timespec times[2], int flags)
{
    (void)dirfd_arg;
    (void)path;
    (void)times;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

int unlinkat(int dirfd_arg, const char *path, int flags)
{
    (void)dirfd_arg;
    (void)path;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

int openat(int dirfd_arg, const char *path, int flags, ...)
{
    (void)dirfd_arg;
    (void)path;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

int fchmod(int fd, mode_t mode)
{
    (void)fd;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

int truncate(const char *path, off_t length)
{
    (void)path;
    (void)length;
    errno = ENOSYS;
    return -1;
}

#endif /* ONEOS_PLATFORM && LoongArch64 */
