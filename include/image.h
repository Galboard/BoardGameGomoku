// include/image.h
#pragma once

#include <cstdint>
#include <string>
#include "../include/lcd.h"

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

class BmpImage {
public:
    explicit BmpImage(const std::string& file_path);
    ~BmpImage();

    BmpImage(const BmpImage&) = delete;
    BmpImage& operator=(const BmpImage&) = delete;

    int get_width() const { return info_header_.width; }
    int get_height() const { return std::abs(info_header_.height); }
    uint16_t get_bit_depth() const { return info_header_.bit_count; }

    // 绘制时不绑定特定的 LCD 对象，通过参数传入解耦
    void draw_to_lcd(Lcd& screen, int start_x = 0, int start_y = 0);

private:
    BmpFileHeader file_header_;
    BmpInfoHeader info_header_;
    
    unsigned char* pixel_data_; 
    size_t data_size_;
    int row_bytes_;
};