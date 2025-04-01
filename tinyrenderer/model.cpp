#include "model.h"
#include "thirdParty/OBJ_Loader.h"
Model::Model(const char * objFileName,const char * texFileName) : texture(texFileName){
    objl::Loader Loader;
    Loader.LoadFile(objFileName);
    this->modelMatrix = Eigen::Matrix4f::Identity();
    this->viewMatrix = Eigen::Matrix4f::Identity();
    this->projectionMatrix = Eigen::Matrix4f::Identity();
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


Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos, Eigen::Vector3f target, Eigen::Vector3f up) {
    // TODO:目前只支持摄像机沿z轴移动
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
            0,1,0,-eye_pos[1],
            0,0,1,-eye_pos[2],
            0,0,0,1;

    view = translate*view;

    return view;
}
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float n, float f) {

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    float t = -tan((eye_fov/360)*M_PI)*(abs(n)); //top
    float r = t/aspect_ratio;

    Eigen::Matrix4f Mp;//透视矩阵
    Mp <<
       n, 0, 0,   0,
            0, n, 0,   0,
            0, 0, n+f, -n*f,
            0, 0, 1,   0;
    Eigen::Matrix4f Mo_tran;//平移矩阵
    Mo_tran <<
            1, 0, 0, 0,
            0, 1, 0, 0,  //b=-t;
            0, 0, 1, -(n+f)/2 ,
            0, 0, 0, 1;
    Eigen::Matrix4f Mo_scale;//缩放矩阵
    Mo_scale <<
             1/r,     0,       0,       0,
            0,       1/t,     0,       0,
            0,       0,       2/(n-f), 0,
            0,       0,       0,       1;
    projection = (Mo_scale*Mo_tran)* Mp;//投影矩阵
    //这里一定要注意顺序，先透视再正交;正交里面先平移再缩放；否则做出来会是一条直线！
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
void Model::setViewTransformation(Eigen::Vector3f eye_pos, Eigen::Vector3f target, Eigen::Vector3f up) {
    viewMatrix = get_view_matrix(eye_pos,target,up);
}
// 应用透视变换的函数
void Model::setProjectionTransformation(float fovY, float aspectRatio, float near, float far) {
    projectionMatrix = get_projection_matrix(fovY, aspectRatio, near, far);
}

Matrix4f Model::getMVP(){
    return projectionMatrix * viewMatrix * modelMatrix;
}


Model::~Model() {
}
