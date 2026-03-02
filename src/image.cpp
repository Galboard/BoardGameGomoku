// src/image.cpp
#include "../include/image.h"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <cmath> 

// 将 BMP 结构体隐藏在 cpp 文件中，不对外暴露
#pragma pack(push, 1)
struct BmpFileHeader {
    uint16_t file_type;      
    uint32_t file_size;      
    uint16_t reserved1;      
    uint16_t reserved2;      
    uint32_t offset_data;    
};

struct BmpInfoHeader {
    uint32_t size;           
    int32_t  width;          
    int32_t  height;         
    uint16_t planes;         
    uint16_t bit_count;      
    uint32_t compression;    
    uint32_t size_image;     
    int32_t  x_pixels_per_meter; 
    int32_t  y_pixels_per_meter; 
    uint32_t colors_used;    
    uint32_t colors_important; 
};
#pragma pack(pop)

// --- Image 类的实现 ---

Image::Image(const std::string& file_path) 
    : pixel_data_(nullptr), data_size_(0), row_bytes_(0) {
    
    int bmp_fd = open(file_path.c_str(), O_RDONLY);
    if (bmp_fd < 0) {
        throw std::runtime_error("Image open failed [" + file_path + "]: " + std::string(strerror(errno)));
    }
    
    BmpFileHeader file_header;
    if (read(bmp_fd, &file_header, sizeof(BmpFileHeader)) != sizeof(BmpFileHeader)) {
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP file header");
    }

    if (file_header.file_type != 0x4D42) { 
         close(bmp_fd);
         throw std::runtime_error("Not a valid BMP file");
    }

    BmpInfoHeader info_header;
    if (read(bmp_fd, &info_header, sizeof(BmpInfoHeader)) != sizeof(BmpInfoHeader)) {
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP info header");
    }

    if (info_header.bit_count != 24 && info_header.bit_count != 32) {
        close(bmp_fd);
        throw std::runtime_error("Only 24-bit and 32-bit BMPs are supported");
    }

    if (lseek(bmp_fd, file_header.offset_data, SEEK_SET) == (off_t)-1) {
        close(bmp_fd);
        throw std::runtime_error("Failed to seek to pixel data");
    }

    // 提取我们需要的高频数据，丢弃沉重的 header 结构
    width_ = info_header.width;
    height_ = std::abs(info_header.height);
    bit_count_ = info_header.bit_count;
    is_bottom_up_ = (info_header.height > 0);

    row_bytes_ = (width_ * bit_count_ + 31) / 32 * 4;
    
    data_size_ = info_header.size_image;
    if (data_size_ == 0) { 
        data_size_ = row_bytes_ * height_;
    }

    pixel_data_ = new unsigned char[data_size_];

    if (read(bmp_fd, pixel_data_, data_size_) != data_size_) {
        delete[] pixel_data_;
        close(bmp_fd);
        throw std::runtime_error("Failed to read BMP pixel data");
    }

    close(bmp_fd);
}

Image::~Image() {
    if (pixel_data_ != nullptr) {
        delete[] pixel_data_;
    }
}

void Image::draw(Lcd& screen, int start_x, int start_y) {
    int bytes_per_pixel = bit_count_ / 8;

    for (int y = 0; y < height_; ++y) {
        int memory_y = is_bottom_up_ ? (height_ - 1 - y) : y;
        
        for (int x = 0; x < width_; ++x) {
            int offset = memory_y * row_bytes_ + x * bytes_per_pixel;
            uint32_t color = 0;

            if (bytes_per_pixel == 3) {
                uint8_t b = pixel_data_[offset];
                uint8_t g = pixel_data_[offset + 1];
                uint8_t r = pixel_data_[offset + 2];
                color = (r << 16) | (g << 8) | b;
            } else if (bytes_per_pixel == 4) {
                uint8_t b = pixel_data_[offset];
                uint8_t g = pixel_data_[offset + 1];
                uint8_t r = pixel_data_[offset + 2];
                uint8_t a = pixel_data_[offset + 3];
                
                // 【透明抠图核心】：如果 Alpha 通道为 0 (完全透明)，则跳过绘制该像素。
                // 这样当你把一个带透明背景的圆形黑棋画在棋盘上时，就不会有一个难看的正方形白框！
                if (a == 0) continue; 

                color = (a << 24) | (r << 16) | (g << 8) | b;
            }

            screen.render_pixel(start_x + x, start_y + y, color);
        }
    }
}

// --- ImageManager 类的实现 ---

ImageManager& ImageManager::get_instance() {
    static ImageManager instance;
    return instance;
}

ImageManager::~ImageManager() {
    clear_cache();
}

void ImageManager::preload(const std::string& file_path) {
    get_image(file_path);
}

Image* ImageManager::get_image(const std::string& file_path) {
    // 使用 C++17 特有的带初始化的 if 语句，逻辑紧凑，作用域安全
    if (auto it = image_cache_.find(file_path); it != image_cache_.end()) {
        return it->second; // 命中缓存，瞬间返回！不会再读硬盘。
    }

    // 没命中缓存，才去解析硬盘文件，并加入哈希字典
    Image* new_image = new Image(file_path);
    image_cache_[file_path] = new_image;
    return new_image;
}

void ImageManager::draw_image(Lcd& screen, const std::string& file_path, int start_x, int start_y) {
    if (Image* img = get_image(file_path)) {
        img->draw(screen, start_x, start_y);
    }
}

void ImageManager::clear_cache() {
    for (auto& pair : image_cache_) {
        delete pair.second;
    }
    image_cache_.clear();
}