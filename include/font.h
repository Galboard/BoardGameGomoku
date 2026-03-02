// include/font.h
#pragma once

#include <string>
#include <vector>
#include "../include/lcd.h"
#include "stb_truetype.h" // 刚刚下载的神器

class Font {
public:
    // 初始化字体，传入 ttf 文件路径和需要的字号大小 (像素)
    explicit Font(const std::string& ttf_path, float font_size);
    ~Font() = default;

    // 禁用拷贝
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // 核心绘制功能：支持 UTF-8 编码的中文！
    // start_x, start_y 为文字左上角的起始坐标
    void draw_text(Lcd& screen, const std::string& text, int start_x, int start_y, uint32_t color);

private:
    std::vector<unsigned char> font_buffer_; // 用于在内存中保存整个 ttf 文件
    stbtt_fontinfo font_info_;               // stb 内部数据结构
    float font_size_;
    float scale_;
    int ascent_;
    int descent_;
    int line_gap_;

    // 内部工具：从 UTF-8 字符串中解析出一个完整的 Unicode 码位
    int decode_utf8(const std::string& text, size_t& offset);
};