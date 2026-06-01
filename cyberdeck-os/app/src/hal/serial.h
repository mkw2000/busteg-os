#ifndef CYBERDECK_SERIAL_H
#define CYBERDECK_SERIAL_H

#include <stddef.h>

typedef struct SerialDevice {
    char path[128];
} SerialDevice;

int Serial_ListDevices(SerialDevice *devices, int max_devices);
int Serial_Open(const char *path, int baud_rate);
void Serial_Close(int fd);
int Serial_WriteText(int fd, const char *text);
int Serial_ReadAvailable(int fd, char *buffer, size_t size);

#endif
