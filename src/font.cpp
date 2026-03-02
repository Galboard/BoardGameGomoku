// src/font.cpp
#include "../include/font.h"
#include <fstream>
#include <stdexcept>

// ！！！必须只在一个 cpp 文件中定义这个宏，它会把实现代码展开！！！
#define STB_TRUETYPE_IMPLEMENTATION
#include "../include/stb_truetype.h"

Font::Font(const std::string& ttf_path, float font_size) : font_size_(font_size) {
    // 1. 以二进制模式读取整个 TTF 文件到内存
    std::ifstream file(ttf_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open font file: " + ttf_path);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    font_buffer_.resize(size);
    if (!file.read(reinterpret_cast<char*>(font_buffer_.data()), size)) {
        throw std::runtime_error("Failed to read font data");
    }

    // 2. 初始化 stb_truetype
    if (!stbtt_InitFont(&font_info_, font_buffer_.data(), 0)) {
        throw std::runtime_error("Failed to initialize stb_truetype font info");
    }

    // 3. 计算缩放比例和垂直排版参数
    scale_ = stbtt_ScaleForPixelHeight(&font_info_, font_size_);
    stbtt_GetFontVMetrics(&font_info_, &ascent_, &descent_, &line_gap_);
    ascent_ = ascent_ * scale_;
    descent_ = descent_ * scale_;
}

// 轻量级 UTF-8 解析器：把中文字符（3字节）转换成 Unicode ID
int Font::decode_utf8(const std::string& text, size_t& i) {
    unsigned char c = text[i];
    if (c <= 0x7F) { // ASCII
        i += 1; return c;
    } else if ((c & 0xE0) == 0xC0) { // 2 字节字符
        int cp = ((c & 0x1F) << 6) | (text[i+1] & 0x3F);
        i += 2; return cp;
    } else if ((c & 0xF0) == 0xE0) { // 3 字节字符 (绝大部分中文)
        int cp = ((c & 0x0F) << 12) | ((text[i+1] & 0x3F) << 6) | (text[i+2] & 0x3F);
        i += 3; return cp;
    } else if ((c & 0xF8) == 0xF0) { // 4 字节字符 (Emoji等)
        int cp = ((c & 0x07) << 18) | ((text[i+1] & 0x3F) << 12) | ((text[i+2] & 0x3F) << 6) | (text[i+3] & 0x3F);
        i += 4; return cp;
    }
    i += 1; return '?'; // 无法识别的乱码
}

void Font::draw_text(Lcd& screen, const std::string& text, int start_x, int start_y, uint32_t color) {
    // 拆解目标颜色 (0xAARRGGBB)
    uint8_t fg_r = (color >> 16) & 0xFF;
    uint8_t fg_g = (color >> 8) & 0xFF;
    uint8_t fg_b = color & 0xFF;

    int current_x = start_x;
    // start_y 是文字整体的左上角，但在排版学里画字要基于“基线 (Baseline)”
    int baseline = start_y + ascent_;

    size_t i = 0;
    while (i < text.length()) {
        int codepoint = decode_utf8(text, i);

        // 获取字形的尺寸和偏移
        int advance_width, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font_info_, codepoint, &advance_width, &left_side_bearing);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font_info_, codepoint, scale_, scale_, &x0, &y0, &x1, &y1);

        // 渲染成单通道(灰度)的 Alpha 遮罩
        int w = x1 - x0;
        int h = y1 - y0;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font_info_, scale_, scale_, codepoint, &w, &h, nullptr, nullptr);

        if (bitmap) {
            // 将点阵绘制到屏幕
            for (int r = 0; r < h; ++r) {
                for (int c = 0; c < w; ++c) {
                    int alpha = bitmap[r * w + c];
                    if (alpha == 0) continue; // 完全透明，跳过

                    int draw_x = current_x + x0 + c;
                    int draw_y = baseline + y0 + r;

                    // 获取屏幕上该点原本的颜色 (背景色)
                    uint32_t bg_color = screen.get_pixel(draw_x, draw_y);
                    uint8_t bg_r = (bg_color >> 16) & 0xFF;
                    uint8_t bg_g = (bg_color >> 8) & 0xFF;
                    uint8_t bg_b = bg_color & 0xFF;

                    // 【核心算法】：Alpha 混合！实现极其平滑的字体边缘
                    uint8_t out_r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
                    uint8_t out_g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
                    uint8_t out_b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;

                    screen.render_pixel(draw_x, draw_y, (out_r << 16) | (out_g << 8) | out_b);
                }
            }
            stbtt_FreeBitmap(bitmap, nullptr); // 必须释放，否则内存泄漏
        }

        // 游标向右移动，准备画下一个字
        current_x += (advance_width * scale_);
    }
}