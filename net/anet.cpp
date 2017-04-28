#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "anet.h"

void anetSetError(char *err, const char *fmt, ...)
{
    va_list ap;

    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}

int anetCreateSocket(char *err, int domain, int type)
{
    int s = socket(domain, type, 0);
    if (s == -1)
    {
        anetSetError(err, "creating socket: %s", strerror(errno));
        return ANET_ERR;
    }

    return s;
}

int anetSetReuseAddr(char *err, int fd)
{
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        anetSetError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetSetBlock(char *err, int fd, int non_block)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
        anetSetError(err, "fcntl(F_GETFL): %s", strerror(errno));
        return ANET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        anetSetError(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetSetTcpNoDelay(char *err, int fd, int val)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
        anetSetError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetResolve(char *err, const char *host, char *ipbuf, size_t ipbuf_len)
{
    struct addrinfo hints, *info;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0)
    {
        anetSetError(err, "getaddrinfo: %s", gai_strerror(rv));
        return ANET_ERR;
    }

    if (info->ai_family == AF_INET)
    {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        if (inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len))
        {
            anetSetError(err, "inet_ntop: %s", strerror(errno));
            freeaddrinfo(info);
            return ANET_ERR;
        }
    }
    else
    {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        if (inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len))
        {
            anetSetError(err, "inet_ntop: %s", strerror(errno));
            freeaddrinfo(info);
            return ANET_ERR;
        }
    }

    freeaddrinfo(info);
    return ANET_OK;
}

ssize_t anetRead(int fd, char *buf, int count, int *err)
{
    ssize_t nread, totlen = 0;
    *err = ANET_OK;
    while (totlen != count)
    {
        nread = read(fd, buf, count - totlen);
        if (nread == 0) break;
        if (nread == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                *err = ANET_ERR;
            }
            break;
        }
        totlen += nread;
        buf += nread;
    }
    return totlen;
}

