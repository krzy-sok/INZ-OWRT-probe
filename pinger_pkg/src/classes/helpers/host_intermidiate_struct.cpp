#include <string>

struct host_intermidiate{
    std::string ip;
    std::string mac;
    std::string interface;
    bool operator==(const host_intermidiate &other) const {
        return ip==other.ip;
    }
    bool operator<(const host_intermidiate &other) const {
        return ip<other.ip;
    }
    host_intermidiate& operator=(const host_intermidiate& other){
        ip=other.ip;
        mac=other.mac;
        interface=other.interface;
        return *this;
    }
};