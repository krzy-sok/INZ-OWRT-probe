#ifndef HOST
#define HOSt
#include <string>
#include <netdb.h>
#include "ping_row.hpp"

class Host
{
    private:
        std::string ip;
        std::string mac;
        std::string interface;
        struct sockaddr_in dst;

    public:
        Host(std::string ip, std::string mac, std::string interface);
        PingRow ping(int sock);
};
#endif