#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform mat4 projection;
uniform mat4 view;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmission;
uniform sampler2D gDepth;
uniform sampler2D shadowMap;
uniform sampler2D lightTexture;
// SSR
uniform int EnableSSR;
uniform int totalStepTimes;
uniform float stepSize;
uniform float EPS;
uniform float threshold;
uniform float SSRStrength;

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


bool RayMarch(vec3 ori, vec3 dir, out vec3 hitPos) {
    if(EnableSSR == 0){
        return false;
    }
    bool result = false;
    bool firstIn = false;
    vec3 curPos = ori;
    float stepSzieInnder = stepSize;
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
    
void main() {
    vec3 lightColor = texture(lightTexture, TexCoords).rgb;
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 albedo     = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    float roughness = texture(gMaterial, TexCoords).g;

    vec3 N = texture(gNormal, TexCoords).rgb;   // 法线
    vec3 V = normalize(camPos - WorldPos); // 视线方向
    vec3 R = normalize(reflect(-V, N)); // 反射方向

    vec3 finalColor = lightColor;
    float roughnessThreshold = 0.3;
    // SSR计算光滑表面镜面反射
    if(roughness < roughnessThreshold){
        bool hit = false;
        vec3 ssrColor = vec3(0.0);
        vec3 hitPos;
        hit = RayMarch(WorldPos, R, hitPos);
        if(hit) {
            vec2 uvHit = GetScreenCoordinate(hitPos);
            // 从之前的渲染结果中采样反射颜色
            ssrColor = pow(texture(lightTexture, uvHit).rgb, vec3(2.2));
            // 混合SSR颜色和IBL
            // 混合SSR颜色和IBL
            float fade = smoothstep(0.0, 0.05, uvHit.x) * smoothstep(0.0, 0.05, uvHit.y) *
            smoothstep(0.0, 0.95, 1.0 - uvHit.x) * smoothstep(0.0, 0.95, 1.0 - uvHit.y);

            // 根据粗糙度动态调整混合权重，粗糙度越高权重越低
            float roughnessFactor = 1.0 - smoothstep(0.0, roughnessThreshold, roughness);
            float mixWeight = SSRStrength * fade * roughnessFactor;

            finalColor = mix(finalColor, ssrColor, mixWeight);
        }
    }
    FragColor = vec4(finalColor, 1.0);
}
