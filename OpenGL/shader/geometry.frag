#version 330 core

// 输入从顶点着色器传过来的数据
in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;

// 纹理采样器
uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;
uniform sampler2D texture_metallic;
uniform sampler2D texture_roughness;
uniform sampler2D texture_ao;
uniform sampler2D texture_emission;

// 输出到多个颜色附件
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gMaterial;

void main()
{
    // 位置
    gPosition = FragPos;

    // 法线贴图
    vec3 normalTS = texture(texture_normal, TexCoords).rgb;
    normalTS = normalTS * 2.0 - 1.0; // 转换为 [-1,1]
    gNormal = normalize(TBN * normalTS);  // 切线空间转为全局空间

    // Albedo
    vec3 albedo = texture(texture_albedo, TexCoords).rgb;
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0; // 预留透明度通道

    // Metallic, Roughness, AO, Emission
    float metallic = texture(texture_metallic, TexCoords).r;
    float roughness = texture(texture_roughness, TexCoords).r;
    float ao = texture(texture_ao, TexCoords).r;
    float emission = texture(texture_emission, TexCoords).r;


    gMaterial = vec4(metallic, roughness, ao, emission);
}
