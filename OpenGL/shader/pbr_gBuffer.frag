#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
// G-buffer 纹理输入
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmission;
uniform sampler2D gDepth;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform int shadowType;
// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D lutMap;


const float PI = 3.14159265359;
#define MAX_LIGHTS 16
struct Light {
    vec3 position;
    vec3 color;
};
//uniform int lightCount;
//uniform Light lights[MAX_LIGHTS];


float ShadowCalculation(vec3 fragPosWorld, vec3 normal) {
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPosWorld, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, normalize(lightPos))), 0.001);

    // shadow type switching
    if (shadowType == 0) {
        return 0.0; // no shadow
    }
    else if (shadowType == 1) {
        return (currentDepth - bias > closestDepth) ? 1.0 : 0.0; // hard shadow
    }
    else if (shadowType == 2) {
        // --- PCF ---
        float shadow = 0.0;
        ivec2 texSize = textureSize(shadowMap, 0);
        vec2 texelSize = 1.0 /vec2(texSize);
        for (int x = -5; x <= 5; ++x) {
            for (int y = -5; y <= 5; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
            }
        }
        shadow /= 100.0;
        return shadow;
    }
    else if (shadowType == 3) {
        // --- PCSS (略简化版) ---
        // Blocker search
        float avgBlockerDepth = 0.0;
        int blockers = 0;
        ivec2 texSize = textureSize(shadowMap, 0);

        vec2 texelSize = 1.0 / vec2(texSize);
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                if (sampleDepth < currentDepth - bias) {
                    avgBlockerDepth += sampleDepth;
                    blockers++;
                }
            }
        }

        float shadow = 0.0;
        if (blockers > 0) {
            avgBlockerDepth /= blockers;
            float penumbra = (currentDepth - avgBlockerDepth) * 50.0; // scale factor
            int kernel = clamp(int(penumbra), 1, 5);

            for (int x = -kernel; x <= kernel; ++x) {
                for (int y = -kernel; y <= kernel; ++y) {
                    float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                    shadow += (currentDepth - bias > sampleDepth) ? 1.0 : 0.0;
                }
            }
            shadow /= float((2 * kernel + 1) * (2 * kernel + 1));
        }
        return shadow;
    }

    return 0.0;
}


// GGX NDF
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Geometry: Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel: Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFilmToneMapping(vec3 color)
{
    // 适当的曝光缩放，可以视为手动曝光调整（可调参数）
    color *= 0.6;

    // ACES tone mapping 曲线
    color = RRTAndODTFit(color);

    // Clamp 到 [0, 1]
    return clamp(color, 0.0, 1.0);
}
void main()
{
    // 纹理采样
    vec3 albedo     = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(gMaterial, TexCoords).r;
    float roughness = texture(gMaterial, TexCoords).g;
    float ao        = texture(gMaterial, TexCoords).b;
    vec3 emission     = pow(texture(gEmission, TexCoords).rgb, vec3(2.2)); // gamma correct
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;



    // 参数准备
    vec3 N = texture(gNormal, TexCoords).rgb;   // 法线
    vec3 V = normalize(camPos - WorldPos); // 视线方向
    vec3 R = reflect(-V, N); // 反射方向
    vec3 F0 = mix(vec3(0.04), albedo, metallic); // 菲涅尔反射 F0
//    vec3 L = normalize(lightPos - WorldPos); // 光源方向   点光源
    vec3 L = normalize(-lightPos); // 光源方向  平行光
    vec3 H = normalize(V + L); // 半程向量

    // 光照部分
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = lightColor;
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL * (1-ShadowCalculation(WorldPos, N));
//    vec3 Lo = (kD * albedo / PI + specular) * radiance* NdotL;


    // IBL ambient
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    F        = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec2 lut = texture(lutMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * lut.x + lut.y);
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo / PI;  // learnOpenGL中并没有 /PI
    vec3 ambient = (kD * diffuse + specularIBL) * ao;


    // 光照相加
    vec3 color = ambient + Lo + emission;
    // toneMapping
    color = color / (color + vec3(1.0));
//    color = ACESFilmToneMapping(color)  ;

    // 伽马矫正
    color = pow(color, vec3(1.0 / 2.2));
//    FragColor = vec4(color, 1.0);


    gl_FragDepth = texture(gDepth, TexCoords).r;

    FragColor =  vec4(color, 1.0);
}