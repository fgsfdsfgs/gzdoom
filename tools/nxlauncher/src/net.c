#include "config.h"
#include "net.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

char net_my_ip[64] = { 0 };

int NET_Init(void)
{
    socketInitializeDefault();
    NET_GetMyIP(net_my_ip, sizeof(net_my_ip));
    socketExit();
    return 0;
}

void NET_Free(void)
{
}

int NET_GetMyIP(char *out, int outlen)
{
    if (!out || !outlen) return -1;
    u32 ip = gethostid();
    snprintf(out, outlen, "%d.%d.%d.%d",
      ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    return 0;
}
