#include "thirdParty/tgaimage.h"
#include "model.h"
#include <vector>
#include <cmath>
#include "iostream"
using namespace std;
constexpr static int width  = 1000;
constexpr static int height = 1000;
Eigen::Vector3f lightDir{1,1,0};
Eigen::Vector3f lightIntensity{5,5,5};
Eigen::Vector3f viewDir{0,0,5};
// 计算三角形面积，可能返回负数，表示背对屏幕
float signed_triangle_area(float ax, float ay, float bx, float by, float cx, float cy) {
    return .5f*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}
Eigen::Vector3f TGAColorToVector3f(const TGAColor& color) {
    float r = static_cast<float>(color.bgra[2]) / 255.0f;
    float g = static_cast<float>(color.bgra[1]) / 255.0f;
    float b = static_cast<float>(color.bgra[0]) / 255.0f;
    return Eigen::Vector3f(r, g, b);
}
// 将Eigen::Vector3f转换为TGAColor的函数
TGAColor Vector3fToTGAColor(const Eigen::Vector3f& vectorColor) {
    auto clamp = [](float v) { return std::max(0.0f, std::min(1.0f, v)); };
    unsigned char r = static_cast<unsigned char>(clamp(vectorColor.x()) * 255.0f);
    unsigned char g = static_cast<unsigned char>(clamp(vectorColor.y()) * 255.0f);
    unsigned char b = static_cast<unsigned char>(clamp(vectorColor.z()) * 255.0f);
    return TGAColor(r, g, b);
}
// blinnPhongShading
TGAColor blinnPhongShading(const TGAColor & kdColor,const Vector3f & point,const Vector3f & normal) {
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = TGAColorToVector3f(kdColor);
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);
    // 环境光强度
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    // 高光的指数，越大对角度越敏感
    float p = 150;
    // 计算点到光源的向量
    Eigen::Vector3f light_vec = lightDir - point;
    // 计算点到光源的距离
    float r = light_vec.norm();
    // 归一化从点到光源的向量
    Eigen::Vector3f light_dir = light_vec.normalized();
    // 漫反射
    Eigen::Vector3f diffuse = kd.cwiseProduct(lightIntensity / (r * r)) * std::max(0.0f, normal.dot(light_dir));
    // 高光反射
    // 计算从表面点到观察者的向量
    Eigen::Vector3f view_dir = (viewDir - point).normalized();
    // 计算半程向量
    Eigen::Vector3f halfVector = (light_dir + view_dir).normalized();
    Eigen::Vector3f specular = ks.cwiseProduct(lightIntensity / (r * r)) * std::pow(std::max(0.0f, normal.dot(halfVector)), p);
    Eigen::Vector3f all = diffuse + specular + ka.cwiseProduct(amb_light_intensity);
    return Vector3fToTGAColor(all);
}
// 插值函数
Eigen::Vector3f interpolate(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, double alpha, double beta, double gamma) {
    return alpha * v0 + beta * v1 + gamma * v2;
}
float interpolate(float v0, float v1, float v2, float alpha, float beta, float gamma) {
    return alpha * v0 + beta * v1 + gamma * v2;
}

// 绘制一个三角形
void drawTriangle(Triangle &triangle, TGAImage &framebuffer, std::vector<std::vector<float>> * zBuffer,Texture &texture) {
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
    float total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return;

    #pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            // 虽然可以把整个三角形直接剔除，但是我希望只是把屏幕外的像素剔除
            if (x<0||x>=width || y<0||y>=height){
                continue;
            }
            float alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            float beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            float gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // 说明当前像素不在三角形内部
            float barycentricZ = interpolate(triangle.screenCoords[0].z(), triangle.screenCoords[1].z(), triangle.screenCoords[2].z(), alpha, beta, gamma);
            Eigen::Vector3f barycentricNorm = interpolate(triangle.normal[0], triangle.normal[1], triangle.normal[2], alpha, beta, gamma);
            Eigen::Vector3f barycentricGlobalCoord = interpolate(triangle.globalCoords[0].head<3>(), triangle.globalCoords[1].head<3>(), triangle.globalCoords[2].head<3>(), alpha, beta, gamma);
            float texU = interpolate(triangle.texCoords[0].x() ,triangle.texCoords[1].x() ,triangle.texCoords[2].x(),alpha,beta,gamma);
            float texV = interpolate(triangle.texCoords[0].y() ,triangle.texCoords[1].y() ,triangle.texCoords[2].y(),alpha,beta,gamma);
            TGAColor texColor = texture.getColor(texU, texV);

            // zbuffer中缓存的渲染物体距离小于当前渲染物体的距离时，才覆盖渲染
            if (zBuffer->at(x).at(y) < barycentricZ){
                zBuffer->at(x).at(y) = barycentricZ;
                framebuffer.set(x,y, blinnPhongShading(texColor,barycentricGlobalCoord,barycentricNorm));
            }
        }
    }
}

int main() {
    Model model("./obj/diablo3_pose/diablo3_pose.obj","./obj/diablo3_pose/diablo3_pose_diffuse.tga");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    // 定义一个zBuffer,并设置全部数据为最小负数
    auto * zBuffer = new std::vector<std::vector<float>>(width, std::vector<float>(height,std::numeric_limits<float>::lowest()));
    // 获取法线贴图
    Texture nm("./obj/diablo3_pose/diablo3_pose_nm.tga");
    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;
    float tx = 0.0f;
    float ty = 0.0f;
    float tz = 0.0f;
    float sx = 1.0f;
    float sy = 1.0f;
    float sz = 1.0f;
    Eigen::Vector3f eye_pos = viewDir;
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



