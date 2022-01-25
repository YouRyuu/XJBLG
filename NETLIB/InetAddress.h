#include <netinet/in.h>
#include <string>


class InetAddress
{
public:
    InetAddress(uint16_t port);
    InetAddress(std::string ip, uint16_t port);
    InetAddress(const struct sockaddr_in& addr):addr_(addr)
    {
    }
    
    sa_family_t family() const { return addr_.sin_family; }
    std::string toIP() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

private:
    struct sockaddr_in addr_;
};