#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "classes/host.hpp"
#include "classes/ping_row.hpp"

// ISSUE: ping somehow succeds when host is unreachable
int main()
{
    Host host = Host("8.8.8.8", "no", "no");

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        perror("Error crating socket\n");
        exit(-2);
    }
    std::cout<<"created socket\n";

    auto pingres = host.ping(sock);
    if(pingres.has_value())
    {
        PingRow ping = pingres.value();
        std::cout<< ping.to_string();
    }
    else
    {
        std::cout << "Ping to host failed \n";
    }

    return 0;
}