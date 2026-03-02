// src/ui.cpp
#include "../include/ui.h"
// #include "../include/font.h" // 下一步实现 font 时再解开注释

Button::Button(int x, int y, int width, int height, const std::string& text)
    : x_(x), y_(y), width_(width), height_(height), text_(text),
      bg_color_(0x00A0A0A0),   // 默认给一个灰色背景
      text_color_(0x00000000), // 默认黑色文字
      on_click_callback_(nullptr) 
{}

void Button::set_bg_image(const std::string& bmp_path) {
    bg_image_path_ = bmp_path;
}

void Button::set_bg_color(uint32_t color) {
    bg_color_ = color;
}

void Button::set_text_color(uint32_t color) {
    text_color_ = color;
}

void Button::set_on_click(std::function<void()> callback) {
    on_click_callback_ = callback;
}

// 核心：AABB (Axis-Aligned Bounding Box) 碰撞检测
bool Button::check_click(const TouchPoint& pt) {
    // 1. 判断触摸点是否落在按钮的矩形范围内
    if (pt.x >= x_ && pt.x <= x_ + width_ &&
        pt.y >= y_ && pt.y <= y_ + height_) {
        
        // 2. 如果落在范围内，且绑定了回调函数，就执行它
        if (on_click_callback_) {
            on_click_callback_();
        }
        return true; // 报告上层：这点击事件被我拦截处理了
    }
    return false;
}

void Button::draw(Lcd& screen, Font* font) {
    // 1. 绘制背景层
    if (!bg_image_path_.empty()) {
        // 优先使用我们刚写的 ImageManager 绘制贴图
        ImageManager::get_instance().draw_image(screen, bg_image_path_, x_, y_);
    } else {
        // 如果没有贴图，就画一个带颜色的矩形兜底
        screen.render_rectangle(width_, height_, x_, y_, bg_color_);
    }

    // 2. 绘制文字层
    if (!text_.empty() && font != nullptr) {
        // 这里预留给下一步的 Font 模块
        // 理想情况是让 Font 模块计算文字的宽高，从而实现绝对居中
        // 临时伪代码演示用法：
        // font->draw_text(screen, text_, x_ + 10, y_ + height_ / 2, text_color_);
    }
}