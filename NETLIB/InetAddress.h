#include <netinet/in.h>
#include <string>

extern const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);

class InetAddress
{
public:
    InetAddress(uint16_t port);
    InetAddress(std::string ip, uint16_t port);
    InetAddress(const struct sockaddr_in& addr):addr_(addr)
    {
    }
    
    sa_family_t family() const { return addr_.sin_family; }
    //std::string toIP() const;
    //std::string toIpPort() const;
    //uint16_t toPort() const;

    const struct sockaddr* getSockAddr() const 
    {
        return sockaddr_cast(&addr_);
    }

    void setSockAddrInet(const struct sockaddr_in& addr)
    {
        addr_ = addr;
    }

private:
    struct sockaddr_in addr_;
};