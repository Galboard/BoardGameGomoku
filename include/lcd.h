// include/lcd.h
#pragma once

#include <cstdint>
#include <string>

constexpr char kDefaultLcdPath[] = "/dev/fb0";

class Lcd {
public:
    static Lcd& get_instance(const std::string& dev_path = kDefaultLcdPath);
    
    // 禁用拷贝与赋值
    Lcd(const Lcd&) = delete;
    Lcd& operator=(const Lcd&) = delete;
    ~Lcd();

    void render_pixel(int x, int y, uint32_t color);
    void show();
    void render_circle(int radius, int x, int y, uint32_t color);
    void render_rectangle(int width, int height, int x, int y, uint32_t color);
    void clear(uint32_t color);

    // 新增：对外提供获取屏幕真实分辨率的接口
    int get_width() const { return screen_width_; }
    int get_height() const { return screen_height_; }

private:
    explicit Lcd(const std::string& dev_path);
    
    int dev_fd_;
    int* lptr_;
    int* back_lptr_;

    // 新增：用于动态保存底层的真实物理分辨率
    int screen_width_;
    int screen_height_;
};