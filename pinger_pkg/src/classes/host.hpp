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
        int _pid;
        std::vector<std::string> split_args(std::string nping_args);

    public:
        Host(std::string ip, std::string mac, std::string interface);
        std::optional<PingRow> ping(int sock, bool flood_flag);
        int startFlood(std::string nping_args);
        void stopFlood();
};
#endif