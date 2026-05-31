#ifndef CYBERDECK_SYSTEM_H
#define CYBERDECK_SYSTEM_H

#include <stddef.h>

typedef struct SystemInfo {
    char hostname[64];
    char uptime[64];
    char cpu_temp[64];
    char memory[64];
    char disk[64];
    char ip_address[64];
} SystemInfo;

void System_ReadInfo(SystemInfo *info);
int System_ListUsb(char lines[][96], int max_lines);

#endif
