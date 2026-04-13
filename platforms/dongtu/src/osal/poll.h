/*
 * poll.h
 *
 *  Created on: 2026Äê4ÔÂ13ÈÕ
 *      Author: Pan_Xuan
 */

#ifndef OSAL_POLL_H_
#define OSAL_POLL_H_

#include <lwip/sockets.h>

typedef unsigned long nfds_t;

struct pollfd {
    int   fd;
    short events;
    short revents;
};

#ifndef POLLIN
#define POLLIN    0x0001
#endif

#ifndef POLLPRI
#define POLLPRI   0x0002
#endif

#ifndef POLLOUT
#define POLLOUT   0x0004
#endif

#ifndef POLLERR
#define POLLERR   0x0008
#endif

#ifndef POLLHUP
#define POLLHUP   0x0010
#endif

#ifndef POLLNVAL
#define POLLNVAL  0x0020
#endif

static inline int poll(struct pollfd *fds, nfds_t nfds, int timeout_ms)
{
    fd_set rfds, wfds, efds;
    struct timeval tv;
    struct timeval *ptv = 0;
    int maxfd = -1;
    int rc;
    nfds_t i;
    int ready = 0;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    for (i = 0; i < nfds; ++i) {
        fds[i].revents = 0;

        if (fds[i].fd < 0) {
            continue;
        }

        if (fds[i].events & (POLLIN | POLLPRI)) {
            FD_SET(fds[i].fd, &rfds);
        }
        if (fds[i].events & POLLOUT) {
            FD_SET(fds[i].fd, &wfds);
        }

        FD_SET(fds[i].fd, &efds);

        if (fds[i].fd > maxfd) {
            maxfd = fds[i].fd;
        }
    }

    if (maxfd < 0) {
        return 0;
    }

    if (timeout_ms >= 0) {
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        ptv = &tv;
    }

    rc = select(maxfd + 1, &rfds, &wfds, &efds, ptv);
    if (rc <= 0) {
        return rc;
    }

    for (i = 0; i < nfds; ++i) {
        if (fds[i].fd < 0) {
            continue;
        }

        if (FD_ISSET(fds[i].fd, &rfds)) {
            fds[i].revents |= POLLIN;
        }
        if (FD_ISSET(fds[i].fd, &wfds)) {
            fds[i].revents |= POLLOUT;
        }
        if (FD_ISSET(fds[i].fd, &efds)) {
            fds[i].revents |= POLLERR;
        }

        if (fds[i].revents) {
            ++ready;
        }
    }

    return ready;
}

#endif /* OSAL_POLL_H_ */

