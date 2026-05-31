#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct Image {
    int width;
    int height;
    uint8_t *pixels;
} Image;

static void skip_ws_and_comments(FILE *file) {
    int c = 0;
    do {
        c = fgetc(file);
        if (c == '#') {
            while (c != '\n' && c != EOF) {
                c = fgetc(file);
            }
        }
    } while (c == ' ' || c == '\n' || c == '\r' || c == '\t');

    if (c != EOF) {
        ungetc(c, file);
    }
}

static int read_number(FILE *file) {
    int value = 0;
    skip_ws_and_comments(file);
    if (fscanf(file, "%d", &value) != 1) {
        return -1;
    }
    return value;
}

static int load_ppm(const char *path, Image *image) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror(path);
        return -1;
    }

    char magic[3] = {0};
    if (fread(magic, 1, 2, file) != 2 || strcmp(magic, "P6") != 0) {
        fprintf(stderr, "%s: expected raw P6 PPM\n", path);
        fclose(file);
        return -1;
    }

    image->width = read_number(file);
    image->height = read_number(file);
    int max_value = read_number(file);
    if (image->width <= 0 || image->height <= 0 || max_value != 255) {
        fprintf(stderr, "%s: unsupported PPM dimensions or max value\n", path);
        fclose(file);
        return -1;
    }

    fgetc(file);
    size_t pixel_bytes = (size_t)image->width * (size_t)image->height * 3u;
    image->pixels = malloc(pixel_bytes);
    if (!image->pixels) {
        fclose(file);
        return -1;
    }

    if (fread(image->pixels, 1, pixel_bytes, file) != pixel_bytes) {
        fprintf(stderr, "%s: truncated PPM data\n", path);
        free(image->pixels);
        image->pixels = NULL;
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

static uint32_t scale_channel(uint8_t value, uint32_t length, uint32_t offset) {
    if (length == 0) {
        return 0;
    }
    return ((uint32_t)value * ((1u << length) - 1u) / 255u) << offset;
}

static void write_pixel(uint8_t *dst, const struct fb_var_screeninfo *var, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t pixel = 0;
    pixel |= scale_channel(r, var->red.length, var->red.offset);
    pixel |= scale_channel(g, var->green.length, var->green.offset);
    pixel |= scale_channel(b, var->blue.length, var->blue.offset);
    if (var->transp.length > 0) {
        pixel |= ((1u << var->transp.length) - 1u) << var->transp.offset;
    }

    switch (var->bits_per_pixel) {
    case 16:
        {
            uint16_t packed = (uint16_t)pixel;
            memcpy(dst, &packed, sizeof(packed));
        }
        break;
    case 24:
        dst[0] = (uint8_t)(pixel & 0xffu);
        dst[1] = (uint8_t)((pixel >> 8) & 0xffu);
        dst[2] = (uint8_t)((pixel >> 16) & 0xffu);
        break;
    case 32:
        memcpy(dst, &pixel, sizeof(pixel));
        break;
    }
}

int main(int argc, char **argv) {
    const char *image_path = argc > 1 ? argv[1] : "/etc/cyberdeck/bootsplash.ppm";
    Image image = {0};
    if (load_ppm(image_path, &image) != 0) {
        return 1;
    }

    int fb = open("/dev/fb0", O_RDWR);
    if (fb < 0) {
        perror("/dev/fb0");
        free(image.pixels);
        return 1;
    }

    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &var) != 0 || ioctl(fb, FBIOGET_FSCREENINFO, &fix) != 0) {
        perror("FBIOGET_*SCREENINFO");
        close(fb);
        free(image.pixels);
        return 1;
    }

    if (var.bits_per_pixel != 16 && var.bits_per_pixel != 24 && var.bits_per_pixel != 32) {
        fprintf(stderr, "unsupported framebuffer depth: %u\n", var.bits_per_pixel);
        close(fb);
        free(image.pixels);
        return 1;
    }

    size_t fb_size = (size_t)fix.line_length * (size_t)var.yres_virtual;
    uint8_t *fb_mem = mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fb_mem == MAP_FAILED) {
        perror("mmap");
        close(fb);
        free(image.pixels);
        return 1;
    }

    memset(fb_mem, 0, fb_size);

    int x_offset = ((int)var.xres - image.width) / 2;
    int y_offset = ((int)var.yres - image.height) / 2;
    if (x_offset < 0) x_offset = 0;
    if (y_offset < 0) y_offset = 0;

    int draw_width = image.width < (int)var.xres ? image.width : (int)var.xres;
    int draw_height = image.height < (int)var.yres ? image.height : (int)var.yres;
    int bytes_per_pixel = (int)var.bits_per_pixel / 8;

    for (int y = 0; y < draw_height; y++) {
        for (int x = 0; x < draw_width; x++) {
            uint8_t *src = &image.pixels[((size_t)y * (size_t)image.width + (size_t)x) * 3u];
            uint8_t *dst = fb_mem + ((size_t)(y + y_offset) * (size_t)fix.line_length) +
                           ((size_t)(x + x_offset) * (size_t)bytes_per_pixel);
            write_pixel(dst, &var, src[0], src[1], src[2]);
        }
    }

    munmap(fb_mem, fb_size);
    close(fb);
    free(image.pixels);
    return 0;
}
