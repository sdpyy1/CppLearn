#version 330 core

// 输入从顶点着色器传过来的数据
in vec2 TexCoords;
in vec3 FragPos;

in vec3 T;
in vec3 B;
in vec3 N;

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
layout (location = 4) out vec4 gEmission;
vec3 getNormalFromMap()
{
    vec3 n = normalize(N);
    vec3 t = normalize(T - dot(T, n) * n);
    vec3 b = normalize(cross(n, t));

    mat3 TBN = mat3(t, b, n);
    vec3 normalTS = texture(texture_normal, TexCoords).rgb;
    normalTS = normalTS * 2.0 - 1.0;
    return normalize(TBN * normalTS);
}
void main()
{
    // 位置
    gPosition = FragPos;

    // 法线贴图
    gNormal = getNormalFromMap();  // 切线空间转为全局空间

    // Albedo
    vec3 albedo = texture(texture_albedo, TexCoords).rgb;
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0; // 预留透明度通道

    // Metallic, Roughness, AO, Emission
    float metallic = texture(texture_metallic, TexCoords).r;
    float roughness = texture(texture_roughness, TexCoords).r;
    float ao = texture(texture_ao, TexCoords).r;
    vec3 emission = texture(texture_emission, TexCoords).rgb;


    gMaterial = vec4(metallic, roughness, ao, 1.0);
    gEmission = vec4(emission, 1.0);
}
