#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

// G-buffer 纹理输入
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmission;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

// 摄像机位置
uniform vec3 viewPos;

// 光源定义（最多支持 16 个）
#define MAX_LIGHTS 16
struct Light {
    vec3 position;
    vec3 color;
};

uniform vec3 lightPos;
uniform vec3 lightColor;
float ShadowCalculation(vec3 fragPosWorldSpace, vec3 Normal,vec3 lightDir)
{
    // 1. 将世界坐标转换到光源空间
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPosWorldSpace, 1.0);

    // 2. 做齐次除法 [-1, 1] → [0, 1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 3. 从 shadowMap 采样深度
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // 4. 阴影偏移（防止阴影伪影/阴影彼得潘效应）
    float bias = max(0.005 * (1.0 - dot(Normal, lightDir)), 0.001);

    // 5. 判断是否在阴影中
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // 如果在光照范围外直接不加光照（防止超出光照空间出错）
    if (projCoords.z > 1.0)
    shadow = 0.0;

    return shadow;
}

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

    // 方向光设置（注意 lightPos 是方向而不是位置）
    vec3 lightDir = normalize(-lightPos);
    vec3 viewDir  = normalize(viewPos - FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // 阴影判断
    float shadow = ShadowCalculation(FragPos, Normal, lightDir);

    // 漫反射和高光
    float diff = max(dot(Normal, lightDir), 0.0);
    float spec = pow(max(dot(Normal, halfway), 0.0), 32.0) * 0.1;

    // 光照结果（加入阴影遮蔽）
    vec3 lightEffect = (Albedo * diff + vec3(spec)) * (1.0 - shadow) * lightColor;

    // 综合其他效果
    vec3 result = lightEffect;
    result *= ao;
    result += Albedo * emission;

    FragColor = vec4(result, 1.0);
}
