#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "helper/ImGUIManger.h"
#include "RenderPipeline.h"
#include "glm/gtx/string_cast.hpp"
struct AtmosphereParameter {
    float RayleighScatteringScalarHeight;
    float MieScatteringScalarHeight;
    float MieAnisotropy;
    float OzoneLevelCenterHeight;
    float OzoneLevelWidth;
    float PlanetRadius;
    float AtmosphereHeight;
    float radiusCloudStart;
    float radiusCloudEnd;
    glm::vec3 earthCenter;
};
glm::vec2 RaySphereDst(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& pos, const glm::vec3& rayDir)
{
    glm::vec3 oc = pos - sphereCenter;
    float b = glm::dot(rayDir, oc);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    float t = b * b - c; // t > 0 有两个交点, =0 相切, <0 不相交

    if (t < 0.0f) {
        // 射线不相交球
        return glm::vec2(0.0f, 0.0f);
    }

    float delta = std::sqrt(t);
    float dstToSphere = std::max(-b - delta, 0.0f);
    float dstInSphere = std::max(-b + delta - dstToSphere, 0.0f);

    return glm::vec2(dstToSphere, dstInSphere);
}

// RayCloudLayerDst 函数
glm::vec2 RayCloudLayerDst(const glm::vec3& sphereCenter, float earthRadius, float heightMin, float heightMax,
                           const glm::vec3& pos, const glm::vec3& rayDir, bool isShape = true)
{
    glm::vec2 cloudDstMin = RaySphereDst(sphereCenter, heightMin + earthRadius, pos, rayDir);
    glm::vec2 cloudDstMax = RaySphereDst(sphereCenter, heightMax + earthRadius, pos, rayDir);

    float dstToCloudLayer = 0.0f;
    float dstInCloudLayer = 0.0f;

    if (isShape)
    {
        // 在地表上
        if (pos.y <= heightMin + earthRadius)
        {
            glm::vec3 startPos = pos + rayDir * cloudDstMin.y;
            if (startPos.y >= 0.0f)
            {
                dstToCloudLayer = cloudDstMin.y;
                dstInCloudLayer = cloudDstMax.y - cloudDstMin.y;
            }
            return glm::vec2(dstToCloudLayer, dstInCloudLayer);
        }

        // 在云层内
        if (pos.y > heightMin + earthRadius && pos.y <= heightMax + earthRadius)
        {
            dstToCloudLayer = 0.0f;
            dstInCloudLayer = cloudDstMin.y > 0.0f ? cloudDstMin.x : cloudDstMax.y;
            return glm::vec2(dstToCloudLayer, dstInCloudLayer);
        }

        // 在云层外
        dstToCloudLayer = cloudDstMax.x;
        dstInCloudLayer = cloudDstMin.y > 0.0f ? cloudDstMin.x - dstToCloudLayer : cloudDstMax.y;
    }
    else // 光照步进，步进开始点一定在云层内
    {
        dstToCloudLayer = 0.0f;
        dstInCloudLayer = cloudDstMin.y > 0.0f ? cloudDstMin.x : cloudDstMax.y;
    }

    return glm::vec2(dstToCloudLayer, dstInCloudLayer);
}
float sampleNoise(const glm::vec3& pos) {
    // 测试用简单噪声
    return glm::clamp(glm::fract(glm::sin(glm::dot(pos, glm::vec3(12.9898,78.233,45.164))) * 43758.5453f), 0.0f, 1.0f);
}

glm::vec3 rayMarchToLight(const AtmosphereParameter& param, const glm::vec3& pos, const glm::vec3& lightDir) {
    // 简化版本：返回白光
    return glm::vec3(1.0f);
}

float phase(float cosAngle) {
    // 简单Henyey-Greenstein近似
    float g = 0.8f; // Mie散射方向性
    return (1.0f - g * g) / glm::pow(1.0f + g*g - 2.0f*g*cosAngle, 1.5f);
}

