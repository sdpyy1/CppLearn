#ifndef OPENGLRENDER_SCENE_H
#define OPENGLRENDER_SCENE_H

#include <vector>
#include <memory>
#include "../model/Model.h"
#include "../core/Camera.h"
#include "Light.h"
#include "../core/Shader.h"

class Scene
{
public:
    int width;
    int height;
    Camera* camera = nullptr;
    std::vector<std::shared_ptr<Light>> lights;
    GLuint envCubemap = 0;
    std::vector<Model> models;

    explicit Scene(Camera* camera);

    void addModel(const Model& model);
    void addModel(const string& path);
    void addLight(const std::shared_ptr<Light>& light);
    void addDefaultModel(const string& name);

    void drawAll(Shader& shader);
    void renderCube();
    void renderQuad();
    void renderSphere();
    GLuint loadCubemapFromHDR(const char *path);
    GLuint loadCubemapFromSkybox(const string &path);

private:
    GLuint defaultAlbedo = 0;
    GLuint defaultNormal = 0;
    GLuint defaultMetallic = 0;
    GLuint defaultRoughness = 0;
    GLuint defaultAO = 0;
    GLuint defaultEmissive = 0;
    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    unsigned int sphereVAO = 0;
    unsigned int indexCount = 0;
    void setVPAndUseShader(Shader& shader) const;

    void createDefaultTexture();

    static GLuint create1x1Texture(const glm::vec4 &color, GLenum format, GLenum internalFormat);
};

#endif //OPENGLRENDER_SCENE_H
