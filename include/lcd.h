#pragma once

#include <cstdint>
#include <string>

#define DEFAULT_DEVPATH "/dev/fb0"

class lcd {
public:
    static lcd& get_instance(const std::string& dev_path = DEFAULT_DEVPATH);
    lcd(const lcd&) = delete;
    lcd& operator=(const lcd&) = delete;
    ~lcd();
    void render_pixel(int x, int y, uint32_t color);
    void show();
    void render_circle(int radius, int x, int y, uint32_t color);
    void render_rectangle(int width, int height, int x, int y, uint32_t color);
    void clear(uint32_t color);
private:
    explicit lcd(const std::string& dev_path);
    int dev_fd;
    int* lptr;
    int* back_lptr;
};