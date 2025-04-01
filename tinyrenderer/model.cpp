#include "model.h"
#include "thirdParty/OBJ_Loader.h"
// 加载模型就直接用教程的代码了！
Model::Model(const char *objFileName,const char * texFileName) : texture(texFileName){
    objl::Loader Loader;
    Loader.LoadFile(objFileName);

    for (const auto &mesh: Loader.LoadedMeshes){
        for(int i=0;i<mesh.Vertices.size();i+=3)
        {

            Triangle * t = new Triangle;
            for(int j=0;j<3;j++)
            {
                t->setGlobalCoords(j, Vector4f(mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y,mesh.Vertices[i + j].Position.Z, 1.0));
                t->setNormal(j,Vector3f(mesh.Vertices[i+j].Normal.X,mesh.Vertices[i+j].Normal.Y,mesh.Vertices[i+j].Normal.Z));
                t->setTexCoord(j,Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
            }
            this->triangleList.push_back(*t);
        }
    }
}

Model::~Model() {
}
