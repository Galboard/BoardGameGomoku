#include "../include/event.h"
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

event::event(const std::string& dev_path) {
    dev_fd = open(dev_path.c_str(), O_RDWR);
    if (dev_fd < 0) {
        throw std::runtime_error("Open failed: " + std::string(strerror(errno)));
    }
}

event::~event() {
    if (dev_fd > 0) {
        close(dev_fd);
    }
}

