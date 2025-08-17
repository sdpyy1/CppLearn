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
vec3 cloudBoxMin = vec3(20.0, 6.0, 20.0);
vec3 cloudBoxMax = vec3(-20.0, 4.0, -20.0);
//vec3 cloudBoxMin = vec3(-3.0, -3.0, -3.0);
//vec3 cloudBoxMax = vec3(3.0, 3.0, 3.0);

float LinearizeDepth(float d) {
    float nearPlane = 0.1;
    float farPlane = 100;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

// 返回射线进入时间和在盒内的持续时间
vec2 rayBoxDst(vec3 origin, vec3 direction, vec3 boxMin, vec3 boxMax){
    vec3 invDirection = vec3(
    abs(direction.x) < 1e-5 ? 1e5 : 1.0 / direction.x,
    abs(direction.y) < 1e-5 ? 1e5 : 1.0 / direction.y,
    abs(direction.z) < 1e-5 ? 1e5 : 1.0 / direction.z
    );
    vec3 t0 = (boxMin - origin) * invDirection;
    vec3 t1 = (boxMax - origin) * invDirection;
    // 每个轴上的进入时间和离开时间
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    // 求三个轴的交集，就可以确定具体的进入和离开的交点到达时间
    float tIn = max(max(tmin.x,tmin.y),tmin.z);
    float tOut = min(min(tmax.x,tmax.y),tmax.z);
    return vec2(max(0, tIn), max(0, tOut-max(0,tIn)));
}
float sampleNoise(vec3 testPoint)
{
    // TODO：就剩下怎么进行采样噪声了
    vec3 regionSize = cloudBoxMax - cloudBoxMin;
    vec3 uvw = (testPoint - cloudBoxMin) / regionSize;

    return texture(noiseTexture, uvw).r;

}

float BeerPowder(float opticalDepth) {
    return 2 * exp(-opticalDepth) * (1-exp(-opticalDepth * 2));
}
// 计算一个采样点的光照
vec3 rayMarchToLight(vec3 currentPos, vec3 dirToLight)
{
    float lightAbsorptionTowardSun = 0.3;
    // beer衰减系数
    float dstInsideBox = rayBoxDst(currentPos, dirToLight, cloudBoxMin, cloudBoxMax).y;
    float stepSize = dstInsideBox / 10.0;

    // 计算当前采样点到光源之间的云层密度
    float totalDensity = 0.0;
    for (int i = 0; i < 20; i++)
    {
        currentPos += dirToLight * stepSize;
        totalDensity += max(0.0, sampleNoise(currentPos) * stepSize);
    }
    // 透光率
    float transmittance = exp(-totalDensity * lightAbsorptionTowardSun);
    // TODO:BeerPowder
    return transmittance * lightColor;
}

float HenyeyGreenstein(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}
float phase(float a) {
    vec4 _phaseParams = vec4(0.8, 0.3, 0.4, 1.5);
    float blend = .5;
    float hgBlend = HenyeyGreenstein(a, _phaseParams.x) * (1 - blend) + HenyeyGreenstein(a, -_phaseParams.y) * blend;
    return _phaseParams.z + hgBlend * _phaseParams.w;
}

// 一条射线穿越云层次数越多，表示这个角度云层越厚
vec4 cloudRayMarching(vec3 worldPos, vec3 direction, float depth)
{
    vec2 rayBoxInfo = rayBoxDst(camPos, direction, cloudBoxMin, cloudBoxMax);
    // 相机距离云的距离
    float dstToBox = rayBoxInfo.x;
    // 相机在云内部的穿行距离
    float dstInsideBox = rayBoxInfo.y;
    // 图1取第一项，图二取第一项但是为负数（所以当dstLimit为负数表示云彻底被遮挡），图三也是第一项， 所以取第二项的情况就是没有遮挡的情况
    float dstLimit = min(depth - dstToBox, dstInsideBox);  // 步进距离限制
    // 视线被模型遮挡，没有云参与计算
    if (dstLimit <= 0.0) return vec4(0,0,0,1);
    // 传入的lightPos是光照方向，不是光照位置
    vec3 lightDir = -lightPos;
    // 描述视角与光源方向夹角不同时的散射强度
    float phase = phase(dot(direction, lightDir));
    // 直接从AABB盒表面开始RM
    vec3 startPos = camPos + direction * dstToBox;
    vec3 sumLight = vec3(0);
    float stepSize = 0.5;
    // 已步进距离（时间）
    float travelled = 0.0;
    float transmittance = 1;
    for (int i = 0; i < 32; i++) {
        if(travelled < dstLimit){
            // 采样点
            vec3 testPoint = startPos + (direction * travelled);
            // 采样点光照
            vec3 light = rayMarchToLight(testPoint, normalize(lightDir));
            // 采样密度
            float density = sampleNoise(testPoint);
            sumLight += light * density * stepSize  * transmittance * phase;
            transmittance *= exp(-density * stepSize);
            if (transmittance < 0.01) break;
        }
        travelled += stepSize;
    }
    return vec4(sumLight,transmittance);
}
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
    viewPos /= viewPos.w; // 透视除法
    // Step 4: 观察空间 → 世界坐标
    return (inverseView * vec4(viewPos.xyz, 1.0)).xyz;
}
void main() {
    // 世界坐标（这里也可以从gBuffer的位置贴图获取，但是没有模型的地方就会出错（返回的是clearColor））
    vec3 worldPos = reconstructWorldPos(TexCoords);
    // raymarching方向
    vec3 rayMarchDir = normalize(worldPos - camPos);
    // 像素zBuffer深度（线性）
    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
    vec3 preColor = texture(preTexture, TexCoords).rgb;
    // 无模型位置 && 开启天空盒
    if(depth > 99 && showSkyBox == 1) {
        // 从天空盒拿颜色
        preColor = texture(environmentMap, rayMarchDir).rgb;
    }
    // 计算的是当前着色点云的密度
    vec4 cloudInfo = cloudRayMarching(worldPos, rayMarchDir, depth);
    float alpha = cloudInfo.w;
    vec3 cloudColor = cloudInfo.xyz;

    FragColor = vec4(mix(preColor,cloudColor,1-alpha), 1);

}
