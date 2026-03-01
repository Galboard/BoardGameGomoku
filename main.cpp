#include "include/event.h"
#include "include/lcd.h"
#include "include/image.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

int main() {
    try {
        Lcd& screen = Lcd::get_instance();
        InputEvent& input = InputEvent::get_instance();
        BmpImage bmp_selector("test.bmp");
        bmp_selector.draw_to_lcd(screen);
        screen.show();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}