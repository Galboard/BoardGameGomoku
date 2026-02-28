#include "include/event.h"
#include "include/lcd.h"

#include <iostream>

int main() {
    try {
        lcd screen(DEFAULT_DEVPATH);
        screen.render_rectangle(300, 200, 100, 100, 0x00ffff00);
        screen.show();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}