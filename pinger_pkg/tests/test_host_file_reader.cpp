#include <catch2/catch_test_macros.hpp>
#include <sstream>


#define private public
#include "../src/host_file_reader.hpp"
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
    bool res = reader.add_single_host("10.0.0.1", "00-15-5D-2A-E7-14", "eth0");

    REQUIRE(res);
    REQUIRE(reader.include_hosts.size() == 1);
    REQUIRE(reader.include_hosts[0].size() == 3);
    REQUIRE(reader.include_hosts[0][0] == parsed_line[0]);
    REQUIRE(reader.include_hosts[0][1] == parsed_line[1]);
    REQUIRE(reader.include_hosts[0][2] == parsed_line[2]);
}

TEST_CASE("add single host with range", "[include],[single],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_single_host("10.0.0.1-10.0.0.10", "00-15-5D-2A-E7-14", "eth0");

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add single host with wildcard", "[include],[single],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_single_host("10.0.0.00*", "00-15-5D-2A-E7-14", "eth0");

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add single host with invalid ip", "[include],[single],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_single_host("10.0.0.261", "00-15-5D-2A-E7-14", "eth0");

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}


TEST_CASE("add host range", "[include],[range]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.1-10.0.0.10", "N/A", "eth0");

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
    bool res = reader.add_host_range("10.0.0.1", "N/A", "eth0");

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host range with wildcard", "[include],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.00*", "N/A", "eth0");

    REQUIRE(!res);
    REQUIRE(reader.include_hosts.size() == 0);
}

TEST_CASE("add host range with invalid ip", "[include],[range],[negative]"){
    hostFileReader reader = hostFileReader("");
    bool res = reader.add_host_range("10.0.0.251-10.0.0.261", "N/A", "eth0");

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

// TEST_CASE("add host wildcard", "[include],[wildcard]"){
//     hostFileReader reader = hostFileReader("");
//     std::stringstream host_line("10.0.0.00* N/A eth0");
//     reader.add_host(host_line);

//     REQUIRE(reader.include_hosts.size() == 10);

//     std::string partial_ip = "10.0.0.";
//     for(int i=0; i<10; i++){
//         REQUIRE(reader.include_hosts[i].size() == 3);
//         REQUIRE(reader.include_hosts[i][0] == partial_ip + std::to_string(i));
//         REQUIRE(reader.include_hosts[i][1] == "NA");
//         REQUIRE(reader.include_hosts[i][2] == "eth0");
//     }
// }