ssize_t anetWrite(int fd, const char *buf, int count, int *err)
{
    ssize_t nwritten, totlen = 0;
    *err = ANET_OK;
    while (totlen != count)
    {
        nwritten = write(fd, buf, count - totlen);
        if (nwritten == 0) break;
        if (nwritten == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                *err = ANET_ERR;
            }
            break;
        }
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

int anetBind(char *err, int fd, int domain, const char *addr, int port)
{
    struct sockaddr *s_addr_ptr = nullptr;
    socklen_t s_addr_len = 0;
    if (domain == AF_INET)
    {
        struct sockaddr_in s_addr;
        bzero(&s_addr, sizeof(s_addr));
        s_addr.sin_family = AF_INET;
        s_addr.sin_addr.s_addr = inet_addr(addr);
        s_addr.sin_port = htons(port);

        s_addr_ptr = (struct sockaddr *)&s_addr;
        s_addr_len = sizeof(s_addr);
    }
    else
    {
        struct sockaddr_in6 s_addr;
        bzero(&s_addr, sizeof(s_addr));
        s_addr.sin6_family = AF_INET6;
        s_addr.sin6_port = htons(port);
        if (inet_pton(AF_INET6, addr, &s_addr.sin6_addr))
        {
            anetSetError(err, "inet_ntop: %s", strerror(errno));
            return ANET_ERR;
        }

        s_addr_ptr = (struct sockaddr *)&s_addr;
        s_addr_len = sizeof(s_addr);
    }

    if (bind(fd, s_addr_ptr, s_addr_len) == -1)
    {
        anetSetError(err, "bind: %s", strerror(errno));
        return ANET_ERR;
    }

    return ANET_OK;
}

int anetListen(char *err, int fd, int backlog)
{
    if (listen(fd, backlog) == -1)
    {
        anetSetError(err, "listen: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port)
{
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    while(1)
    {
        fd = accept(s, (struct sockaddr*)&sa, &salen);
        if (fd == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                anetSetError(err, "accept: %s", strerror(errno));
                return ANET_ERR;
            }
        }
        break;
    }

    if (sa.ss_family == AF_INET)
    {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (ip) inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin_port);
    }
    else
    {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (ip) inet_ntop(AF_INET6, (void*)&(s->sin6_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin6_port);
    }
    return fd;
}

int anetTcpConnect(char *err, int domain, const char *addr, int port)
{
    int s = anetCreateSocket(err, domain, SOCK_STREAM);
    if (s == ANET_ERR) return ANET_ERR;

    struct sockaddr *s_addr_ptr = nullptr;
    socklen_t s_addr_len = 0;
    if (domain == AF_INET)
    {
        struct sockaddr_in s_addr;
        bzero(&s_addr, sizeof(s_addr));
        s_addr.sin_family = AF_INET;
        s_addr.sin_addr.s_addr = inet_addr(addr);
        s_addr.sin_port = htons(port);

        s_addr_ptr = (struct sockaddr *)&s_addr;
        s_addr_len = sizeof(s_addr);
    }
    else
    {
        struct sockaddr_in6 s_addr;
        bzero(&s_addr, sizeof(s_addr));
        s_addr.sin6_family = AF_INET6;
        s_addr.sin6_port = htons(port);
        if (inet_pton(AF_INET6, addr, &s_addr.sin6_addr))
        {
            anetSetError(err, "inet_ntop: %s", strerror(errno));
            return ANET_ERR;
        }

        s_addr_ptr = (struct sockaddr *)&s_addr;
        s_addr_len = sizeof(s_addr);
    }

    if (connect(s, s_addr_ptr, s_addr_len) == -1)
    {
        anetSetError(err, "connect: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return s;
}

int anetTcpNonBlockConnect(char *err, int domain, const char *addr, int port)
{
    int s = anetCreateSocket(err, domain, SOCK_STREAM);
    if (s == ANET_ERR) return ANET_ERR;

    if (anetSetBlock(err, s, 1) == ANET_ERR)
    {
        close(s);
        return ANET_ERR;
    }

    struct sockaddr *s_addr_ptr = nullptr;
    socklen_t s_addr_len = 0;
    if (domain == AF_INET)
    {
        struct sockaddr_in s_addr;
        bzero(&s_addr, sizeof(s_addr));
        s_addr.sin_family = AF_INET;
        s_addr.sin_addr.s_addr = inet_addr(addr);
        s_addr.sin_port = htons(port);

        s_addr_ptr = (struct sockaddr *)&s_addr;
        s_addr_len = sizeof(s_addr);
    }
    else
    {
        struct sockaddr_in6 s_addr;
        bzero(&s_addr, sizeof(s_addr));
        s_addr.sin6_family = AF_INET6;
        s_addr.sin6_port = htons(port);
        if (inet_pton(AF_INET6, addr, &s_addr.sin6_addr))
        {
            anetSetError(err, "inet_ntop: %s", strerror(errno));
            return ANET_ERR;
        }

        s_addr_ptr = (struct sockaddr *)&s_addr;
        s_addr_len = sizeof(s_addr);
    }

    if (connect(s, s_addr_ptr, s_addr_len) == -1)
    {
        if (errno != EINPROGRESS)
        {
            anetSetError(err, "connect: %s", strerror(errno));
            close(s);
            return ANET_ERR;
        }
    }

    return s;
}

int anetTcpServer(char *err, int domain, const char *bindaddr, int port, int backlog)
{
    int s = anetCreateSocket(err, domain, SOCK_STREAM);
    if (s == ANET_ERR) return ANET_ERR;

    if (anetSetReuseAddr(err, s) == ANET_ERR)
    {
        close(s);
        return ANET_ERR;
    }

    if (anetBind(err, s, domain, bindaddr, port) == ANET_ERR)
    {
        close(s);
        return ANET_ERR;
    }

    if (anetListen(err, s, backlog) == ANET_ERR)
    {
        close(s);
        return ANET_ERR;
    }

    return s;
}

int anetGetSocketError(int fd)
{
    int status, err;
    socklen_t len;

    status = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
    return err;
}
