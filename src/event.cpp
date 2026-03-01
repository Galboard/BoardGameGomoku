// src/event.cpp
#include "../include/event.h"
#include "../include/lcd.h"  // 【新增】：引入 Lcd 单例以获取真实屏幕分辨率
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <chrono>
#include <sys/ioctl.h> // 确保包含了 ioctl 的头文件

InputEvent::InputEvent(const std::string& dev_path) 
    : dev_fd_(-1), touch_max_x_(0), touch_max_y_(0) { 
    
    dev_fd_ = open(dev_path.c_str(), O_RDONLY); 
    if (dev_fd_ < 0) {
        throw std::runtime_error("InputEvent open failed: " + std::string(strerror(errno)));
    }

    // 【动态联动】：获取 LCD 真实分辨率作为 ioctl 失败时的垫底默认值
    int default_w = Lcd::get_instance().get_width();
    int default_h = Lcd::get_instance().get_height();
    touch_max_x_ = default_w;
    touch_max_y_ = default_h;

    // 通过 ioctl 动态获取触摸屏的真实物理范围
    struct input_absinfo abs_x, abs_y;
    if (ioctl(dev_fd_, EVIOCGABS(ABS_X), &abs_x) >= 0 && ioctl(dev_fd_, EVIOCGABS(ABS_Y), &abs_y) >= 0) {
        touch_max_x_ = abs_x.maximum;
        touch_max_y_ = abs_y.maximum;
        std::cout << "[Info] Touch panel real resolution: " 
                  << touch_max_x_ << " x " << touch_max_y_ << std::endl;
    } else {
        std::cerr << "[Warn] Could not get absolute axis info, using LCD resolution " 
                  << default_w << "x" << default_h << " as fallback." << std::endl;
    }
}

InputEvent::~InputEvent() {
    if (dev_fd_ >= 0) {
        close(dev_fd_);
    }
}

InputEvent& InputEvent::get_instance(const std::string& dev_path) {
    static InputEvent instance(dev_path);
    return instance;
}

bool InputEvent::read_raw_event(struct input_event& ev) {
    int ret = read(dev_fd_, &ev, sizeof(struct input_event));
    return ret == sizeof(struct input_event);
}

// 核心解析逻辑：从碎片化的输入事件中拼凑出一个完整的触摸点
bool InputEvent::get_touch_point(TouchPoint& point) {
    struct input_event ev;
    bool point_updated = false;

    // 【动态联动】：每次映射坐标前，获取当前 LCD 的真实分辨率
    int screen_w = Lcd::get_instance().get_width();
    int screen_h = Lcd::get_instance().get_height();

    while (read_raw_event(ev)) {
        if (ev.type == EV_ABS) { 
            if (ev.code == ABS_X || ev.code == ABS_MT_POSITION_X) {
                // 等比例映射：把物理坐标映射到真实的 LCD 宽度上
                point.x = (ev.value * screen_w) / touch_max_x_;
                point_updated = true;
            } else if (ev.code == ABS_Y || ev.code == ABS_MT_POSITION_Y) {
                // 等比例映射：把物理坐标映射到真实的 LCD 高度上
                point.y = (ev.value * screen_h) / touch_max_y_;
                point_updated = true;
            }
        } else if (ev.type == EV_KEY) { 
            if (ev.code == BTN_TOUCH) {
                point.is_pressed = (ev.value > 0);
                point_updated = true;
            }
        } else if (ev.type == EV_SYN) { 
            if (point_updated) {
                return true; 
            }
        }
    }
    return false;
}

TouchPoint operator+(const TouchPoint& other1, const TouchPoint& other2) {
    TouchPoint new_point;
    new_point.x = other1.x + other2.x;
    new_point.y = other1.y + other2.y;
    new_point.is_pressed = other1.is_pressed || other2.is_pressed;  // 逻辑或，任一触摸点按下则结果为按下
    return new_point;
}

TouchPoint operator-(const TouchPoint& other1, const TouchPoint& other2) {
    TouchPoint new_point;
    new_point.x = other1.x - other2.x;
    new_point.y = other1.y - other2.y;
    new_point.is_pressed = other1.is_pressed || other2.is_pressed;  // 逻辑或，任一触摸点按下则结果为按下
    return new_point;
}

EventStatus InputEvent::get_current_status(TouchPoint& out_point) {
    TouchPoint start_point, end_point;
    
    // 1. 捕获按下瞬间
    while (get_touch_point(start_point)) {
        if (start_point.is_pressed) break;
    }
    auto start_time = std::chrono::steady_clock::now();

    // 2. 捕获抬起瞬间
    while (get_touch_point(end_point)) {
        if (!end_point.is_pressed) break;
    }
    auto end_time = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    int dx = end_point.x - start_point.x;
    int dy = end_point.y - start_point.y;
    int abs_dx = std::abs(dx);
    int abs_dy = std::abs(dy);

    const int kMinSwipeDistance = 50; 
    const int kMaxSwipeTime = 400;    
    const float kDirectionRatio = 1.5f; 

    // --- 核心逻辑判定树 ---
    
    // 【关键修改】：情况 A：位移极小，判定为点击 (TAP)
    if (abs_dx < kMinSwipeDistance && abs_dy < kMinSwipeDistance) {
        // 把点击的具体坐标传给外面！
        // 通常点击操作，以手指按下的起始坐标为准最符合直觉
        out_point.x = start_point.x;
        out_point.y = start_point.y;
        out_point.is_pressed = false; // 触发动作时，手指已经抬起了
        return EventStatus::TAP; 
    }

    // 其他情况暂不需要传出坐标（或者你可以让滑动传出起始坐标）
    if (duration > kMaxSwipeTime) return EventStatus::NONE; 

    if (abs_dx > abs_dy * kDirectionRatio) {
        return (dx > 0) ? EventStatus::MOVE_RIGHT : EventStatus::MOVE_LEFT;
    } else if (abs_dy > abs_dx * kDirectionRatio) {
        return (dy > 0) ? EventStatus::MOVE_DOWN : EventStatus::MOVE_UP;
    }

    return EventStatus::NONE;
}