// 主函数
glm::vec4 cloudRayMarching(const AtmosphereParameter& param, const glm::vec3& worldPosInPlanet,
                           const glm::vec3& viewDir, float depth, const glm::vec3& lightPos)
{
    const int SAMPLE_NUM = 16;
    glm::vec3 lightDir = glm::normalize(-lightPos); // 根据需求可改成 lightPos - worldPos
    glm::vec2 rayBoxInfo = RayCloudLayerDst(param.earthCenter, param.PlanetRadius, param.radiusCloudStart, param.radiusCloudEnd,
                                           worldPosInPlanet, viewDir, true);

    float dstToCloud = rayBoxInfo.x;
    float dstInsideCloud = rayBoxInfo.y;

    if(dstInsideCloud <= 0.0f || depth < 99.0f) {
        return glm::vec4(0.0f,0.0f,0.0f,1.0f);
    }

    float dstLimit = dstInsideCloud;
    float cosAngle = glm::dot(viewDir, lightDir);
    float phaseVal = phase(cosAngle);

    glm::vec3 startPos = worldPosInPlanet + viewDir * dstToCloud;
    glm::vec3 sumLight(0.0f);
    float stepSize = dstInsideCloud / float(SAMPLE_NUM);
    float travelled = 0.0f;
    float transmittance = 1.0f;

    for(int i = 0; i < SAMPLE_NUM; i++) {
        if(travelled < dstLimit){
            glm::vec3 testPoint = startPos + viewDir * travelled;
            glm::vec3 light = rayMarchToLight(param, testPoint, lightDir);
            float density = sampleNoise(testPoint);

            sumLight += light * density * stepSize * transmittance * phaseVal;
            transmittance *= std::exp(-density * stepSize);

            if(transmittance < 0.01f) break;
        }
        travelled += stepSize;
    }

    return glm::vec4(sumLight, transmittance);
}

int main() {
    // 初始化系统
    WindowManager app(1920, 1680);

    // 搭建场景
    Scene scene(&app.camera);
    // 太阳光
    glm::vec3 Sun = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
    DirectionalLight directionalLight(Sun, glm::vec3(1.0f), 1.0f);
    scene.addLight(std::make_shared<DirectionalLight>(directionalLight));
    // 模型
    Model plane = Model::createPlane(100, 1);
    scene.loadHDRAndIBL("assets/HDR/2.hdr");
    scene.addModel(plane);
    scene.addDefaultModel("helmet");

    // 管线
    RenderPipeline renderPipeline;
    renderPipeline.setupDeferredRenderPipeline(scene);

    // GUI
    ImGUIManger imGUIManger(scene, &renderPipeline);

    // 实验
    // glm::vec4 pos = {1,3,4,0};
    // auto transtion = glm::mat4(1.0);
    // std::cout << glm::to_string(app.camera.getProjectionMatrix()*app.camera.getViewMatrix()*transtion*pos) << std::endl;
    AtmosphereParameter a = AtmosphereParameter(
    8000,   // RayleighScatteringScalarHeight
    1200,   // MieScatteringScalarHeight
    0.8,     // MieAnisotropy
    25000,  // OzoneLevelCenterHeight
    15000,  // OzoneLevelWidth
    6360000,// PlanetRadius
    60000,  // AtmosphereHeight
    2000, // CloudStartHeight
    5000,   // CloudEndHeight
    glm::vec3(0,0,0)
    );

    glm::vec3 camPos(0.0f, 6360000.0f + 100.0f, 0.0f); // 相机在地表上方1km
    glm::vec3 viewDir = glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)); // 向上
    glm::vec3 worldPos = camPos + viewDir;
    glm::vec3 lightPos(1000000.0f, 1000000.0f, 1000000.0f); // 测试光源位置

    glm::vec4 cloud = cloudRayMarching(a, worldPos, viewDir, 100.0f, lightPos);

    std::cout << "Cloud color: " << cloud.x << ", " << cloud.y << ", " << cloud.z
              << "  transmittance: " << cloud.w << std::endl;



    // 渲染
    while (!glfwWindowShouldClose(app.window)) {
        app.processInput();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // --- 渲染 Pipeline---
        renderPipeline.render();

        // --- 渲染 ImGui ---
        imGUIManger.render();

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    // 结束
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
