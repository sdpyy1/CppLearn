#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform mat4 projection;
uniform mat4 view;
uniform sampler2D gPosition;
uniform sampler2D gDepth;
uniform sampler2D preTexture;
uniform samplerCube environmentMap;
uniform int showSkyBox;
// 3D噪声图
uniform sampler3D noiseTexture;
// AABB盒
vec3 cloudBoxMin = vec3(20.0, 30.0, 20.0);
vec3 cloudBoxMax = vec3(-20.0, 26.0, -20.0);
//vec3 cloudBoxMin = vec3(-3.0, -3.0, -3.0);
//vec3 cloudBoxMax = vec3(3.0, 3.0, 3.0);

float LinearizeDepth(float d) {
    float nearPlane = 0.1;
    float farPlane = 100;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}
//
//// 返回射线进入时间和在盒内的持续时间
//vec2 rayBoxDst(vec3 origin, vec3 invDirection, vec3 boxMin, vec3 boxMax){
//    vec3 t0 = (boxMin - origin) * invDirection;
//    vec3 t1 = (boxMax - origin) * invDirection;
//    // 每个轴上的进入时间和离开时间
//    vec3 tmin = min(t0, t1);
//    vec3 tmax = max(t0, t1);
//    // 求三个轴的交集，就可以确定具体的进入和离开的交点到达时间
//    float tIn = max(max(tmin.x,tmin.y),tmin.z);
//    float tOut = min(min(tmax.x,tmax.y),tmax.z);
//    return vec2(max(0, tIn), max(0, tOut-max(0,tIn)));
//}
//float sampleNoise(vec3 testPoint)
//{
//    vec3 regionSize = cloudBoxMax - cloudBoxMin;
//    vec3 uvw = (testPoint - cloudBoxMin) / regionSize;
//    return texture(noiseTexture, uvw).x;
//}
//float Beer(float opticalDepth) {
//    // σ_t 是消光系数，这里取常数
//    const float extinction = 0.1;
//    return exp(-opticalDepth * extinction);
//}
//float BeerPowder(float opticalDepth) {
//    const float powderStrength = 2.0; // 适当调整
//    return 1.0 - exp(-opticalDepth * powderStrength);
//}
//// 计算一个采样点的光照
//vec3 GetCurrentPositionLum(vec3 currentPos, vec3 dirToLight)
//{
//    // 1. 避免除零
//    vec3 invLightDir = vec3(
//    abs(dirToLight.x) < 1e-5 ? 1e5 : 1.0 / dirToLight.x,
//    abs(dirToLight.y) < 1e-5 ? 1e5 : 1.0 / dirToLight.y,
//    abs(dirToLight.z) < 1e-5 ? 1e5 : 1.0 / dirToLight.z
//    );
//
//    // 2. 算出光线从当前点到云包围盒边界的距离
//    float dstInsideBox = rayBoxDst(currentPos, invLightDir, cloudBoxMin, cloudBoxMax).y;
//
//    // 3. 每步的长度（这里 10 可以调，越小越精细）
//    float stepSize = dstInsideBox / 10.0;
//
//    // 4. 累计密度
//    float totalDensity = 0.0;
//    for (int i = 0; i <= 10; i++)
//    {
//        currentPos += dirToLight * stepSize;
//        totalDensity += max(0.0, sampleNoise(currentPos) * stepSize);
//    }
//
//    // 5. 计算透光率（Beer-Lambert 定律）
//    float transmittance = exp(-totalDensity * 1);
//    // 光吸收系数：数值越大，云层越容易变暗
//    float lightAbsorptionTowardSun = 1.5;
//    // 中亮颜色：云中等亮度部分的颜色（暖白偏灰）
//    vec3 colA = vec3(0.85, 0.82, 0.78);
//    // 暗部颜色：云阴影部分（冷灰偏蓝）
//    vec3 colB = vec3(0.35, 0.38, 0.45);
//    // 太阳光颜色（暖色调白光）
//    vec3 lightColor = vec3(1.0, 0.96, 0.92);
//    // 明亮段映射系数：越大亮部越宽
//    float colorOffset1 = 1.0;
//    // 暗部映射系数：越大暗部范围越窄
//    float colorOffset2 = 1.2;
//    // 最低亮度阈值：防止暗部完全黑
//    float darknessThreshold = 0.2;
//    // 三段颜色映射
//    vec3 cloudColor = mix(colA, lightColor, clamp(transmittance * colorOffset1, 0.0, 1.0));
//    cloudColor = mix(colB, cloudColor, clamp(pow(transmittance * colorOffset2, 3.0), 0.0, 1.0));
//
//    // 加上暗部亮度阈值
//    return darknessThreshold + transmittance * (1.0 - darknessThreshold) * cloudColor;
//}
//
//
//
//float HenyeyGreenstein(float cosTheta, float g) {
//    float g2 = g * g;
//    return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
//}
//// 一条射线穿越云层次数越多，表示这个角度云层越厚
//vec2 cloudRayMarching(vec3 worldPos, vec3 direction, float depth)
//{
//    vec3 safeInvDir = vec3(
//    abs(direction.x) < 1e-5 ? 1e5 : 1.0 / direction.x,
//    abs(direction.y) < 1e-5 ? 1e5 : 1.0 / direction.y,
//    abs(direction.z) < 1e-5 ? 1e5 : 1.0 / direction.z
//    );
//    vec2 rayBoxInfo = rayBoxDst(camPos, safeInvDir, cloudBoxMin, cloudBoxMax);
//    // 相机距离云的距离
//    float dstToBox = rayBoxInfo.x;
//    // 相机在云内部的穿行距离
//    float dstInsideBox = rayBoxInfo.y;
//    // 图1取第一项，图二取第一项但是为负数（所以当dstLimit为负数表示云彻底被遮挡），图三也是第一项， 所以取第二项的情况就是没有遮挡的情况
//    float dstLimit = min(depth - dstToBox, dstInsideBox);  // 步进距离限制
//    // 视线被模型遮挡，没有云参与计算
//    if (dstLimit <= 0.0) return vec2(0.0,1);
//    // 传入的lightPos是光照方向，不是光照位置
//    vec3 lightDir = -lightPos;
//
//    float phase = HenyeyGreenstein(dot(direction, lightDir), 0.6);
//    // 从AABB盒表面开始RM
//    vec3 startPos = camPos + direction * dstToBox;
//
//    float sumDensity = 0.0;
//    float stepSize = 0.5;
//    // 已步进距离（时间）
//    float travelled = 0.0;
//    //总光照
//    float totalLum = 0;
//    for (int i = 0; i < 32; i++) {
//        if(travelled < dstLimit){
//            // 测试点
//            vec3 testPoint = startPos + (direction * travelled);
//            // 采样密度
//            float density = sampleNoise(testPoint);
//            if(density > 0.0){
//                // 采样点的光照
//                vec3 light = GetCurrentPositionLum(testPoint,-lightPos);
//                // 计算当前步进区间内的总光照
//                totalLum += lightTransmittance * stepSize * sumDensity;
//                sumDensity *= exp(-density * stepSize);
//                if (sumDensity < 0.01) break;
//            }
//        }
//
//        travelled += stepSize;
//    }
//    return vec2(clamp(sumDensity, 0.0, 1.0),totalLum);
//}
vec3 reconstructWorldPos(vec2 uv) {
    mat4 inverseProjection = inverse(projection);
    mat4 inverseView = inverse(view);
    // Step 1: UV → NDC
    vec2 ndc = uv * 2.0 - 1.0;
    // Step 2: 深度值 → 裁剪空间
    float depth = texture(gDepth, uv).r;
    vec4 clipPos = vec4(ndc, depth * 2.0 - 1.0, 1.0);
    // Step 3: 裁剪空间 → 观察空间
    vec4 viewPos = inverseProjection * clipPos;
    viewPos /= viewPos.w;
    // Step 4: 观察空间 → 世界坐标
    return (inverseView * vec4(viewPos.xyz, 1.0)).xyz;
}
void main() {
    // 世界坐标 （这里也可以从gBuffer的位置贴图获取，但是没有模型的地方就会出错）
    vec3 worldPos = reconstructWorldPos(TexCoords);
    // raymarching方向
    vec3 worldViewDir = normalize(worldPos - camPos);
//    // 像素zBuffer深度
//    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
//    vec3 preColor = texture(preTexture, TexCoords).rgb;
//    if(depth > 99 && showSkyBox == 1) {
//        preColor = texture(environmentMap, worldViewDir).rgb;
//    }
//    // 计算的是当前着色点云的密度
//    vec2 cloudInfo = cloudRayMarching(worldPos,worldViewDir,depth);
//    float alpha = cloudInfo.x;
//    vec3 cloudColor = cloudInfo.y * lightColor;
//    FragColor = vec4(mix(preColor, cloudColor, alpha), 1);


    // TODO:暂时
    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
    vec3 preColor;
    if(depth > 99 && showSkyBox == 1) {
        preColor = texture(environmentMap, worldViewDir).rgb;
    }else{
        preColor = texture(preTexture, TexCoords).rgb;
    }
    FragColor = vec4(preColor, 1);
}
