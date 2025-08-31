#ifndef HOST
#define HOSt
#include <string>
#include <netdb.h>
#include <optional>
#include "ping_row.hpp"

class Host
{
    private:
        std::string _ip;
        std::string _mac;
        std::string _interface;
        struct sockaddr_in dst;

    public:
        Host(std::string ip, std::string mac, std::string interface);
        std::optional<PingRow> ping(int sock);
};
#endif