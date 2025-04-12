// test to ping a machine of ip given on program call
//  to see how to ping, extract info and what will happen if adress is not valid

// references
// https://nsrc.org/wrc/materials/src/ping.c
// https://github.com/amitsaha/ping/blob/master/ping.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>


// //  ping a given ip
// // will send a few packets ~4
// // will get ip from rotate_target
// int ping_host()
// {


// }

// // host discovery
// // get ip and mac addrsess of devices connected to a given network
// // device running this program must be connected to the given network
// int host_discovery()
// {

// }

// // ping hosts cyclicly
// // periods should be configurable
// // rotation alg should be configurable
// // rotation algs will be in superate functions
// int rotate_target()
// {

// }

// // introduce dalays between ping periods
// int delay()
// {

// }


// // send collected data to openwisp-api
// // will be extracted to different file
// int send_to_api()
// {

// }

// create socket
// get network ip from user
// will be extracted different file
int main(int argc, char **argv)
{
    int sock;
    // struct sockaddr_in source = {.sin_family = AF_INET};
    struct sockaddr_in dst;

    if(argc < 2){
        printf("Ip address needed\n");
        exit(-1);
    }
    // check if given ip is valid
    if (inet_aton(argv[1], &dst.sin_addr) == 0){
        perror("Invalid ip addr\n");
        exit(-1);
    }

    // crate icmp socket
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        perror("Error crating socket\n");
        exit(-2);
    }
    printf("created socket\n");

    close(sock);
    printf("closed socket\n");
    return 0;
}