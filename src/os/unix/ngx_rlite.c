
/*
 * Copyright (C) Vincenzo Maffione
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <fcntl.h>

#include <rlite/rlite.h>
#include <rlite/list.h>


struct ngx_rlite_dev {
    struct rl_ctrl ctrl;
    struct list_head node;
};

static struct list_head rlite_devs;

void
rlite_init(void)
{
    list_init(&rlite_devs);
}

static struct ngx_rlite_dev *
dev_lookup(int fd)
{
    struct ngx_rlite_dev *cur;

    list_for_each_entry(cur, &rlite_devs, node) {
        if (cur->ctrl.rfd == fd) {
            return cur;
        }
    }

    return NULL;
}

static int
inet4(ngx_socket_t s)
{
    char buf[sizeof(struct sockaddr_in6)];
    struct sockaddr *addr = (struct sockaddr *)buf;
    socklen_t addrlen = sizeof(buf);
    int ret;

    ret = getsockname(s, addr, &addrlen);

    return (ret == 0) && (addr->sa_family == AF_INET);
}

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
    struct ngx_rlite_dev *dev;
    int ret;

    if (domain != AF_INET) {
        return socket(domain, type, protocol);
    }

    dev = malloc(sizeof(*dev));
    if (!dev) {
        errno = ENOMEM;
        return -1;
    }

    ret = socket(domain, type, protocol);

    ret = rl_ctrl_init(&dev->ctrl, NULL, 0);
    if (ret) {
        free(dev);
    } else {
        list_add_tail(&dev->node, &rlite_devs);
    }

    rl_log(0, "socket(%d,%d,%d) --> %d", domain, type, protocol, ret);

    return dev->ctrl.rfd;
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
    struct ngx_rlite_dev *dev;
    int ret;

    if (is_socket(s)) {
        return close(s);
    }

    if ((dev = dev_lookup(s))) {
        /* Control device. */
        list_del(&dev->node);
        ret = rl_ctrl_fini(&dev->ctrl);
        free(dev);

    } else {
        /* I/O device */
        ret = close(s);
    }

    rl_log(0, "close(%d)", s);

    return ret;
}

int
rlite_bind(ngx_socket_t s, const char *rina_appl_name,
           const char *rina_dif_name,
           const struct sockaddr *addr, socklen_t addrlen)
{
    char strbuf[INET_ADDRSTRLEN];
    struct ngx_rlite_dev *dev;
    struct rina_name srv_name;
    int ret;

    if (!(dev = dev_lookup(s))) {
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

    if (rina_name_from_string(rina_appl_name, &srv_name)) {
        ngx_log_stderr(0, "Invalid RINA appl name %s", rina_appl_name);
        errno = EINVAL;
        return -1;
    }

    ret = rl_ctrl_register(&dev->ctrl, rina_dif_name, &srv_name);
    rina_name_free(&srv_name);

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
rlite_accept_intn(struct ngx_rlite_dev *dev, struct sockaddr_in *addr,
                  socklen_t *addrlen)
{
    int fd;

    fd = rl_ctrl_flow_accept(&dev->ctrl);
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
rlite_accept(ngx_socket_t s,struct sockaddr *addr,
             socklen_t *addrlen)
{
    struct ngx_rlite_dev *dev;
    int ret;

    if (!(dev = dev_lookup(s))) {
        return accept(s, addr, addrlen);
    }

    ret = rlite_accept_intn(dev, (struct sockaddr_in *)addr, addrlen);

    rl_log(0, "accept(%d) --> %d", s, ret);

    return ret;
}

int
rlite_accept4(ngx_socket_t s,struct sockaddr *addr,
              socklen_t *addrlen, int flags)
{
    struct ngx_rlite_dev *dev;
    int ret;

    if (!(dev = dev_lookup(s))) {
        return accept4(s, addr, addrlen, flags);
    }

    ret = rlite_accept_intn(dev, (struct sockaddr_in *)addr, addrlen);

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
