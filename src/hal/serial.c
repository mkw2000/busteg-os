#include "hal/serial.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

static int add_if_serial(SerialDevice *devices, int count, int max_devices, const char *name) {
    if (count >= max_devices) {
        return count;
    }
    if (strncmp(name, "ttyUSB", 6) == 0 || strncmp(name, "ttyACM", 6) == 0) {
        snprintf(devices[count].path, sizeof(devices[count].path), "/dev/%s", name);
        return count + 1;
    }
    return count;
}

int Serial_ListDevices(SerialDevice *devices, int max_devices) {
    int count = 0;
    DIR *dir = opendir("/dev");
    if (!dir) {
        return 0;
    }
    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        count = add_if_serial(devices, count, max_devices, entry->d_name);
    }
    closedir(dir);
    return count;
}
