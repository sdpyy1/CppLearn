//
// Created by Administrator on 2025/7/23.
//

#include <glm/gtc/type_ptr.hpp>
#include "ImGUIManger.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

void ImGUIManger::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug UI", nullptr, ImGuiWindowFlags_NoMove);
    // 得先执行一次，不然没背景
    scene.loadHDRAndIBL(std::string("assets/HDR/1.hdr"));

    if (ImGui::CollapsingHeader("Scene Settings")) {
        renderIBLSelectionUI();
        renderShadowSetting();
    }

    if (ImGui::CollapsingHeader("Model")) {
        renderAddModelUI();
        renderModelListUI();
        renderModelTransformUI();
    }

    if (ImGui::CollapsingHeader("Light")) {
        renderLightListUI();
        renderLightProperties();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void ImGUIManger::renderShadowSetting(){
    static const char* shadowTypes[] = { "No Shadow", "Hard Shadow", "PCF", "PCSS" };
    ImGui::Text("Shadow Type:");
    ImGui::Combo("##ShadowTypeCombo", &scene.shadowType, shadowTypes, IM_ARRAYSIZE(shadowTypes));

    if (scene.shadowType == 2) {
        ImGui::SliderInt("PCF Scope", &scene.pcfScope, 1, 10);
    }
    else if (scene.shadowType == 3) {
        ImGui::SliderFloat("Blocker Search Radius", &scene.PCSSBlockerSearchRadius, 1.0f, 5.0f, "%.1f");
        ImGui::SliderFloat("Penumbra Scale", &scene.PCSSScale, 0.001f, 0.05f, "%.4f");
        ImGui::SliderFloat("Max Blur Kernel", &scene.PCSSKernelMax, 1.0f, 20.0f, "%.1f");
    }
}
void ImGUIManger::renderModelTransformUI() {
    if (scene.selModel) {
        static bool uniformScale = true;
        static float uniformScaleValue = 1.0f;

        ImGui::Separator();
        ImGui::Text(scene.selModel->directory.c_str());
        // 位置控制
        ImGui::DragFloat3("position", glm::value_ptr(scene.selModel->translation), 0.1f);
        // 旋转控制
        ImGui::DragFloat3("rotation", glm::value_ptr(scene.selModel->rotation), 1.0f);
        // 缩放控制
        ImGui::Text("scale");
        ImGui::SameLine();
        ImGui::Checkbox("lockScale", &uniformScale);
        if (uniformScale) {
            if (ImGui::DragFloat("scale ratio", &uniformScaleValue, 0.01f, 0.01f, 10.0f)) {
                scene.selModel->scale = glm::vec3(uniformScaleValue);
            }
        } else {
            if (ImGui::DragFloat3("scale", glm::value_ptr(scene.selModel->scale), 0.01f, 0.01f, 10.0f)) {
                uniformScaleValue = scene.selModel->scale.x;
            }
        }
    }

}

void ImGUIManger::renderModelListUI() {
    ImGui::Checkbox("Enable Outline", &scene.enableOutline);

    if (ImGui::BeginListBox("##ModelList", ImVec2(-FLT_MIN, 0))) {
        for (int i = 0; i < scene.models.size(); ++i) {
            auto& model = scene.models[i];
            std::string label = model.directory + std::to_string(i);

            bool is_selected = (scene.selModel == &model);
            if (ImGui::Selectable(label.c_str(), is_selected)) {
                scene.selModel = &model;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }
}

std::string getLightTypeName(const Light& light) {
    if (dynamic_cast<const DirectionalLight*>(&light)) return "Directional";
    if (dynamic_cast<const PointLight*>(&light)) return "Point";
    if (dynamic_cast<const SpotLight*>(&light)) return "Spot";
    return "Unknown";
}
void ImGUIManger::renderLightListUI() {
    ImGui::Checkbox("Draw Light Cubes", &scene.drawLightCube);

    if (ImGui::BeginListBox("##LightList", ImVec2(-FLT_MIN, 0))) {
        for (int i = 0; i < scene.lights.size(); ++i) {
            const auto& light = scene.lights[i];
            std::string label = "Light " + std::to_string(i);

            bool is_selected = (scene.selLight == light);
            if (ImGui::Selectable(label.c_str(), is_selected)) {
                scene.selLight = light;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }
}
void ImGUIManger::renderLightProperties() {
    // Edit currently selected light
    if (scene.selLight) {
        ImGui::Separator();
        ImGui::Text("Light Properties");

        // 显示光源类型
        ImGui::Text("Type: %s", getLightTypeName(*scene.selLight).c_str());

        ImGui::ColorEdit3("Color", glm::value_ptr(scene.selLight->color));
        ImGui::DragFloat("Intensity", &scene.selLight->intensity, 0.01f, 0.0f, 10.0f);

        // Specific properties by light type
        if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(scene.selLight)) {
            ImGui::DragFloat3("Direction", glm::value_ptr(dirLight->position), 0.1f);
        }
        else if (auto pointLight = std::dynamic_pointer_cast<PointLight>(scene.selLight)) {
            ImGui::DragFloat3("Position", glm::value_ptr(pointLight->position), 0.1f);
        }
        else if (auto spotLight = std::dynamic_pointer_cast<SpotLight>(scene.selLight)) {
            ImGui::DragFloat3("Position", glm::value_ptr(spotLight->position), 0.1f);
            ImGui::DragFloat3("Direction", glm::value_ptr(spotLight->direction), 0.1f);
            float cutOffDeg = glm::degrees(acosf(spotLight->cutOff));
            float outerCutOffDeg = glm::degrees(acosf(spotLight->outerCutOff));
            if (ImGui::SliderFloat("Inner Angle", &cutOffDeg, 0.0f, 90.0f)) {
                spotLight->cutOff = glm::cos(glm::radians(cutOffDeg));
            }
            if (ImGui::SliderFloat("Outer Angle", &outerCutOffDeg, 0.0f, 90.0f)) {
                spotLight->outerCutOff = glm::cos(glm::radians(outerCutOffDeg));
            }
        }
    }
}

void ImGUIManger::renderAddModelUI() {
    static int currentModelIndex = 0;
    const char* modelNames[] = { "helmet", "gun", "gaoda" };

    ImGui::Text("Add Default Model:");
    if (ImGui::BeginCombo("DefaultModel", modelNames[currentModelIndex])) {
        for (int n = 0; n < IM_ARRAYSIZE(modelNames); n++) {
            bool is_selected = (currentModelIndex == n);
            if (ImGui::Selectable(modelNames[n], is_selected)) {
                currentModelIndex = n;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Add Model##AddModelBtn")) {
        scene.addDefaultModel(modelNames[currentModelIndex]);
    }
}

void ImGUIManger::renderIBLSelectionUI() {
    static int currentHDRIndex = 1;
    const char* hdrFiles[] = { "No IBL","1.hdr", "2.hdr", "3.hdr", "4.hdr" };


    ImGui::Text("Select Environment HDR:");

    if (ImGui::BeginCombo("HDR Cubemap", hdrFiles[currentHDRIndex])) {
        for (int n = 0; n < IM_ARRAYSIZE(hdrFiles); n++) {
            bool is_selected = (currentHDRIndex == n);
            if (ImGui::Selectable(hdrFiles[n], is_selected)) {
                currentHDRIndex = n;
                if (currentHDRIndex == 0) {
                    scene.disableIBL();
                } else {
                    scene.loadHDRAndIBL(std::string("assets/HDR/") + hdrFiles[n]);
                }
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}