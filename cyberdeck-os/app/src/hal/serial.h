#ifndef CYBERDECK_SERIAL_H
#define CYBERDECK_SERIAL_H

#include <stddef.h>

typedef struct SerialDevice {
    char path[128];
} SerialDevice;

int Serial_ListDevices(SerialDevice *devices, int max_devices);

#endif
