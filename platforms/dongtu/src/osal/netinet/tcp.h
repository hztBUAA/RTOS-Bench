/*
 * tcp.h
 *
 *  Created on: 2026Äê4ÔÂ13ÈÕ
 *      Author: Pan_Xuan
 */

#ifndef OSAL_NETINET_TCP_H_
#define OSAL_NETINET_TCP_H_

#include <lwip/sockets.h>

#ifndef TCP_NODELAY
#define TCP_NODELAY 1
#endif

#ifndef TCP_KEEPIDLE
#define TCP_KEEPIDLE 4
#endif

#ifndef TCP_KEEPINTVL
#define TCP_KEEPINTVL 5
#endif

#ifndef TCP_KEEPCNT
#define TCP_KEEPCNT 6
#endif

#endif /* OSAL_NETINET_TCP_H_ */
