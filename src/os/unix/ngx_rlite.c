
/*
 * Copyright 2016 (C) Vincenzo Maffione
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <fcntl.h>

#include <rina/api.h>

static int
is_socket(ngx_socket_t s)
{
    char buf[sizeof(struct sockaddr_in6)];
    struct sockaddr *addr = (struct sockaddr *)buf;
    socklen_t addrlen = sizeof(buf);
    int ret;

    ret = getsockname(s, addr, &addrlen);

    return !ret;
}

ngx_socket_t
rlite_socket(int domain, int type, int protocol)
{
    int fd;

    if (domain != AF_INET) {
        return socket(domain, type, protocol);
    }

    fd = rina_open();
    rl_log(0, "socket(%d,%d,%d) --> %d", domain, type, protocol, fd);

    return fd;
}

int
rlite_nonblocking(ngx_socket_t s)
{
    if (!is_socket(s)) {
        rl_log(0, "nonblocking(%d)", s);
    }

    return fcntl(s, F_SETFL, O_NONBLOCK);
}

int
rlite_blocking(ngx_socket_t s)
{
    int flags;

    if (!is_socket(s)) {
        rl_log(0, "blocking(%d)", s);
    }

    flags = fcntl(s, F_GETFL);
    if (flags < 0) {
        return flags;
    }

    flags &= ~O_NONBLOCK;

    return fcntl(s, F_SETFL, flags);
}

int
rlite_nopush(ngx_socket_t s)
{
    if (is_socket(s)) {
        return 0;
    }

    rl_log(0, "nopush(%d)", s);

    return 0;
}

int
rlite_push(ngx_socket_t s)
{
    if (is_socket(s)) {
        return 0;
    }

    rl_log(0, "push(%d)", s);

    return 0;
}

int
rlite_shutdown(ngx_socket_t s, int flags)
{
    if (is_socket(s)) {
        return shutdown(s, flags);
    }

    rl_log(0, "shutdown(%d)", s);

    return 0;
}

int
rlite_close(ngx_socket_t s)
{
    if (!is_socket(s)) {
        rl_log(0, "close(%d)", s);
    }

    return close(s);
}

int
rlite_bind(ngx_socket_t s, const char *rina_appl_name,
           const char *rina_dif_name,
           const struct sockaddr *addr, socklen_t addrlen)
{
    char strbuf[INET_ADDRSTRLEN];
    int ret;

    if (is_socket(s)) {
        return bind(s, addr, addrlen);
    }

    memset(strbuf, '\0', sizeof(strbuf));
    inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr,
              strbuf, sizeof(strbuf));

    rl_log(0, "bind(%d,'%s')", s, strbuf);

    if (!rina_appl_name || !rina_dif_name) {
        ngx_log_stderr(0, "NULL RINA names");
        errno = EINVAL;
        return -1;
    }

    ret = rina_register(s, rina_dif_name, rina_appl_name);

    return ret;
}

int rlite_listen(ngx_socket_t s, int backlog)
{
    if (is_socket(s)) {
        return listen(s, backlog);
    }

    rl_log(0, "listen(%d,%d)", s, backlog);

    return 0;
}

static int
rlite_accept_intn(ngx_socket_t s, struct sockaddr_in *addr,
                  socklen_t *addrlen)
{
    int fd;

    fd = rina_flow_accept(s, NULL, NULL, 0);
    if (fd < 0) {
        return fd;
    }

    if (*addrlen < sizeof(struct sockaddr_in)) {
        close(fd);
        errno = ENOSPC;
        return -1;
    }

    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons((5000 + fd) & 0xffff);
    addr->sin_addr.s_addr = htonl(0x0AA80000 + fd);
    *addrlen = sizeof(struct sockaddr_in);

    return fd;
}

int
rlite_accept(ngx_socket_t s, struct sockaddr *addr,
             socklen_t *addrlen)
{
    int ret;

    if (is_socket(s)) {
        return accept(s, addr, addrlen);
    }

    ret = rlite_accept_intn(s, (struct sockaddr_in *)addr, addrlen);

    rl_log(0, "accept(%d) --> %d", s, ret);

    return ret;
}

int
rlite_accept4(ngx_socket_t s, struct sockaddr *addr,
              socklen_t *addrlen, int flags)
{
    int ret;

    if (is_socket(s)) {
        return accept4(s, addr, addrlen, flags);
    }

    ret = rlite_accept_intn(s, (struct sockaddr_in *)addr, addrlen);

    rl_log(0, "accept4(%d) --> %d", s, ret);

    return ret;
}

int
rlite_getsockopt(ngx_socket_t s, int level, int optname,
                 void *optval, socklen_t *optlen)
{
    if (is_socket(s)) {
        return getsockopt(s, level, optname, optval, optlen);
    }

    rl_log(0, "getsockopt(%d,%d,%d)", s, level, optname);

    return 0;
}

int
rlite_setsockopt(ngx_socket_t s, int level, int optname,
                 const void *optval, socklen_t optlen)
{
    if (is_socket(s)) {
        return setsockopt(s, level, optname, optval, optlen);
    }

    rl_log(0, "setsockopt(%d,%d,%d)", s, level, optname);

    return 0;
}
