#define _DEFAULT_SOURCE
#include "hal/system.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <unistd.h>

static void read_first_line(const char *path, char *out, size_t size, const char *fallback) {
    FILE *file = fopen(path, "r");
    if (!file) {
        snprintf(out, size, "%s", fallback);
        return;
    }
    if (!fgets(out, (int)size, file)) {
        snprintf(out, size, "%s", fallback);
    }
    out[strcspn(out, "\r\n")] = '\0';
    fclose(file);
}

static void read_driver_name(const char *device_name, char *out, size_t size) {
    char driver_path[224];
    char link_target[224];
    ssize_t length = 0;

    snprintf(driver_path, sizeof(driver_path), "/sys/bus/usb/devices/%s/driver", device_name);
    length = readlink(driver_path, link_target, sizeof(link_target) - 1);
    if (length < 0) {
        snprintf(out, size, "none");
        return;
    }

    link_target[length] = '\0';
    char *name = strrchr(link_target, '/');
    snprintf(out, size, "%s", name ? name + 1 : link_target);
}

static bool usb_device_has_product_file(const char *device_name) {
    char path[192];
    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", device_name);
    return access(path, R_OK) == 0;
}

static bool usb_device_is_interface(const char *device_name) {
    return strchr(device_name, ':') != NULL;
}

void System_ReadInfo(SystemInfo *info) {
    if (gethostname(info->hostname, sizeof(info->hostname)) != 0) {
        snprintf(info->hostname, sizeof(info->hostname), "unknown");
    }

    FILE *uptime = fopen("/proc/uptime", "r");
    double seconds = 0.0;
    if (uptime && fscanf(uptime, "%lf", &seconds) == 1) {
        int minutes = (int)(seconds / 60.0);
        snprintf(info->uptime, sizeof(info->uptime), "%dd %02dh %02dm", minutes / 1440, (minutes / 60) % 24, minutes % 60);
    } else {
        snprintf(info->uptime, sizeof(info->uptime), "unavailable");
    }
    if (uptime) fclose(uptime);

    char temp_raw[32];
    read_first_line("/sys/class/thermal/thermal_zone0/temp", temp_raw, sizeof(temp_raw), "");
    if (temp_raw[0]) {
        snprintf(info->cpu_temp, sizeof(info->cpu_temp), "%.1f C", atof(temp_raw) / 1000.0);
    } else {
        snprintf(info->cpu_temp, sizeof(info->cpu_temp), "unavailable");
    }

    FILE *meminfo = fopen("/proc/meminfo", "r");
    long total = 0;
    long available = 0;
    char key[64];
    long value = 0;
    char unit[16];
    while (meminfo && fscanf(meminfo, "%63s %ld %15s", key, &value, unit) == 3) {
        if (strcmp(key, "MemTotal:") == 0) total = value;
        if (strcmp(key, "MemAvailable:") == 0) available = value;
    }
    if (meminfo) fclose(meminfo);
    if (total > 0) {
        snprintf(info->memory, sizeof(info->memory), "%ld/%ld MB", (total - available) / 1024, total / 1024);
    } else {
        snprintf(info->memory, sizeof(info->memory), "unavailable");
    }

    struct statvfs fs;
    if (statvfs("/", &fs) == 0) {
        unsigned long total_mb = (unsigned long)((fs.f_blocks * fs.f_frsize) / (1024 * 1024));
        unsigned long free_mb = (unsigned long)((fs.f_bavail * fs.f_frsize) / (1024 * 1024));
        snprintf(info->disk, sizeof(info->disk), "%lu/%lu MB", total_mb - free_mb, total_mb);
    } else {
        snprintf(info->disk, sizeof(info->disk), "unavailable");
    }

    snprintf(info->ip_address, sizeof(info->ip_address), "see ip addr");
}

