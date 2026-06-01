#define _DEFAULT_SOURCE
#include "hal/system.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        char product[64];
        char vendor[16];

        if (entry->d_name[0] == '.') {
            continue;
        }

        snprintf(product_path, sizeof(product_path), "/sys/bus/usb/devices/%s/product", entry->d_name);
        snprintf(vendor_path, sizeof(vendor_path), "/sys/bus/usb/devices/%s/idVendor", entry->d_name);
        read_first_line(product_path, product, sizeof(product), "");
        read_first_line(vendor_path, vendor, sizeof(vendor), "");

        if (product[0]) {
            snprintf(lines[count++], 96, "%s vendor=%s", product, vendor[0] ? vendor : "?");
        }
    }
    closedir(dir);

    if (count == 0) {
        snprintf(lines[count++], 96, "No USB products listed in sysfs");
    }
    return count;
}
