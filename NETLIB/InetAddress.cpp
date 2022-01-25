#include "InetAddress.h"
#include <netdb.h>

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
    addr_.sin_port = port;  //fix
    addr_.sin_addr.s_addr = ip; //fix
}

InetAddress::InetAddress(std::string ip, uint16_t port)
{
    
}