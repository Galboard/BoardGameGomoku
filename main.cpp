#include "include/event.h"
#include "include/lcd.h"

#include <iostream>

int main() {
    try {
        lcd& screen = lcd::get_instance();
        screen.clear(0x00ffffff);
        screen.render_circle(50, 400, 240, 0x0000ff);
        screen.show();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}