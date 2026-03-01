// include/event.h
#pragma once

#include <linux/input.h>
#include <string>

constexpr char kDefaultInputDevPath[] = "/dev/input/event0";

// 自定义一个简化的触摸点结构体，方便游戏逻辑调用
struct TouchPoint {
    int x;
    int y;
    bool is_pressed; // true 为按下，false 为松开
    friend TouchPoint& operator+(const TouchPoint& other);
    friend TouchPoint& operator-(const TouchPoint& other);
};

enum class EventStatus {
    NONE,       // 无动作或无效操作
    TAP,        // 原地点击
    MOVE_UP,
    MOVE_RIGHT,
    MOVE_DOWN,
    MOVE_LEFT
};

class InputEvent {
public:
    static InputEvent& get_instance(const std::string& dev_path = kDefaultInputDevPath);
    
    InputEvent(const InputEvent&) = delete;
    InputEvent& operator=(const InputEvent&) = delete;
    ~InputEvent();

    // 底层接口：读取一次原始事件
    bool read_raw_event(struct input_event& ev);

    // 高层接口：获取经过映射的触摸点坐标 (自动映射到屏幕分辨率)
    bool get_touch_point(TouchPoint& point);

    EventStatus get_current_status(TouchPoint& out_point);
private:
    explicit InputEvent(const std::string& dev_path);
    int dev_fd_;

    // 保存触摸屏底层的真实物理分辨率范围，用于坐标映射
    int touch_max_x_;
    int touch_max_y_;
};