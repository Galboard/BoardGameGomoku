#include "include/event.h"
#include "include/lcd.h"

#include <iostream>

int main() {
    try {
        lcd screen(DEFAULT_DEVPATH);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}