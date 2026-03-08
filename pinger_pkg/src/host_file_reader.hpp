#ifndef HOSTFILEREADER
#define HOSTFILEREADER

#include <vector>

#include "classes/host.hpp"
#include "classes/helpers/ip_range.hpp"
#include "classes/helpers/host_intermidiate_struct.cpp"

class hostFileReader
{
    private:
        std::string _filepath;
        std::vector<host_intermidiate> include_hosts;
        std::vector<host_intermidiate> exclude_hosts;
        std::vector<Host> hosts;
        std::string ARP_PATH;

        void parse_arp_line(std::stringstream &line_stream);
        void parse_host_line(std::stringstream &line_stream);
        bool parseIp(std::string ip, int target[]);
        std::optional<ipRange> try_parse_ip_range(std::string range);

        //
        void add_host(std::stringstream &line_stream,  std::vector<host_intermidiate> &target);

        bool add_single_host(std::string ip, std::string mac, std::string interface, std::vector<host_intermidiate> &target );
        bool add_host_range(std::string ip, std::string mac, std::string interface,  std::vector<host_intermidiate> &target);
        bool add_host_wildcard(std::string ip, std::string mac, std::string interface,  std::vector<host_intermidiate> &target);

    public:
        hostFileReader(std::string filePath);
        std::vector<Host> read_host_file();

};

#endif