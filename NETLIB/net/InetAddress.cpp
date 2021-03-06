#include "InetAddress.h"
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

InetAddress::InetAddress(uint16_t port)
{
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = INADDR_ANY;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(ip); 
}

InetAddress::InetAddress(std::string ip, uint16_t port)
{
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr)<=0)
    {
        printf("InetAddress::inet_pton error\n");
        exit(-1);
    }
}