// Custom ping implementation in C
// For purpose of probing hosts in network and measuring its rtt
// rtt, ip


// reference https://www.geeksforgeeks.org/ping-in-c/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>


// #define pings  64;
// global vars not in define for ability to set them from call arguments
// how many packets should be send to a single host in each cycle
int PING_NUM;
const int pkt_size = 64;
