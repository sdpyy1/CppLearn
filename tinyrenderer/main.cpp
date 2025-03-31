#include "thirdParty/tgaimage.h"
#include "model.h"
#include <vector>
#include <Eigen/Eigen>
constexpr TGAColor white   = {255, 255, 255, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  255,   0, 0, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};
constexpr static int width  = 2560;
constexpr static int height = 1920;
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
// 三角形面积，可能返回负数，表示背对屏幕
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}
// 绘制一个三角形
void drawTriangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, std::vector<std::vector<float>> * zBuffer,TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx);
    int bbminy = std::min(std::min(ay, by), cy);
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue;
            if
            framebuffer.set(x, y, color);
        }
    }
}
// 简单实现正交投影
Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}
int main() {
    Model * model = new Model("./obj/african_head/african_head.obj");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    // 定义一个zBuffer,并设置全部数据为最小负数
    std::vector<std::vector<float>> * zBuffer = new std::vector<std::vector<float>>(width, std::vector<float>(height,-std::numeric_limits<float>::max()));

    // 遍历obj文件中的每个三角形
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f curTriangle[3];
        // 将当前三角形的三个顶点都投影到屏幕
        for (int i=0; i<3; i++) curTriangle[i] = world2screen(model->vert(face[i]));
        drawTriangle(curTriangle, framebuffer, zBuffer, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }
    framebuffer.flip_vertically();
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}