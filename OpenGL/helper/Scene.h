#ifndef OPENGLRENDER_SCENE_H
#define OPENGLRENDER_SCENE_H

#include <vector>
#include <memory>
#include "../model/Model.h"
#include "../core/Camera.h"
#include "Light.h"
#include "../core/Shader.h"

struct IBLMaps {
    GLuint irradianceMap = 0;
    GLuint prefilterMap = 0;
};
class Scene
{
public:
    int width;
    int height;
    Camera* camera = nullptr;
    std::vector<std::shared_ptr<Light>> lights;
    GLuint envCubemap = 0;
    GLuint prefilterMap = 0;
    GLuint lutMap = 0;
    GLuint  irradianceMap = 0;
    std::vector<Model> models;
    Model* selModel = nullptr;
    std::shared_ptr<Light> selLight = nullptr;

    // GUI参数
    bool enableOutline = false;
    bool drawLightCube = true;
    // 0: 无阴影 1:硬阴影 2:PCF 3:PCSS
    int shadowType = 3;
    int pcfScope = 5;
    float PCSSBlockerSearchRadius = 1.0;
    float PCSSScale = 0.01;
    float PCSSKernelMax = 10.0;

    explicit Scene(Camera* camera);
    void addModel(Model& model);
    void addModel(const string& path);
    void addLight(const std::shared_ptr<Light>& light);
    void addDefaultModel(const string& name);
    void drawAll(Shader& shader);
    void loadHDRAndIBL(const std::string& hdrPath);
    void renderCube();
    void renderQuad();
    void renderSphere();

    void disableIBL();

private:

    GLuint loadCubemapFromHDR(const char *path);
    GLuint loadCubemapFromSkybox(const string &path);
    std::map<std::string, IBLMaps> iblCache;
    std::map<std::string, GLuint> envCubemapCache;
    std::string currentHDRPath;
    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    unsigned int sphereVAO = 0;
    unsigned int indexCount = 0;
    void setVP(Shader& shader) const;
    void createDefaultTexture();
    static GLuint create1x1Texture(const glm::vec4 &color, GLenum format, GLenum internalFormat);
    GLuint computeLutMap();
    GLuint computePrefilterMap(GLuint envCubemap);
    GLuint computeIrradianceMap(GLuint envCubemap);
};

#endif //OPENGLRENDER_SCENE_H