int System_ListUsb(char lines[][96], int max_lines) {
    FILE *file = fopen("/sys/kernel/debug/usb/devices", "r");
    int count = 0;
    if (file) {
        char line[160];
        while (fgets(line, sizeof(line), file) && count < max_lines) {
            if (strncmp(line, "P:", 2) == 0 || strncmp(line, "S:  Product", 11) == 0) {
                line[strcspn(line, "\r\n")] = '\0';
                snprintf(lines[count++], 96, "%s", line);
            }
        }
        fclose(file);
        return count;
    }

    /*
     * debugfs is often not mounted on a stripped-down appliance image. The
     * sysfs USB tree is still available and works for hot-plug diagnostics.
     */
    DIR *dir = opendir("/sys/bus/usb/devices");
    if (!dir) {
        snprintf(lines[count++], 96, "USB inventory unavailable");
        return count;
    }

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL && count < max_lines) {
        char product_path[192];
        char vendor_path[192];
        char product_id_path[192];
        char product[64];
        char vendor[16];
        char product_id[16];

        if (entry->d_name[0] == '.') {
            continue;
        }
        if (usb_device_is_interface(entry->d_name) || !usb_device_has_product_file(entry->d_name)) {
            continue;
        }

        snprintf(product_path, sizeof(product_path), "/sys/bus/usb/devices/%s/product", entry->d_name);
        snprintf(vendor_path, sizeof(vendor_path), "/sys/bus/usb/devices/%s/idVendor", entry->d_name);
        snprintf(product_id_path, sizeof(product_id_path), "/sys/bus/usb/devices/%s/idProduct", entry->d_name);
        read_first_line(product_path, product, sizeof(product), "");
        read_first_line(vendor_path, vendor, sizeof(vendor), "");
        read_first_line(product_id_path, product_id, sizeof(product_id), "");

        if (product[0]) {
            snprintf(lines[count++], 96, "%s %s:%s", product, vendor[0] ? vendor : "?", product_id[0] ? product_id : "?");
        }
    }
    closedir(dir);

    dir = opendir("/sys/bus/usb/devices");
    while (dir && (entry = readdir(dir)) != NULL && count < max_lines) {
        char class_path[192];
        char subclass_path[192];
        char protocol_path[192];
        char driver[48];
        char class_code[16];
        char subclass[16];
        char protocol[16];

        if (!usb_device_is_interface(entry->d_name)) {
            continue;
        }

        snprintf(class_path, sizeof(class_path), "/sys/bus/usb/devices/%s/bInterfaceClass", entry->d_name);
        snprintf(subclass_path, sizeof(subclass_path), "/sys/bus/usb/devices/%s/bInterfaceSubClass", entry->d_name);
        snprintf(protocol_path, sizeof(protocol_path), "/sys/bus/usb/devices/%s/bInterfaceProtocol", entry->d_name);
        read_first_line(class_path, class_code, sizeof(class_code), "?");
        read_first_line(subclass_path, subclass, sizeof(subclass), "?");
        read_first_line(protocol_path, protocol, sizeof(protocol), "?");
        read_driver_name(entry->d_name, driver, sizeof(driver));

        snprintf(lines[count++], 96, "if %s class=%s/%s/%s driver=%s", entry->d_name, class_code, subclass, protocol, driver);
    }
    if (dir) {
        closedir(dir);
    }

    if (count == 0) {
        snprintf(lines[count++], 96, "No USB products listed in sysfs");
    }
    return count;
}

bool System_FindUsbId(const char *vendor_id, const char *product_id, char *label, size_t label_size) {
    DIR *dir = opendir("/sys/bus/usb/devices");
    if (!dir) {
        return false;
    }

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        char vendor_path[192];
        char product_id_path[192];
        char product_path[192];
        char vendor[16];
        char product[16];
        char name[64];

        if (entry->d_name[0] == '.' || usb_device_is_interface(entry->d_name)) {
            continue;
        }

        snprintf(vendor_path, sizeof(vendor_path), "/sys/bus/usb/devices/%s/idVendor", entry->d_name);
        snprintf(product_id_path, sizeof(product_id_path), "/sys/bus/usb/devices/%s/idProduct", entry->d_name);
        snprintf(product_path, sizeof(product_path), "/sys/bus/usb/devices/%s/product", entry->d_name);
        read_first_line(vendor_path, vendor, sizeof(vendor), "");
        read_first_line(product_id_path, product, sizeof(product), "");

        if (strcmp(vendor, vendor_id) == 0 && strcmp(product, product_id) == 0) {
            read_first_line(product_path, name, sizeof(name), "USB device");
            snprintf(label, label_size, "%s %s:%s", name, vendor, product);
            closedir(dir);
            return true;
        }
    }

    closedir(dir);
    return false;
}
