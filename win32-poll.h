/**
 * @file poll.h
 */
#ifndef WIN32_POLL_H
#define WIN32_POLL_H

#ifndef HAVE_POLL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

enum {
#undef POLLIN
  POLLIN = 0x0001,
#undef POLLPRI
  POLLPRI = 0x0002,
#undef POLLOUT
  POLLOUT = 0x0004,
#undef POLLERR
  POLLERR = 0x0008,
#undef POLLHUP
  POLLHUP = 0x0010,
#undef POLLNVAL
  POLLNVAL = 0x0020,
#undef POLLRDNORM
  POLLRDNORM = 0x0040,
#undef POLLRDBAND
  POLLRDBAND = 0x0080,
#undef POLLWRNORM
  POLLWRNORM = 0x0100,
#undef POLLWRBAND
  POLLWRBAND = 0x0200,
#undef POLLMSG
  POLLMSG = 0x0400,
#undef POLLREMOVE
  /* POLLREMOVE is for /dev/epoll (/dev/misc/eventpoll),
   * a new event notification mechanism for 2.6 */
  POLLREMOVE = 0x1000,
};

#if defined(__sparc__) || defined(__mips__)
#define POLLWRNORM POLLOUT
#endif

#ifndef struct_pollfd_defined
#define struct_pollfd_defined 1
struct pollfd {
  int fd;
  short events;
  short revents;
};
#endif

typedef unsigned int nfds_t;

extern int poll(struct pollfd* ufds, nfds_t nfds, int timeout);

#if defined(_WIN32) && !defined(CYGWIN)
typedef int sigset_t[32];
#endif

#ifdef _GNU_SOURCE
#include <signal.h>
int ppoll(struct pollfd* fds, nfds_t nfds, const struct timespec* timeout, const sigset_t* sigmask);
#endif

#ifdef __cplusplus
}

#endif /* defined(__cplusplus) */

#endif /* defined(HAVE_POLL_H) */

#endif /* WIN32_POLL_H */
