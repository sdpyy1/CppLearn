#include "model.h"
#include "thirdParty/OBJ_Loader.h"
Model::Model(const char * objFileName,const char * texFileName) : texture(texFileName){
    objl::Loader Loader;
    Loader.LoadFile(objFileName);
    this->modelMatrix = Eigen::Matrix4f::Identity();
    this->viewMatrix = Eigen::Matrix4f::Identity();
    this->projectionMatrix = Eigen::Matrix4f::Identity();
    this->viewportMatrix = Eigen::Matrix4f::Identity();
    for (const auto &mesh: Loader.LoadedMeshes){
        for(int i=0;i<mesh.Vertices.size();i+=3)
        {
            Triangle t;
            for(int j=0;j<3;j++)
            {
                // 此处设置每个三角形的属性
                t.setGlobalCoord(j, Vector4f(mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y,
                                              mesh.Vertices[i + j].Position.Z, 1.0));
                t.setNormal(j,Vector3f(mesh.Vertices[i+j].Normal.X,mesh.Vertices[i+j].Normal.Y,mesh.Vertices[i+j].Normal.Z));
                t.setTexCoord(j,Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
                Matrix4f mvp = projectionMatrix * viewMatrix * modelMatrix;
            }
            this->triangleList.push_back(t);
        }
    }
}
// 将角度转换为弧度
constexpr float deg2rad(float degrees) {
    return degrees * M_PI / 180.0f;
}
// 生成绕 x, y, z 轴旋转的变换矩阵
Eigen::Matrix4f rotation(float angleX, float angleY, float angleZ) {
    // 分别计算绕 x, y, z 轴旋转的矩阵
    Eigen::Matrix4f rotationX = Eigen::Matrix4f::Identity();
    float radX = deg2rad(angleX);
    rotationX(1, 1) = std::cos(radX);
    rotationX(1, 2) = -std::sin(radX);
    rotationX(2, 1) = std::sin(radX);
    rotationX(2, 2) = std::cos(radX);

    Eigen::Matrix4f rotationY = Eigen::Matrix4f::Identity();
    float radY = deg2rad(angleY);
    rotationY(0, 0) = std::cos(radY);
    rotationY(0, 2) = std::sin(radY);
    rotationY(2, 0) = -std::sin(radY);
    rotationY(2, 2) = std::cos(radY);

    Eigen::Matrix4f rotationZ = Eigen::Matrix4f::Identity();
    float radZ = deg2rad(angleZ);
    rotationZ(0, 0) = std::cos(radZ);
    rotationZ(0, 1) = -std::sin(radZ);
    rotationZ(1, 0) = std::sin(radZ);
    rotationZ(1, 1) = std::cos(radZ);

    // 组合三个旋转矩阵，这里假设旋转顺序为 Z -> Y -> X
    Eigen::Matrix4f modelMatrix = rotationX * rotationY * rotationZ;

    return modelMatrix;
}
// 生成平移变换矩阵
Eigen::Matrix4f translation(float tx, float ty, float tz) {
    Eigen::Matrix4f translationMatrix = Eigen::Matrix4f::Identity();
    translationMatrix(0, 3) = tx;
    translationMatrix(1, 3) = ty;
    translationMatrix(2, 3) = tz;
    return translationMatrix;
}

// 生成缩放变换矩阵
Eigen::Matrix4f scaling(float sx, float sy, float sz) {
    Eigen::Matrix4f scalingMatrix = Eigen::Matrix4f::Identity();
    scalingMatrix(0, 0) = sx;
    scalingMatrix(1, 1) = sy;
    scalingMatrix(2, 2) = sz;
    return scalingMatrix;
}
// 视图变换矩阵
Eigen::Matrix4f get_view_matrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up) {
    Eigen::Vector3f z = (eye - center).normalized();
    Eigen::Vector3f x = up.cross(z).normalized();
    Eigen::Vector3f y = z.cross(x);

    Eigen::Matrix4f view;
    view << x[0], x[1], x[2], -x.dot(eye),
            y[0], y[1], y[2], -y.dot(eye),
            z[0], z[1], z[2], -z.dot(eye),
            0, 0, 0, 1;

    return view;
}
// 生成透视变换矩阵，左乘后直接到 NDC 标准坐标
Eigen::Matrix4f get_projection_matrix(float fovY, float aspectRatio, float near, float far) {
    float f = 1.0f / std::tan(deg2rad(fovY) / 2.0f);
    Eigen::Matrix4f projection;
    projection << f / aspectRatio, 0, 0, 0,
            0, f, 0, 0,
            0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
            0, 0, -1, 0;
    return projection;
}
void Model::setModelTransformation(float angleX, float angleY, float angleZ, float tx, float ty, float tz, float sx, float sy, float sz){
    if (triangleList.empty()){
        std::cout << "模型未导入！"<<std::endl;
        return;
    }
    Eigen::Matrix4f rotationMatrix = rotation(angleX, angleY, angleZ);
    Eigen::Matrix4f translationMatrix = translation(tx, ty, tz);
    Eigen::Matrix4f scalingMatrix = scaling(sx, sy, sz);
    // 按缩放 -> 旋转 -> 平移的顺序组合变换矩阵
    modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;
}
// 应用视图变换的函数
void Model::setViewTransformation(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up) {
    viewMatrix = get_view_matrix(eye, center, up);
}
// 应用透视变换的函数
void Model::setProjectionTransformation(float fovY, float aspectRatio, float near, float far) {
    projectionMatrix = get_projection_matrix(fovY, aspectRatio, near, far);
}
// 生成视口变换矩阵（NDC → 屏幕坐标）
void Model::setViewPortMatrix(int width, int height) {
    Eigen::Matrix4f viewport = Eigen::Matrix4f::Identity();
    viewport(0, 0) = width / 2.0f;
    viewport(0, 3) = width / 2.0f;
    viewport(1, 1) = height / 2.0f;
    viewport(1, 3) = height / 2.0f;
    this->viewportMatrix = viewport;
}
Matrix4f Model::getAllTransMatrix(){
    return viewportMatrix * projectionMatrix * viewMatrix * modelMatrix;
}


Model::~Model() {
}
