#include <arpa/inet.h>

int createNonblockingOrDie(sa_family_t family);
int connect(int sockfd, const struct sockaddr* addr);
void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
int accept_(int sockfd, struct sockaddr_in* addr);
ssize_t read(int sockfd, void* buf, size_t size);
ssize_t write(int sockfd, const void* buf, size_t size);
void close(int sockfd);
void shutdownWrite(int sockfd);
int getSocketError(int sockfd);
bool isSelfConnect(int sockfd);