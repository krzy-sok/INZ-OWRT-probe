#ifndef HOST
#define HOSt
#include <string>
#include <netdb.h>
#include <optional>
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
        std::optional<PingRow> ping(int sock);
};
#endif