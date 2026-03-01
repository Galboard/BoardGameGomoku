#pragma once

#include <linux/input.h>
#include <string>

#define DEFAULT_INPUT_DEV_PATH "dev/input/event0"

class event {
public:
    static event& get_instance();
    ~event();
private:
    explicit event(const std::string& dev_path = DEFAULT_INPUT_DEV_PATH);
    int dev_fd;
};
