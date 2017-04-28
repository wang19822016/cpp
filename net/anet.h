#ifndef ANET_H
#define ANET_H

#include <sys/types.h>
#include <sys/socket.h>

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

// domain - AF_INET AF_INET6
// type - SOCK_STREAM SOCK_DGRAM
int anetCreateSocket(char *err, int domain, int type);
int anetSetReuseAddr(char *err, int fd);
int anetSetBlock(char *err, int fd, int non_block); // 1=no block
int anetSetTcpNoDelay(char *err, int fd, int val);// 1=no delay
int anetResolve(char *err, const char *host, char *ipbuf, size_t ipbuf_len);
ssize_t anetRead(int fd, char *buf, int count, int *err);
ssize_t anetWrite(int fd, const char *buf, int count, int *err);
int anetBind(char *err, int fd, int domain, const char *addr, int port);
int anetListen(char *err, int fd, int backlog);
int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port);
int anetTcpConnect(char *err, int domain, const char *addr, int port);
int anetTcpNonBlockConnect(char *err, int domain, const char *addr, int port);
int anetTcpServer(char *err, int domain, const char *bindaddr, int port, int backlog);
int anetGetSocketError(int fd);

#endif
