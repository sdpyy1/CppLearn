//
// Created by 刘卓昊 on 2025/4/1.
//
#include "thirdParty/tgaimage.h"
// 画线尝试1
void drawLine_first(int ax, int ay, int bx, int by, TGAImage &img, TGAColor color){
    for(float t = 0;t<=1;t+=0.02){
        int x = std::round(ax + t * (bx - ax)); // round会进行四舍五入
        int y = std::round(ay + t * (by - ay));
        img.set(x,y,color);
    }
}
// 画线尝试2
void drawLine_second(int ax, int ay, int bx, int by, TGAImage &img, TGAColor color){
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x = ax ; x<= bx; x++) { // 不再以t控制，而是以x的进行进行控制，保证了水平方向上不会有空隙
        // 如果不加强制转换，当分子分母都是整数时，计算结果的小数部分会被截断
        float t = (x-ax)/static_cast<float>(bx-ax); // 变换了形式，表示出当x移动一格时，t是多少，
        int y = std::round( ay + (by-ay)*t );
        img.set(x, y, color);
    }
}
// 画线尝试3
void drawLine_third(int ax, int ay, int bx, int by, TGAImage &img, TGAColor color){
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the drawLine is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x = ax ; x<= bx; x++) { // 不再以t控制，而是以x的进行进行控制，保证了水平方向上不会有空隙
        // 如果不加强制转换，当分子分母都是整数时，计算结果的小数部分会被截断
        float t = (x-ax)/static_cast<float>(bx-ax); // 变换了形式，表示出当x移动一格时，t是多少，
        int y = std::round( ay + (by-ay)*t );
        if (steep) // if transposed, de−transpose
            img.set(y, x, color);
        else
            img.set(x, y, color);
    }
}
// 最终版本 对计算进行了优化
void drawLine(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the drawLine is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);
    }
}