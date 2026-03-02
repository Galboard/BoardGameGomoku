// include/ui.h
#pragma once

#include <string>
#include <functional>
#include "../include/lcd.h"
#include "../include/event.h"
#include "../include/image.h"

// 前向声明，告诉编译器存在这个类，避免此时强依赖 font.h
class Font; 

class Button {
public:
    // 构造函数：初始化坐标、尺寸和文本
    Button(int x, int y, int width, int height, const std::string& text = "");

    // --- 样式设置接口 ---
    void set_bg_image(const std::string& bmp_path);
    void set_bg_color(uint32_t color);
    void set_text_color(uint32_t color);

    // --- 核心交互接口 ---
    // 绑定点击发生时要执行的代码块 (Callback)
    void set_on_click(std::function<void()> callback);

    // 检查触摸点是否落在本按钮内。如果是，自动触发 callback 并返回 true
    bool check_click(const TouchPoint& pt);

    // --- 渲染接口 ---
    // 传入 Font 指针，如果传 nullptr 则只画背景不画文字（兼容纯图片按钮）
    void draw(Lcd& screen, Font* font = nullptr);

private:
    int x_, y_;
    int width_, height_;
    std::string text_;
    
    std::string bg_image_path_;
    uint32_t bg_color_;     // 无背景图时的纯色兜底
    uint32_t text_color_;

    // 存储回调函数的“容器”
    std::function<void()> on_click_callback_;
};