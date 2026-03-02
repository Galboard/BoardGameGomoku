#include "include/lcd.h"
#include "include/ui.h"
#include "include/font.h"
#include <exception>
#include <iostream>

int main() {
    try {
        Lcd& screen = Lcd::get_instance();
        screen.clear(0x00D0D0D0); // 灰色背景

        // 加载字体文件（请确保路径下有这个ttf文件）
        Font main_font("SimSun.ttf", 40); 

        // 创建一个按钮
        Button btn(300, 200, 200, 60, "重新开始");
        btn.set_bg_color(0x00336699);   // 蓝色按钮底色
        btn.set_text_color(0x00FFFFFF); // 白色文字

        // 绘制按钮（文字会被优雅地画上去）
        btn.draw(screen, &main_font); 
        screen.show();
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}