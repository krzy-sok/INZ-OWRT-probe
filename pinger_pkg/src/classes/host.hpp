#ifndef HOST
#define HOST
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
        int _flood_flag;
        struct sockaddr_in dst;
        int _pid;
        std::vector<PingRow> ping_results;
        std::vector<std::string> split_args(std::string nping_args);

    public:
        Host(std::string ip, std::string mac, std::string interface);
        void ping(int sock, bool flood_flag);
        int startFlood(std::string nping_args);
        void stopFlood();
        std::string to_json();
};
#endif