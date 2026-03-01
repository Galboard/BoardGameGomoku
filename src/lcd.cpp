// src/lcd.cpp
#include "../include/lcd.h"

#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>  // 新增：用于 ioctl 系统调用
#include <linux/fb.h>   // 新增：包含 Framebuffer 的结构体定义
#include <iostream>

Lcd::Lcd(const std::string& dev_path)
    : dev_fd_(-1), lptr_(nullptr), back_lptr_(nullptr), 
      screen_width_(800), screen_height_(480) { // 默认值垫底，防查询失败
      
    if ((dev_fd_ = open(dev_path.c_str(), O_RDWR)) < 0) {
        throw std::runtime_error("Lcd open failed: " + std::string(strerror(errno)));
    }
    
    // 【核心黑魔法】：向内核查询真实的 LCD 屏幕信息
    struct fb_var_screeninfo vinfo;
    if (ioctl(dev_fd_, FBIOGET_VSCREENINFO, &vinfo) == 0) {
        screen_width_ = vinfo.xres;   // 获取真实物理宽度
        screen_height_ = vinfo.yres;  // 获取真实物理高度
        std::cout << "[Info] LCD real resolution: " 
                  << screen_width_ << " x " << screen_height_ 
                  << " (" << vinfo.bits_per_pixel << "bpp)" << std::endl;
    } else {
        std::cerr << "[Warn] Could not get LCD info, using default 800x480" << std::endl;
    }

    // 内存大小现在是动态计算的
    size_t mem_size = screen_width_ * screen_height_ * sizeof(int);
    lptr_ = (int*)mmap(nullptr, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd_, 0);
    if (lptr_ == MAP_FAILED) {
        close(dev_fd_); 
        throw std::runtime_error("Lcd mmap failed: " + std::string(strerror(errno)));
    }
    
    // 双缓冲内存也动态分配
    back_lptr_ = new int[screen_width_ * screen_height_];
}

Lcd::~Lcd() {
    if (lptr_ && lptr_ != MAP_FAILED) {
        // 释放时同样依赖动态计算的大小
        munmap(lptr_, screen_width_ * screen_height_ * sizeof(int));
    }
    if (dev_fd_ >= 0) {
        close(dev_fd_);
    }
    delete[] back_lptr_;
}

void Lcd::render_pixel(int x, int y, uint32_t color) {
    // 越界保护也自动适配了当前真实的屏幕尺寸
    if (x >= 0 && x < screen_width_ && y >= 0 && y < screen_height_) {
        *(back_lptr_ + screen_width_ * y + x) = color;
    }
}

void Lcd::show() {
    memcpy(lptr_, back_lptr_, screen_width_ * screen_height_ * sizeof(int));
}

void Lcd::render_rectangle(int width, int height, int x, int y, uint32_t color) {
    for (int iy = y; iy < y + height; ++iy) {
        for (int ix = x; ix < x + width; ++ix) {
            render_pixel(ix, iy, color);
        }
    }
}

// 补齐 render_circle 的实现
void Lcd::render_circle(int radius, int x, int y, uint32_t color) {
    for (int iy = y - radius; iy < y + radius; ++iy) {
        for (int ix = x - radius; ix < x + radius; ++ix) {
            if ((ix - x) * (ix - x) + (iy - y) * (iy - y) <= (radius * radius)) {
                render_pixel(ix, iy, color);
            }
        }
    }
}

void Lcd::clear(uint32_t color) {
    // 自动根据真实分辨率刷屏
    for (int y = 0; y < screen_height_; ++y) {
        for (int x = 0; x < screen_width_; ++x) {
            render_pixel(x, y, color);
        }
    }
}

Lcd& Lcd::get_instance(const std::string& dev_path) {
    static Lcd instance(dev_path);
    return instance;
}