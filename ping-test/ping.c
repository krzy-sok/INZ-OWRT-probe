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


//  ping a given ip
// will send a few packets ~4
// will get ip from rotate_target
int ping_host()
{


}

// host discovery
// get ip and mac addrsess of devices connected to a given network
// device running this program must be connected to the given network
int host_discovery()
{

}

// ping hosts cyclicly
// periods should be configurable
// rotation alg should be configurable
// rotation algs will be in superate functions
int rotate_target()
{

}


// send collected data to openwisp-api
// will be extracted to different file
int send_to_api()
{

}

// create socket
// get network ip from user
// will be extracted different file
int main()
{

}