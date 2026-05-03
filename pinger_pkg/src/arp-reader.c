/* soln from http://codereview.stackexchange.com/questions/58097/parsing-arp-cache-in-c  */

#include <stdio.h>

#define ARP_CACHE       "/proc/net/arp"
#define ARP_STRING_LEN  200
#define ARP_BUFFER_LEN  (ARP_STRING_LEN + 1)

#define ARP_LINE_FORMAT "%100s %*s 0x%100s %100s %*s %100s"

int main()
{
    FILE *arpCache = fopen(ARP_CACHE, "r");
    if (!arpCache)
    {
        perror("Arp Cache: Failed to open file \"" ARP_CACHE "\"");
        return 1;
    }

    /* Ignore the first line, which contains the header */
    char header[ARP_BUFFER_LEN];
    if (!fgets(header, sizeof(header), arpCache))
    {
        return 1;
    }

    char ipAddr[ARP_BUFFER_LEN], hwAddr[ARP_BUFFER_LEN], device[ARP_BUFFER_LEN], state[ARP_BUFFER_LEN];
    int count = 0;
    while (4 == fscanf(arpCache, ARP_LINE_FORMAT, ipAddr, state, hwAddr, device))
    {
        printf("%03d: Mac Address of [%s]\t on [%s] is \"%s\"  State: %s\n",
                ++count, ipAddr, device, hwAddr, state);
    }
    fclose(arpCache);
    return 0;
}