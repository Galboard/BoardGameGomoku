#pragma once

#include <cstdint>
#include <string>

#define DEFAULT_DEVPATH "/dev/fb0"

class lcd {
public:
    explicit lcd(const std::string& dev_path);
    lcd(const lcd&) = delete;
    lcd& operator=(const lcd&) = delete;
    ~lcd();
    void render_pixel(int x, int y, uint32_t color);
    void show();
    void render_rectangle(int width, int height, int x, int y, uint32_t color);
private:
    int dev_fd;
    int* lptr;
    int* back_lptr;
};