#ifndef HOSTFILEREADER
#define HOSTFILEREADER

#include <vector>
#include <map>

#include "classes/host.hpp"
#include "classes/helpers/ip_range.hpp"

class hostFileReader
{
    private:
        std::string _filepath;
        std::vector<std::array<std::string, 3>> include_hosts;
        std::vector<std::array<std::string, 3>> _hosts;
        // final once created
        std::vector<Host> hosts;

        void parse_arp_line(std::stringstream &line_stream);
        void parse_host_line(std::stringstream &line_stream);
        bool parseIp(std::string ip, int target[]);

        //
        void add_host(std::stringstream &line_stream);
        void exclude_host(std::stringstream &line_stream);

        bool add_single_host(std::string ip, std::string mac, std::string interface);
        bool add_host_range(std::string ip, std::string mac, std::string interface);
        bool add_host_wildcard(std::string ip, std::string mac, std::string interface);

        bool exclude_single_host(std::stringstream &line_stream);
        bool exclude_host_range(std::stringstream &line_stream);
        bool exclude_host_wildcard(std::stringstream &line_stream);

        // save parsed hosts in plain list for easier access on  2-nd and next calls
        void read_intermediate_file();
        void save_intermediate_file();

    public:
        hostFileReader(std::string filePath);
        std::vector<Host> read_host_file();
        std::optional<ipRange> try_parse_ip_range(std::string range);
};

#endif