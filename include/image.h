// include/image.h
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "../include/lcd.h"

// 图像数据类 (纯数据载体)
class Image {
public:
    explicit Image(const std::string& file_path);
    ~Image();

    // 禁用拷贝，防止指针二次释放
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    int get_width() const { return width_; }
    int get_height() const { return height_; }
    
    // 将当前图像画到屏幕上
    void draw(Lcd& screen, int start_x = 0, int start_y = 0);

private:
    int width_;
    int height_;
    int bit_count_;
    size_t data_size_;
    int row_bytes_;
    bool is_bottom_up_;     // 记录图片是否是倒置存储
    unsigned char* pixel_data_;
};

// 图像管理器 (全局单例 - 享元模式)
class ImageManager {
public:
    static ImageManager& get_instance();

    ImageManager(const ImageManager&) = delete;
    ImageManager& operator=(const ImageManager&) = delete;

    // 预加载并缓存图片
    void preload(const std::string& file_path);

    // 获取图片对象指针
    Image* get_image(const std::string& file_path);

    // 快捷绘制接口：直接根据路径画图
    void draw_image(Lcd& screen, const std::string& file_path, int start_x = 0, int start_y = 0);

    // 清理缓存，释放所有图片占用的内存
    void clear_cache();

private:
    ImageManager() = default;
    ~ImageManager();

    std::unordered_map<std::string, Image*> image_cache_;
};