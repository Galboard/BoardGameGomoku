#include "../include/image.h"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <cmath> // 用于 std::abs

// 构造函数：负责分配资源、读取文件、并且执行 Fail-Fast 校验
BmpImage::BmpImage(const std::string& file_path) 
    : pixel_data_(nullptr), data_size_(0), row_bytes_(0) {
    
    // bmp_fd 变成局部变量，用完即焚，不占用类的成员空间
    int bmp_fd = open(file_path.c_str(), O_RDONLY);
    if (bmp_fd < 0) {
        throw std::runtime_error("BmpImage open failed: " + std::string(strerror(errno)));
    }
    
    // 1. 读取并校验文件头
    if (read(bmp_fd, &file_header_, sizeof(BmpFileHeader)) != sizeof(BmpFileHeader)) {
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP file header");
    }

    if (file_header_.file_type != 0x4D42) { // 0x4D42 即 'BM'
         close(bmp_fd);
         throw std::runtime_error("Not a valid BMP file: Magic number mismatch");
    }

    // 2. 读取信息头
    if (read(bmp_fd, &info_header_, sizeof(BmpInfoHeader)) != sizeof(BmpInfoHeader)) {
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP info header");
    }

    // 【关键优化：Fail-Fast 机制】
    // 在去读取可能高达几 MB 的像素数据之前，先检查格式是否支持。
    // 如果不支持直接抛出异常，避免浪费内存和 CPU 算力。
    if (info_header_.bit_count != 24 && info_header_.bit_count != 32) {
        close(bmp_fd);
        throw std::runtime_error("Only 24-bit and 32-bit BMPs are supported");
    }

    // 3. 跳转到真实的像素数据区（跳过可能存在的调色板等废数据）
    if (lseek(bmp_fd, file_header_.offset_data, SEEK_SET) == (off_t)-1) {
        close(bmp_fd);
        throw std::runtime_error("Failed to seek to pixel data");
    }

    // 4. 计算包含对齐填充在内的真实行字节数
    row_bytes_ = (info_header_.width * info_header_.bit_count + 31) / 32 * 4;
    
    data_size_ = info_header_.size_image;
    if (data_size_ == 0) { // 应对某些作坊画图软件不规范填 0 的情况
        data_size_ = row_bytes_ * std::abs(info_header_.height);
    }

    // 5. 分配内存并读取纯像素数据
    pixel_data_ = new unsigned char[data_size_];

    if (read(bmp_fd, pixel_data_, data_size_) != data_size_) {
        delete[] pixel_data_;
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP pixel data");
    }

    // 数据已安全驻留内存，立刻关闭文件描述符，释放系统资源
    close(bmp_fd);
}

// 析构函数：由于禁用了拷贝构造，这里可以安全地释放内存，不用担心 Double Free
BmpImage::~BmpImage() {
    if (pixel_data_ != nullptr) {
        delete[] pixel_data_;
    }
}

// 绘制函数：不再与特定的 Lcd 实例强绑定，而是通过参数注入 (Dependency Injection)
void BmpImage::draw_to_lcd(Lcd& screen, int start_x, int start_y) {
    int width = info_header_.width;
    int height = std::abs(info_header_.height);
    int bytes_per_pixel = info_header_.bit_count / 8;
    
    // BMP 规范：高度为正数时，文件中的像素行是从底向上的
    bool is_bottom_up = (info_header_.height > 0);

    for (int y = 0; y < height; ++y) {
        // 算出当前屏幕的 y 对应在内存大一维数组里的哪一行
        int memory_y = is_bottom_up ? (height - 1 - y) : y;
        
        for (int x = 0; x < width; ++x) {
            // 核心推导：精准跳过对齐填充字节，定位到单个像素的起始偏移量
            int offset = memory_y * row_bytes_ + x * bytes_per_pixel;
            uint32_t color = 0;

            if (bytes_per_pixel == 3) {
                // 24位 BMP 在内存中是 B, G, R 连续排列
                uint8_t b = pixel_data_[offset];
                uint8_t g = pixel_data_[offset + 1];
                uint8_t r = pixel_data_[offset + 2];
                // 组装成 0x00RRGGBB 格式
                color = (r << 16) | (g << 8) | b;
            } else if (bytes_per_pixel == 4) {
                // 32位 BMP 通常带有 Alpha 通道 (B, G, R, A)
                uint8_t b = pixel_data_[offset];
                uint8_t g = pixel_data_[offset + 1];
                uint8_t r = pixel_data_[offset + 2];
                uint8_t a = pixel_data_[offset + 3];
                // 将透明度放在最高 8 位：0xAARRGGBB
                color = (a << 24) | (r << 16) | (g << 8) | b;
            }

            // 直接调用屏幕的渲染接口。
            // 因为在 lcd.cpp 中已经加上了越界保护，所以哪怕图片超出了屏幕范围也不会导致系统崩溃！
            screen.render_pixel(start_x + x, start_y + y, color);
        }
    }
}