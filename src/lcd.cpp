#include "../include/lcd.h"

#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

lcd::lcd(const std::string& dev_path)
    : dev_fd(-1), lptr(nullptr), back_lptr(nullptr) {
        if ((dev_fd = open(dev_path.c_str(), O_RDWR)) < 0) {
            throw std::runtime_error("Open failed: " + std::string(strerror(errno)));
        }
        lptr = (int*)mmap(nullptr, 800 * 480 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, 0);
        if (lptr == MAP_FAILED) {
            throw std::runtime_error("Mmap failed: " + std::string(strerror(errno)));
        }
        back_lptr = new int[800 * 480];
    }

lcd::~lcd() {
    if (dev_fd > 0) {
        close(dev_fd);
    }
    if (lptr && lptr != MAP_FAILED) {
        munmap(lptr, 800 * 480 * sizeof(int));
    }
    delete[] back_lptr;
}

void lcd::render_pixel(int x, int y, uint32_t color) {
    *(back_lptr + 800 * y + x) = color;
}

void lcd::show() {
    memcpy(lptr, back_lptr, 800 * 480 * sizeof(int));
}

void lcd::render_rectangle(int width, int height, int x, int y, uint32_t color) {
    for (int iy = y; iy < y + height; ++y) {
        for (int ix = x; ix < x + width; ++x) {
            if (ix >= 0 && ix < 800 && iy >= 0 && iy < 480) {
                render_pixel(x, y, color);
            }
        }
    }
}