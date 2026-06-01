#define _DEFAULT_SOURCE
#include "hal/serial.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static speed_t baud_to_termios(int baud_rate) {
    switch (baud_rate) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    default: return B115200;
    }
}

static int add_if_serial(SerialDevice *devices, int count, int max_devices, const char *name) {
    if (count >= max_devices) {
        return count;
    }
    if (strncmp(name, "ttyUSB", 6) == 0 ||
        strncmp(name, "ttyACM", 6) == 0 ||
        strncmp(name, "tty.usbserial", 13) == 0 ||
        strncmp(name, "cu.usbserial", 12) == 0 ||
        strncmp(name, "tty.usbmodem", 12) == 0 ||
        strncmp(name, "cu.usbmodem", 11) == 0 ||
        strncmp(name, "tty.wchusbserial", 16) == 0 ||
        strncmp(name, "cu.wchusbserial", 15) == 0) {
        snprintf(devices[count].path, sizeof(devices[count].path), "/dev/%.122s", name);
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

int Serial_Open(const char *path, int baud_rate) {
    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);
    speed_t speed = baud_to_termios(baud_rate);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);
    return fd;
}

void Serial_Close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

int Serial_WriteText(int fd, const char *text) {
    if (fd < 0 || !text) {
        return -1;
    }
    size_t length = strlen(text);
    ssize_t written = write(fd, text, length);
    if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;
    }
    return written < 0 ? -1 : (int)written;
}

int Serial_ReadAvailable(int fd, char *buffer, size_t size) {
    if (fd < 0 || !buffer || size == 0) {
        return -1;
    }
    ssize_t bytes = read(fd, buffer, size);
    if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;
    }
    return bytes < 0 ? -1 : (int)bytes;
}
