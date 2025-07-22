#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

// G-buffer 纹理输入
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmission;

// 摄像机位置
uniform vec3 viewPos;

// 光源定义（最多支持 16 个）
#define MAX_LIGHTS 16
struct Light {
    vec3 position;
    vec3 color;
};

uniform int lightCount;
uniform Light lights[MAX_LIGHTS];

void main() {
    // 从 G-Buffer 获取数据
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal  = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo  = texture(gAlbedo, TexCoords).rgb;

    vec4 material = texture(gMaterial, TexCoords);
    float metallic = material.r;
    float roughness = material.g;
    float ao = material.b;
    vec3 emission = texture(gEmission, TexCoords).rgb;

    // 光照累加器
    vec3 result = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        vec3 viewDir  = normalize(viewPos - FragPos);
        vec3 halfway  = normalize(lightDir + viewDir);

        // 漫反射项
        float diff = max(dot(Normal, lightDir), 0.0);

        // 高光项（Blinn-Phong）
        float spec = pow(max(dot(Normal, halfway), 0.0), 32.0);

        // 最终颜色计算
        vec3 lightColor = lights[i].color;
        result += (Albedo * diff + vec3(spec)) * lightColor;
    }

    // AO 和 Emission 简单加入
    result *= ao;
    result += Albedo * emission;

    FragColor = vec4(result, 1.0);
}
