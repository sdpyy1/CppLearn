#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "pass/GeometryPass.h"
#include "pass/LightingPass.h"
#include <iostream>

#include "precompute/preComputer.h"



int main()
{
    WindowManager app(1280, 720);
    Scene scene(&app.camera);
    // 加载Shader
    Shader pbrShader("shader/pbr.vert", "shader/pbr.frag");
    Shader skyboxShader("shader/skybox.vert", "shader/skybox.frag");

    // 搭建场景
    PointLight pointLight(glm::vec3(.0f, .0f, -5.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
//    scene.addDefaultModel("helmet");
    scene.addDefaultModel("gun");
//    scene.addModel( "assets/asw/scene.gltf");
//    scene.addLight(std::make_shared<PointLight>(pointLight));

    // IBL
    // TODO:cubemap加载目前必须放在模型加载之后
    scene.loadCubemapFromHDR("assets/HDR/2.hdr");
//    scene.loadCubemapFromSkybox("assets/cubemap/Skybox");
    preComputer preComputer(scene);
    GLuint irradianceMap = preComputer.computeIrradianceMap();
    GLuint prefilterMap = preComputer.computePrefilterMap();
    GLuint lutMap = preComputer.computeLutMap();

    // 初始化Pass
    GeometryPass geometryPass(scene);
    geometryPass.init();
    LightingPass lighting(scene,geometryPass.gPosition, geometryPass.gNormal, geometryPass.gAlbedo, geometryPass.gMaterial);
    lighting.init();

    while (!glfwWindowShouldClose(app.window))
    {
        app.processInput();
        GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // G-Buffer Pass
        geometryPass.render();
        geometryPass.debug();

        // PBR材质渲染
        pbrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        pbrShader.setInt("irradianceMap", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        pbrShader.setInt("prefilterMap", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, lutMap);
        pbrShader.setInt("lutMap", 2);
        scene.drawAll(pbrShader);
        // 天空盒
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("projection", scene.camera->getProjectionMatrix());
        skyboxShader.setMat4("view", scene.camera->getViewMatrix());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, scene.envCubemap);
        scene.renderCube();
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}
