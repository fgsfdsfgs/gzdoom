#pragma once

extern char net_my_ip[64];

int NET_Init(void);
void NET_Free(void);

int NET_GetMyIP(char *out, int outlen);
