#include "tgaimage.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    image.set(52, 41, red);
    image.flip_vertically(); // 垂直方向翻转图片，反转y坐标，作者解释是希望图片的原点在左下角，但很多库原点都在左上角
    image.write_tga_file("output.tga");
    return 0;
}