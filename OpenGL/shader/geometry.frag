#version 330 core

// 输入从顶点着色器传过来的数据
in vec2 TexCoords;
in vec3 FragPos;    // 世界空间位置
in vec3 Normal;     // 世界空间法线

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

    // 法线，需从法线贴图转换为世界空间法线（这里假设传入的Normal已是世界空间）
    // 如果用法线贴图，则要转换，示例中假设Normal就是世界空间法线
    vec3 normalMap = texture(texture_normal, TexCoords).rgb;
    normalMap = normalMap * 2.0 - 1.0; // 从[0,1]映射到[-1,1]
    // 这里要用TBN矩阵转换法线贴图法线到世界空间，省略示范，直接用normalMap或Normal
    gNormal = normalize(normalMap);  // 或者normalize(Normal)

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
