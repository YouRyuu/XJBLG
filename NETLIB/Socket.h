#ifndef SOCKET_H
#define SOCKET_H

class InetAddress;
struct tcp_info;

class Socket
{
    public:
        Socket(int sockfd)
        :sockfd_(sockfd)
        { }

        ~Socket();

        int fd() const 
        {
            return sockfd_;
        }

        //bool getTcpInfo(struct tco_info*) const;
        
        //bool getTcpInfoString(char* buf, int len) const;

        void bindAddress(const InetAddress& localAddr);

        void listen();

        int accept(InetAddress* peerAddr);

        void shutdownWrite_();

        void setReuseAddr(bool on);

        void setReusePort(bool on);

        void setKeepAlive(bool on);

    private:
        const int sockfd_;
};

#endif