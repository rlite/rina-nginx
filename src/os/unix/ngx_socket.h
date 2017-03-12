
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SOCKET_H_INCLUDED_
#define _NGX_SOCKET_H_INCLUDED_


#include <ngx_config.h>


#define NGX_WRITE_SHUTDOWN SHUT_WR

typedef int  ngx_socket_t;

#ifndef NGX_RINA

#define ngx_socket          socket
#define ngx_socket_n        "socket()"

#define ngx_bind            bind
#define ngx_listen          listen
#define ngx_accept          accept
#define ngx_accept4         accept4
#define ngx_getsockopt      getsockopt
#define ngx_setsockopt      setsockopt


#if (NGX_HAVE_FIONBIO)

int ngx_nonblocking(ngx_socket_t s);
int ngx_blocking(ngx_socket_t s);

#define ngx_nonblocking_n   "ioctl(FIONBIO)"
#define ngx_blocking_n      "ioctl(!FIONBIO)"

#else

#define ngx_nonblocking(s)  fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)
#define ngx_nonblocking_n   "fcntl(O_NONBLOCK)"

#define ngx_blocking(s)     fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define ngx_blocking_n      "fcntl(!O_NONBLOCK)"

#endif

int ngx_tcp_nopush(ngx_socket_t s);
int ngx_tcp_push(ngx_socket_t s);

#if (NGX_LINUX)

#define ngx_tcp_nopush_n   "setsockopt(TCP_CORK)"
#define ngx_tcp_push_n     "setsockopt(!TCP_CORK)"

#else

#define ngx_tcp_nopush_n   "setsockopt(TCP_NOPUSH)"
#define ngx_tcp_push_n     "setsockopt(!TCP_NOPUSH)"

#endif


#define ngx_shutdown_socket    shutdown
#define ngx_shutdown_socket_n  "shutdown()"

#define ngx_close_socket    close
#define ngx_close_socket_n  "close() socket"


#else  /* ifdef NGX_RINA */

ngx_socket_t rina_socket(int domain, int type, int protocol);
int rina_nonblocking(ngx_socket_t s);
int rina_blocking(ngx_socket_t s);
int rina_nopush(ngx_socket_t s);
int rina_push(ngx_socket_t s);
int rina_shutdown(ngx_socket_t s, int flags);
int rina_close(ngx_socket_t s);
int rina_bind(ngx_socket_t s, const char *rina_appl_name,
               const char *rina_dif_name,
               const struct sockaddr *addr, socklen_t addrlen);
int rina_listen(ngx_socket_t s, int backlog);
int rina_accept(ngx_socket_t s,struct sockaddr *addr,
                 socklen_t *addrlen);
int rina_accept4(ngx_socket_t s, struct sockaddr *addr,
                  socklen_t *addrlen, int flags);
int rina_getsockopt(ngx_socket_t s, int level, int optname,
                     void *optval, socklen_t *optlen);
int rina_setsockopt(ngx_socket_t s, int level, int optname,
                     const void *optval, socklen_t optlen);

ssize_t rina_recv(ngx_connection_t *c, u_char *buf, size_t size);
ssize_t rina_readv_chain(ngx_connection_t *c, ngx_chain_t *in,
                          off_t limit);
ssize_t rina_udp_recv(ngx_connection_t *c, u_char *buf, size_t size);
ssize_t rina_send(ngx_connection_t *c, u_char *buf, size_t size);
ngx_chain_t *rina_writev_chain(ngx_connection_t *c, ngx_chain_t *in,
                                off_t limit);

#define ngx_socket          rina_socket
#define ngx_nonblocking     rina_nonblocking
#define ngx_blocking        rina_blocking
#define ngx_tcp_nopush      rina_nopush
#define ngx_tcp_push        rina_push
#define ngx_shutdown_socket rina_shutdown
#define ngx_close_socket    rina_close
#define ngx_bind            rina_bind
#define ngx_listen          rina_listen
#define ngx_accept          rina_accept
#define ngx_accept4         rina_accept4
#define ngx_getsockopt      rina_getsockopt
#define ngx_setsockopt      rina_setsockopt

#define ngx_socket_n        "socket()"
#define ngx_nonblocking_n   "ioctl(FIONBIO)"
#define ngx_blocking_n      "ioctl(!FIONBIO)"
#define ngx_tcp_nopush_n   "setsockopt(TCP_CORK)"
#define ngx_tcp_push_n     "setsockopt(!TCP_CORK)"
#define ngx_shutdown_socket_n  "shutdown()"
#define ngx_close_socket_n  "close() socket"

#define RINA_DEBUG
#ifdef RINA_DEBUG
#define rl_log  ngx_log_stderr
#else
#define rl_log
#endif

#endif /* NGX_RINA */

#endif /* _NGX_SOCKET_H_INCLUDED_ */
