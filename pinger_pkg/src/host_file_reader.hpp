#ifndef HOSTFILEREADER
#define HOSTFILEREADER

#include <vector>
#include <array>

#include "classes/host.hpp"
#include "classes/helpers/ip_range.hpp"

class hostFileReader
{
    private:
        std::string _filepath;
        std::vector<std::array<std::string, 3>> include_hosts;
        std::vector<std::array<std::string, 3>> exclude_hosts;
        std::vector<Host> hosts;

        void parse_arp_line(std::stringstream &line_stream);
        void parse_host_line(std::stringstream &line_stream);
        bool parseIp(std::string ip, int target[]);

        //
        void add_host(std::stringstream &line_stream,  std::vector<std::array<std::string, 3>> &target);

        bool add_single_host(std::string ip, std::string mac, std::string interface, std::vector<std::array<std::string, 3>> &target );
        bool add_host_range(std::string ip, std::string mac, std::string interface,  std::vector<std::array<std::string, 3>> &target);
        bool add_host_wildcard(std::string ip, std::string mac, std::string interface,  std::vector<std::array<std::string, 3>> &target);

    public:



        hostFileReader(std::string filePath);
        std::vector<Host> read_host_file();
        std::optional<ipRange> try_parse_ip_range(std::string range);
};

#endif