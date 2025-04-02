#include "thirdParty/tgaimage.h"
#include "model.h"
#include <vector>
#include <cmath>
#include "iostream"

constexpr static int width  = 1000;
constexpr static int height = 1000;

// 计算三角形面积，可能返回负数，表示背对屏幕
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}
// 绘制一个三角形
void drawTriangle(Triangle triangle, TGAImage &framebuffer, std::vector<std::vector<float>> * zBuffer,Texture &texture) {
    float ax = triangle.screenCoords[0].x();
    float ay = triangle.screenCoords[0].y();
    float bx = triangle.screenCoords[1].x();
    float by = triangle.screenCoords[1].y();
    float cx = triangle.screenCoords[2].x();
    float cy = triangle.screenCoords[2].y();
    int bbminx = std::floor(std::min(std::min(ax, bx), cx));
    int bbminy = std::ceil(std::min(std::min(ay, by), cy));
    int bbmaxx = std::floor(std::max(std::max(ax, bx), cx));
    int bbmaxy = std::ceil(std::max(std::max(ay, by), cy));

    // 如果面积为负数，背对屏幕，被裁剪
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return;

    #pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            // 虽然可以把整个三角形直接剔除，但是我希望只是把屏幕外的像素剔除
            if (x<0||x>=width || y<0||y>=height){
                continue;
            }
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // 说明当前像素不在三角形内部
            float barycentricZ = alpha*triangle.screenCoords[0].z() + beta*triangle.screenCoords[1].z() + gamma*triangle.screenCoords[2].z();
            float texU = alpha*triangle.texCoords[0].x() + beta*triangle.texCoords[1].x() + gamma*triangle.texCoords[2].x();
            float texV = alpha*triangle.texCoords[0].y() + beta*triangle.texCoords[1].y() + gamma*triangle.texCoords[2].y();
            // zbuffer中缓存的渲染物体距离小于当前渲染物体的距离时，才覆盖渲染
            if (zBuffer->at(x).at(y) < barycentricZ){
                zBuffer->at(x).at(y) = barycentricZ;
                framebuffer.set(x,y,texture.getColor(texU,texV));
            }
        }
    }
}

int main() {
    Model model("./obj/african_head/african_head.obj","./obj/african_head/african_head_diffuse.tga");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    // 定义一个zBuffer,并设置全部数据为最小负数
    auto * zBuffer = new std::vector<std::vector<float>>(width, std::vector<float>(height,std::numeric_limits<float>::lowest()));
    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;
    float tx = 0.0f;
    float ty = 0.0f;
    float tz = 0.0f;
    float sx = 1.0f;
    float sy = 1.0f;
    float sz = 1.0f;
    Eigen::Vector3f eye_pos(0.0f, 0.0f, 3.0f);
    Eigen::Vector3f eye_dir(0.0f, 0.0f, -1.0f);
    Eigen::Vector3f up(0.0f, 1.0f, 0.0f);
    float fovY = 45.0f;
    float aspectRatio = 1.0f;
    float near = 0.1f;
    float far = 100.0f;
    model.setModelTransformation(angleX, angleY, angleZ, tx, ty, tz, sx, sy, sz);
    model.setViewTransformation(eye_pos,eye_dir,up);
    model.setProjectionTransformation(fovY, aspectRatio, near, far);

    // 获取所有变换矩阵
    Eigen::Matrix4f mvp = model.getMVP();

    // 遍历obj文件中的每个三角形
    for (Triangle triangle : model.triangleList) {
        // 坐标投影
        triangle.setScreenCoords(mvp,width,height);
        // 绘制三角形
        drawTriangle(triangle, framebuffer, zBuffer, model.texture);
    }
    framebuffer.write_tga_file("framebuffer.tga");
    delete(zBuffer);
    return 0;
}



