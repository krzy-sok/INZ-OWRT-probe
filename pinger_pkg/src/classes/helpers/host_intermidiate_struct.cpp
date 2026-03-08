#include <string>
#include <arpa/inet.h>

struct host_intermidiate{
    std::string ip;
    std::string mac;
    std::string interface;
    bool operator==(const host_intermidiate &other) const {
        return ip==other.ip;
    }
    bool operator<(const host_intermidiate &other) const {
        in_addr ours, theirs;
        inet_aton(ip.data(), &ours);
        inet_aton(other.ip.data(), &theirs);
        return htonl(ours.s_addr)<htonl(theirs.s_addr);
    }
};