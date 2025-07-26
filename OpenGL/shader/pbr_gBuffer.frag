#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform mat4 projection;
uniform mat4 view;

// G-buffer 纹理输入
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmission;
uniform sampler2D gDepth;
uniform sampler2D shadowMap;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D lutMap;

// 阴影
uniform mat4 lightSpaceMatrix;
uniform int shadowType;
uniform int pcfScope;
uniform float PCSSBlockerSearchRadius;
uniform float PCSSScale;
uniform float PCSSKernelMax;

// SSR
uniform int EnableSSR;
uniform int totalStepTimes;
uniform float stepSize;
uniform float EPS;
uniform float threshold;
uniform float SSRStrength;

const float PI = 3.14159265359;

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
        int range = pcfScope;
        int samples = 0;

        for (int x = -range; x <= range; ++x) {
            for (int y = -range; y <= range; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
                samples++;
            }
        }
        shadow /= float(samples);
        return shadow;
    }
    else if (shadowType == 3) {
        float avgBlockerDepth = 0.0;
        int blockers = 0;

        ivec2 texSize = textureSize(shadowMap, 0);
        vec2 texelSize = 1.0 / vec2(texSize);

        int searchRadius = int(PCSSBlockerSearchRadius);
        for (int x = -searchRadius; x <= searchRadius; ++x) {
            for (int y = -searchRadius; y <= searchRadius; ++y) {
                float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                if (sampleDepth < currentDepth - bias) {
                    avgBlockerDepth += sampleDepth;
                    blockers++;
                }
            }
        }
        if (blockers == 0) return 0.0;
        avgBlockerDepth /= blockers;
        float penumbra = (currentDepth - avgBlockerDepth) * PCSSScale;
        int kernel = int(clamp(penumbra * float(texSize.x), 1.0, PCSSKernelMax));
        float shadow = 0.0;
        for (int x = -kernel; x <= kernel; ++x) {
            for (int y = -kernel; y <= kernel; ++y) {
                float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += (currentDepth - bias > sampleDepth) ? 1.0 : 0.0;
            }
        }
        shadow /= float((2 * kernel + 1) * (2 * kernel + 1));
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
vec4 Project(vec4 a) {
    return a / a.w;
}
vec2 GetScreenCoordinate(vec3 posWorld) {
    vec2 uv = Project(projection*view * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
    return uv;
}
float GetDepth(vec3 posWorld) {
    vec4 clipSpacePos = projection * view * vec4(posWorld, 1.0);
    float ndcZ = clipSpacePos.z / clipSpacePos.w;   // z in [-1, 1]
    float depth = ndcZ * 0.5 + 0.5;                  // map to [0, 1]
    return depth;
}
float GetGBufferDepth(vec2 uv) {
    float depth = texture2D(gDepth, uv).x;
    if (depth < 1e-2) {
        depth = 1000.0;
    }
    return depth;
}
float LinearizeDepth(float d) {
    float nearPlane = 0.1;
    float farPlane = 100;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}
// RayMarch函数，根据你给的代码写
bool RayMarch(vec3 ori, vec3 dir, out vec3 hitPos) {
    if(EnableSSR == 0){
        return false;
    }
    bool result = false;
    bool firstIn = false;
    vec3 curPos = ori;
    float stepSzieInnder = stepSize;  // uniform变量不能在shader中修改
    vec3 nextPos;
    for(int i = 0; i < totalStepTimes; i++) {
        // 步进
        nextPos = curPos + dir * stepSzieInnder;
        // 获取步进后的空间坐标对应的uv坐标
        vec2 uvScreen = GetScreenCoordinate(nextPos);
        // 超出屏幕，直接返回
        if(any(lessThan(uvScreen, vec2(0.0))) || any(greaterThan(uvScreen, vec2(1.0)))) break;
        // 没有碰到物体
        if(LinearizeDepth(GetDepth(nextPos)) < LinearizeDepth(GetGBufferDepth(GetScreenCoordinate(nextPos)))){
            curPos += dir * stepSzieInnder;
            if(firstIn) stepSzieInnder *= 0.5;
            continue;
        }
        firstIn = true;
        if(stepSzieInnder < EPS){
            float s1 = LinearizeDepth(GetGBufferDepth(GetScreenCoordinate(curPos))) - LinearizeDepth(GetDepth(curPos)) + EPS;
            float s2 = LinearizeDepth(GetDepth(nextPos)) - LinearizeDepth(GetGBufferDepth(GetScreenCoordinate(nextPos))) + EPS;
            if(s1 < threshold && s2 < threshold){
                hitPos = curPos + 2.0 * dir * stepSzieInnder * s1 / (s1 + s2);
                result = true;
            }
            break;
        }
        if(firstIn) stepSzieInnder *= 0.5;
    }
    return result;
}


void main()
{
    // 纹理采样
    vec3 albedo     = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(gMaterial, TexCoords).r;
    float roughness = texture(gMaterial, TexCoords).g;
    float ao        = texture(gMaterial, TexCoords).b;
    vec3 emission     = pow(texture(gEmission, TexCoords).rgb, vec3(2.2));
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;

    // 参数准备
    vec3 N = texture(gNormal, TexCoords).rgb;   // 法线
    vec3 V = normalize(camPos - WorldPos); // 视线方向
    vec3 R = normalize(reflect(-V, N)); // 反射方向
    vec3 F0 = mix(vec3(0.04), albedo, metallic); // 菲涅尔反射 F0 = 0.04 * (1-metalic) + albedo * metalic   // 越金属，F0越像它的albedo
    // vec3 L = normalize(lightPos - WorldPos); // 光源方向   点光源
    vec3 L = normalize(-lightPos); // 光源方向  平行光
    vec3 H = normalize(V + L); // 半程向量

    // BRDF项
    float NDF = DistributionGGX(N, H, roughness);  // 粗糙度来决定NDF，粗糙度越小，法线越集中在H
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001);

    // 镜面反射占比
    vec3 kS = F;
    // 漫反射占比
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    // cos项
    float NdotL = max(dot(N, L), 0.0);


    // 直接光照计算
    vec3 Lo = (kD * albedo / PI + specular) * lightColor * NdotL * (1-ShadowCalculation(WorldPos, N));


    // IBL 环境光
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    F        = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec2 lut = texture(lutMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * lut.x + lut.y);
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo / PI;  // learnOpenGL中并没有 /PI

    //TODO： 这部分应该移动到后处理Pass
    // SSR计算光滑表面镜面反射
    bool hit = false;
    vec3 ssrColor = vec3(0.0);
    vec3 hitPos;
    vec3 specularFinal = specularIBL;
    hit = RayMarch(WorldPos, R, hitPos);
    if(hit) {
        vec2 uvHit = GetScreenCoordinate(hitPos);
        // 从之前的渲染结果中采样反射颜色 TODO： 移动到后处理阶段后使用最终颜色来反射
        ssrColor = pow(texture(gAlbedo, uvHit).rgb, vec3(2.2));
        // 混合SSR颜色和IBL
        float fade = smoothstep(0.0, 0.05, uvHit.x) * smoothstep(0.0, 0.05, uvHit.y) *
        smoothstep(0.0, 0.95, 1.0 - uvHit.x) * smoothstep(0.0, 0.95, 1.0 - uvHit.y);
        float mixWeight = SSRStrength * fade;
        specularFinal = mix(specularIBL, ssrColor, mixWeight);
    }

    vec3 ambient = (kD * diffuse + specularFinal) * ao;

    // 光照相加
    vec3 color = ambient + Lo + emission;

    // toneMapping
//    color = color / (color + vec3(1.0));
    color = ACESFilmToneMapping(color)  ;

    // 伽马矫正
    color = pow(color, vec3(1.0 / 2.2));

    // 不写入深度，会让天空盒盖住屏幕
    gl_FragDepth = texture(gDepth, TexCoords).r;

    FragColor =  vec4(color, 1.0);
}