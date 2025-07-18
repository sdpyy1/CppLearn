#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "pass/RenderPass.h"
#include "pass/GeometryPass.h"
#include "pass/LightingPass.h"
#include <iostream>

#include "precompute/preComputer.h"

int main()
{
    WindowManager app(800, 600);
    Scene scene(&app.camera);

    Model model("assets/helmet_pbr/DamagedHelmet.gltf");
    // TODO:cubemap加载目前必须放在模型加载之后
    GLuint envCubemap = scene.loadCubemap();
    preComputer preComputer(scene);
    GLuint irradianceMap = preComputer.computeIrradianceMap(envCubemap);
    PointLight pointLight(glm::vec3(.0f, .0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

    scene.addModel(model);
    scene.addLight(std::make_shared<PointLight>(pointLight));

    Shader pbrShader("shader/pbr.vert", "shader/pbr.frag");
    Shader skyboxShader("shader/skybox.vert", "shader/skybox.frag");
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
        int unit = 0;
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        pbrShader.setInt("irradianceMap", unit);
        scene.drawAll(pbrShader);


        // 天空盒
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("projection", scene.camera->getProjectionMatrix());
        skyboxShader.setMat4("view", scene.camera->getViewMatrix());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        scene.renderCube();
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}
