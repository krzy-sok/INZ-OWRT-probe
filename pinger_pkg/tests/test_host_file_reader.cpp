#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>
#include <fstream>


#define private public
#include "../src/host_file_reader.hpp"
#include "../src/classes/helpers/ip_wildcard.hpp"
#undef private


// TEST_CASE("test pass"){
//     REQUIRE(true);
// }

// TEST_CASE("test fail"){
//     REQUIRE(false);
// }

TEST_CASE("add single host", "[include],[single]"){
    hostFileReader reader = hostFileReader("");
    std::array<std::string, 3> parsed_line = {"10.0.0.1", "00-15-5D-2A-E7-14", "eth0"};
    bool res = reader.add_single_host("10.0.0.1", "00-15-5D-2A-E7-14", "eth0", reader.include_hosts);

    REQUIRE(res);
    REQUIRE(reader.include_hosts.size() == 1);
    REQUIRE(reader.include_hosts[0].size() == 3);
    REQUIRE(reader.include_hosts[0][0] == parsed_line[0]);
    REQUIRE(reader.include_hosts[0][1] == parsed_line[1]);
    REQUIRE(reader.include_hosts[0][2] == parsed_line[2]);
}

TEST_CASE("add single host with range", "[include],[single],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_single_host("10.0.0.1-10.0.0.10", "00-15-5D-2A-E7-14", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add single host with wildcard", "[include],[single],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_single_host("10.0.0.00*", "00-15-5D-2A-E7-14", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add single host with invalid ip", "[include],[single],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_single_host("10.0.0.261", "00-15-5D-2A-E7-14", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}


TEST_CASE("add host range", "[include],[range]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.1-10.0.0.10", "N/A", "eth0", reader.include_hosts);

    REQUIRE(res);
    REQUIRE(reader.include_hosts.size() == 10);

    std::string partial_ip = "10.0.0.";
    for(int i=0; i<10; i++){
        REQUIRE(reader.include_hosts[i].size() == 3);
        REQUIRE(reader.include_hosts[i][0] == partial_ip + std::to_string(i+1));
        REQUIRE(reader.include_hosts[i][1] == "N/A");
        REQUIRE(reader.include_hosts[i][2] == "eth0");
    }
}

