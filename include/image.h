#pragma once

#include <cstdint>
#include "../include/lcd.h"

// 1. 修正编译指令：使用 pack(push, 1) 强制按 1 字节对齐
#pragma pack(push, 1)

// BMP 文件头 (固定 14 字节)
struct BmpFileHeader {
    uint16_t file_type;      // 魔数：必须是 0x4D42 (ASCII 码 "BM")
    uint32_t file_size;      // 整个 BMP 文件的大小 (字节)
    uint16_t reserved1;      // 保留字 1，必须为 0
    uint16_t reserved2;      // 保留字 2，必须为 0
    uint32_t offset_data;    // 从文件头开始，到实际像素数据的字节偏移量
};

// BMP 信息头 (常见的 BITMAPINFOHEADER，固定 40 字节)
struct BmpInfoHeader {
    uint32_t size;           // 此结构体的大小 (必须为 40)
    int32_t  width;          // 图像宽度 (像素)
    int32_t  height;         // 图像高度 (像素)。正数表示图像倒置(自底向上)，负数表示正向
    uint16_t planes;         // 颜色平面数，必须为 1
    uint16_t bit_count;      // 色深，每像素的位数 (常见的有 24 或 32)
    uint32_t compression;    // 压缩类型 (0 表示不压缩 BI_RGB)
    uint32_t size_image;     // 纯像素数据的大小 (包含行对齐填充字节)
    int32_t  x_pixels_per_meter; // 水平分辨率
    int32_t  y_pixels_per_meter; // 垂直分辨率
    uint32_t colors_used;    // 调色板中实际使用的颜色数 (0 表示使用所有颜色)
    uint32_t colors_important; // 重要的颜色数 (0 表示所有颜色都重要)
};

// 恢复默认的内存对齐方式
#pragma pack(pop)

// BMP 图像管理类
class Bmp {
public:
    // 建议在构造函数中传入文件路径，实现 RAII（资源获取即初始化）
    explicit Bmp(const std::string& file_path, lcd& lcd_obj);
    ~Bmp();

    // 禁用拷贝，防止指针二次释放（Rule of Three/Five）
    Bmp(const Bmp&) = delete;
    Bmp& operator=(const Bmp&) = delete;

    // 提供给外部获取图像属性的接口
    int get_width() const { return info_header_.width; }
    int get_height() const { return std::abs(info_header_.height); }
    uint16_t get_bit_depth() const { return info_header_.bit_count; }

    // 未来可以扩展一个将图像绘制到 LCD 的方法
    void draw_to_lcd(int start_x, int start_y);

private:
    int bmp_fd_;
    BmpFileHeader file_header_;
    BmpInfoHeader info_header_;
    lcd& screen_;
    
    unsigned char* pixel_data_; // 用于存储读取出来的纯像素数据
    size_t data_size_;
    int row_bytes_;
};
