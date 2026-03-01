#include "../include/image.h"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <cmath> // 用于 std::abs

Bmp::Bmp(const std::string& file_path, lcd& lcd_screen) 
    : pixel_data_(nullptr), screen_(lcd_screen) {
    
    // bmp_fd_ 作为局部变量即可
    int bmp_fd = open(file_path.c_str(), O_RDONLY);
    if (bmp_fd < 0) {
        throw std::runtime_error("Open failed: " + std::string(strerror(errno)));
    }
    
    if (read(bmp_fd, &file_header_, sizeof(BmpFileHeader)) != sizeof(BmpFileHeader)) {
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP file header");
    }

    if (file_header_.file_type != 0x4D42) {
         close(bmp_fd);
         throw std::runtime_error("Not a valid BMP file: Magic number mismatch");
    }

    if (read(bmp_fd, &info_header_, sizeof(BmpInfoHeader)) != sizeof(BmpInfoHeader)) {
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP info header");
    }

    if (lseek(bmp_fd, file_header_.offset_data, SEEK_SET) == (off_t)-1) {
        close(bmp_fd);
        throw std::runtime_error("Failed to seek to pixel data");
    }

    // 提前计算好每行的真实字节数并存入成员变量，渲染时直接用
    row_bytes_ = (info_header_.width * info_header_.bit_count + 31) / 32 * 4;
    
    data_size_ = info_header_.size_image;
    if (data_size_ == 0) {
        data_size_ = row_bytes_ * std::abs(info_header_.height);
    }

    pixel_data_ = new unsigned char[data_size_];

    if (read(bmp_fd, pixel_data_, data_size_) != data_size_) {
        delete[] pixel_data_;
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP pixel data");
    }

    close(bmp_fd);
}

Bmp::~Bmp() {
    if (pixel_data_ != nullptr) {
        delete[] pixel_data_;
    }
}

void Bmp::draw_to_lcd(int start_x, int start_y) {
    if (info_header_.bit_count != 24 && info_header_.bit_count != 32) {
        throw std::runtime_error("Only 24-bit and 32-bit BMPs are supported");
    }

    int width = info_header_.width;
    int height = std::abs(info_header_.height);
    int bytes_per_pixel = info_header_.bit_count / 8;
    
    // BMP 规范：高度为正数时，像素数据是从下往上倒着存的
    bool is_bottom_up = (info_header_.height > 0);

    for (int y = 0; y < height; ++y) {
        // 如果是倒置的，我们需要算出当前屏幕上的 y 对应内存里的哪一行
        int memory_y = is_bottom_up ? (height - 1 - y) : y;
        
        for (int x = 0; x < width; ++x) {
            // 核心推导：找到单个像素在内存大数组中的精确起始位置
            int offset = memory_y * row_bytes_ + x * bytes_per_pixel;
            uint32_t color = 0;

            if (bytes_per_pixel == 3) {
                // 24位 BMP 在内存中是 B, G, R 连续排列
                uint8_t b = pixel_data_[offset];
                uint8_t g = pixel_data_[offset + 1];
                uint8_t r = pixel_data_[offset + 2];
                // 组装成 0x00RRGGBB 发给 LCD
                color = (r << 16) | (g << 8) | b;
            } else if (bytes_per_pixel == 4) {
                // 32位 BMP 通常是 B, G, R, A
                uint8_t b = pixel_data_[offset];
                uint8_t g = pixel_data_[offset + 1];
                uint8_t r = pixel_data_[offset + 2];
                // 忽略 Alpha 通道，或者将其放在最高位
                color = (r << 16) | (g << 8) | b;
            }

            // 绘制到屏幕对应的坐标上
            screen_.render_pixel(start_x + x, start_y + y, color);
        }
    }
}