TEST_CASE("add host range with single host", "[include],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.1", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host range with wildcard", "[include],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.00*", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host range with invalid ip", "[include],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.251-10.0.0.261", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("try parse ip range", "[parse],[range]"){
    hostFileReader reader = hostFileReader("");
    std::string range = "10.0.0.1-10.0.0.10";
    std::optional<ipRange> parseRes = reader.try_parse_ip_range(range);

    REQUIRE(parseRes.has_value());
    ipRange rangeObj = parseRes.value();
    int curr = htonl(167772161);
    int end = htonl(167772170);

    REQUIRE(rangeObj.octets_curr == curr);
    REQUIRE(rangeObj.octets_end == end);
    for(int i=0; i<10;i++ ){
        rangeObj.next_address();
    }
    REQUIRE(rangeObj.next_address() == "");
}

TEST_CASE("try parse ip range with single host", "[parse],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    std::string range = "10.0.0.1";
    std::optional<ipRange> parseRes = reader.try_parse_ip_range(range);

    REQUIRE(!parseRes.has_value());
}

TEST_CASE("try parse ip range with wildcard", "[parse],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    std::string range = "10.0.0.00*";
    std::optional<ipRange> parseRes = reader.try_parse_ip_range(range);

    REQUIRE(!parseRes.has_value());
}

TEST_CASE("try parse ip range invalid ip", "[parse],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    std::string range = "10.0.0.251-10.0.0.261";
    std::optional<ipRange> parseRes = reader.try_parse_ip_range(range);

    REQUIRE(!parseRes.has_value());
}

TEST_CASE("parse line single host", "[parse],[single],[line]"){
    hostFileReader reader = hostFileReader("");
    std::stringstream line("include 10.0.0.1 00-15-5D-2A-E7-14 eth0");
    std::array<std::string, 3> parsed_line = {"10.0.0.1", "00-15-5D-2A-E7-14", "eth0"};
    reader.parse_host_line(line);

    REQUIRE(reader.include_hosts.size() == 1);
    REQUIRE(reader.include_hosts[0].size() == 3);
    REQUIRE(reader.include_hosts[0][0] == parsed_line[0]);
    REQUIRE(reader.include_hosts[0][1] == parsed_line[1]);
    REQUIRE(reader.include_hosts[0][2] == parsed_line[2]);
}

TEST_CASE("parse line single host exclude", "[parse],[single],[line],[exclude]"){
    hostFileReader reader = hostFileReader("");
    std::stringstream line("exclude 10.0.0.1 00-15-5D-2A-E7-14 eth0");
    std::array<std::string, 3> parsed_line = {"10.0.0.1", "00-15-5D-2A-E7-14", "eth0"};
    reader.parse_host_line(line);

    REQUIRE(reader.exclude_hosts.size() == 1);
    REQUIRE(reader.exclude_hosts[0].size() == 3);
    REQUIRE(reader.exclude_hosts[0][0] == parsed_line[0]);
    REQUIRE(reader.exclude_hosts[0][1] == parsed_line[1]);
    REQUIRE(reader.exclude_hosts[0][2] == parsed_line[2]);
}

TEST_CASE("parse line range", "[parse],[single],[line]"){
    hostFileReader reader = hostFileReader("");
    std::stringstream line("include 10.0.0.1-10.0.0.10 N/A eth0");
    reader.parse_host_line(line);

    REQUIRE(reader.include_hosts.size() == 10);

    std::string partial_ip = "10.0.0.";
    for(int i=0; i<10; i++){
        REQUIRE(reader.include_hosts[i].size() == 3);
        REQUIRE(reader.include_hosts[i][0] == partial_ip + std::to_string(i+1));
        REQUIRE(reader.include_hosts[i][1] == "N/A");
        REQUIRE(reader.include_hosts[i][2] == "eth0");
    }
}

TEST_CASE("parse line range exclude", "[parse],[single],[line],[exclude]"){
    hostFileReader reader = hostFileReader("");
    std::stringstream line("exclude 10.0.0.1-10.0.0.10 N/A eth0");
    reader.parse_host_line(line);

    REQUIRE(reader.exclude_hosts.size() == 10);
    REQUIRE(reader.include_hosts.size() == 0);

    std::string partial_ip = "10.0.0.";
    for(int i=0; i<10; i++){
        REQUIRE(reader.exclude_hosts[i].size() == 3);
        REQUIRE(reader.exclude_hosts[i][0] == partial_ip + std::to_string(i+1));
        REQUIRE(reader.exclude_hosts[i][1] == "N/A");
        REQUIRE(reader.exclude_hosts[i][2] == "eth0");
    }
}

TEST_CASE("parse include only file",  "[parse],[file],[include]"){
    std::ofstream testHosts("./test-hosts");
    testHosts << "include 10.0.0.1 n/a eth0"<<std::endl;
    testHosts << "include 10.0.0.10-10.0.0.20 n/a eth0" << std::endl;
    testHosts.close();

    hostFileReader reader = hostFileReader("./test-hosts");
    std::vector<Host> hosts = reader.read_host_file();
    REQUIRE(reader.include_hosts.size() == 12);
    REQUIRE(hosts.size() == 12);
    REQUIRE(hosts[0]._ip == "10.0.0.1");
    REQUIRE(hosts[0]._mac == "n/a");
    REQUIRE(hosts[0]._interface == "eth0");

    std::string partial_ip = "10.0.0.";
    for(int i=1; i<11; i++){
        REQUIRE(hosts[i]._ip == partial_ip + std::to_string(i+9));
        REQUIRE(hosts[i]._mac == "n/a");
        REQUIRE(hosts[i]._interface == "eth0");
    }

    std::remove("./test-hosts");
}

TEST_CASE("parse file include first",  "[parse],[file],[include],[exclude]"){
    std::ofstream testHosts("./test-hosts");

    testHosts << "include 10.0.0.10-10.0.0.20 n/a eth0" << std::endl;
    testHosts << "exclude 10.0.0.11 n/a eth0"<<std::endl;
    testHosts.close();

    hostFileReader reader = hostFileReader("./test-hosts");
    std::vector<Host> hosts = reader.read_host_file();
    REQUIRE(reader.include_hosts.size() == 11);
    REQUIRE(reader.exclude_hosts.size() == 1);
    REQUIRE(hosts.size() == 10);
    REQUIRE(hosts[0]._ip == "10.0.0.10");
    REQUIRE(hosts[0]._mac == "n/a");
    REQUIRE(hosts[0]._interface == "eth0");

    std::string partial_ip = "10.0.0.";
    for(int i=1; i<9; i++){
        REQUIRE(hosts[i]._ip == partial_ip + std::to_string(i+11));
        REQUIRE(hosts[i]._mac == "n/a");
        REQUIRE(hosts[i]._interface == "eth0");
    }

    std::remove("./test-hosts");
}

TEST_CASE("parse file include second",  "[parse],[file],[include],[exclude]"){
    std::ofstream testHosts("./test-hosts");

    testHosts << "exclude 10.0.0.11 n/a eth0"<<std::endl;
    testHosts << "include 10.0.0.10-10.0.0.20 n/a eth0" << std::endl;
    testHosts.close();

    hostFileReader reader = hostFileReader("./test-hosts");
    std::vector<Host> hosts = reader.read_host_file();
    REQUIRE(reader.include_hosts.size() == 11);
    REQUIRE(reader.exclude_hosts.size() == 1);
    REQUIRE(hosts.size() == 10);
    REQUIRE(hosts[0]._ip == "10.0.0.10");
    REQUIRE(hosts[0]._mac == "n/a");
    REQUIRE(hosts[0]._interface == "eth0");

    std::string partial_ip = "10.0.0.";
    for(int i=1; i<9; i++){
        REQUIRE(hosts[i]._ip == partial_ip + std::to_string(i+11));
        REQUIRE(hosts[i]._mac == "n/a");
        REQUIRE(hosts[i]._interface == "eth0");
    }

    std::remove("./test-hosts");
}

// TEST_CASE("add host wildcard", "[include],[wildcard]"){
//     hostFileReader reader = hostFileReader("");
//     bool res = reader.add_host_wildcard("10.0.0.*", "N/A", "eth0", reader.include_hosts);
//     REQUIRE(res);

//     REQUIRE(reader.include_hosts.size() == 10);

//     std::string partial_ip = "10.0.0.";
//     for(int i=0; i<10; i++){
//         REQUIRE(reader.include_hosts[i].size() == 3);
//         REQUIRE(reader.include_hosts[i][0] == partial_ip + std::to_string(i));
//         REQUIRE(reader.include_hosts[i][1] == "NA");
//         REQUIRE(reader.include_hosts[i][2] == "eth0");
//     }
// }

TEST_CASE("construct wildcard1"){
    ipWildcard wildcard = ipWildcard("10.0.0.*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==2);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard2"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==2);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard3"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*0");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==1);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard4"){
    ipWildcard wildcard = ipWildcard("10.0.0.*00");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==0);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard5"){
    ipWildcard wildcard = ipWildcard("10.0.0.*0");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==1);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard2"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==2);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard3"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*0");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==1);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard4"){
    ipWildcard wildcard = ipWildcard("10.0.0.*00");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0][0]==3);
    REQUIRE(wildcard.wildcard_indexes[0][1]==0);


    std::string res = wildcard.next_address();

    REQUIRE(wildcard.is_last_value == false);
    REQUIRE(res == "10.0.0.0");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 100);
}

TEST_CASE("construct with wildcards on all decials"){
    ipWildcard wildcard = ipWildcard("10.19*.1*1.*12");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 190);
    REQUIRE(wildcard.curr_octets[2] == 101);
    REQUIRE(wildcard.curr_octets[3] == 12);
}

TEST_CASE("add host wildcard too long octet", "[include],[wildcard],[illegal]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_wildcard("1000.0.0.00*", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host wildcard no wildcard", "[include],[wildcard],[illegal]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_wildcard("100.0.0.1", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host wildcard too big octet", "[include],[wildcard],[illegal]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_wildcard("666.0.0.1*", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host wildcard alphabetical chars", "[include],[wildcard],[illegal]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_wildcard("19a.0.0.1*", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host wildcard other non number", "[include],[wildcard],[illegal]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_wildcard("19-.0.0.1*", "N/A", "eth0", reader.include_hosts);

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}
