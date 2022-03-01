#ifndef WS_SOCKETSOPS_H
#define WS_SOCKETSOPS_H

#include <arpa/inet.h>


void setNonblockAndCloseOnExec(int sockfd);
int createNonblockingOrDie(sa_family_t family);
int createBlocking(sa_family_t family);
int connect_(int sockfd, const struct sockaddr* addr);
void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
int accept_(int sockfd, struct sockaddr_in* addr);
ssize_t read_(int sockfd, void* buf, size_t size);
ssize_t write_(int sockfd, const void* buf, size_t size);
ssize_t readv_(int sockfd, const struct iovec* iov, int iovcount);
void close_(int sockfd);
void shutdownWrite(int sockfd);
int getSocketError(int sockfd);
bool isSelfConnect(int sockfd);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

#endif