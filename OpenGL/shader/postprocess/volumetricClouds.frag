#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
precision highp float;
precision highp int;
uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform mat4 projection;
uniform mat4 view;
uniform sampler2D gPosition;
uniform sampler2D gDepth;
uniform sampler2D preTexture;
uniform samplerCube environmentMap;
uniform int skyMode;
uniform bool showClouds;
// 3D噪声图
uniform sampler3D basicNoiseTexture;
uniform sampler3D detailNoiseTexture;
uniform sampler2D weatherTexture;
uniform sampler2D cloudCurlNoise;

// 参数
uniform float cloudFeather;
uniform float time;



// AABB盒
vec3 cloudBoxMin = vec3(-640.0, 210.0, -640.0);
vec3 cloudBoxMax = vec3(640.0, 215.0, 640.0);
//vec3 cloudBoxMin = vec3(-30.0, 0.0, -30.0);
//vec3 cloudBoxMax = vec3(30.0, 3.0, 30.0);
float PI = 3.141592653;
// ------------------------------------------------------------------------- //
// Atmosphere Parameters
struct AtmosphereParameter {
    float RayleighScatteringScalarHeight;
    float MieScatteringScalarHeight;
    float MieAnisotropy;
    float OzoneLevelCenterHeight;
    float OzoneLevelWidth;
    float PlanetRadius;
    float AtmosphereHeight;
    float cloudStart;
    float cloudEnd;
    vec3 earthCenter;
};
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

float LinearizeDepth(float d){
    float nearPlane = 0.1;
    float farPlane = 1000;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}
// -----------------------------大气渲染 BEGIN-------------------------------------------- //
vec3 RayleighCoefficient(AtmosphereParameter param, float h)
{
    const vec3 sigma = vec3(5.802, 13.558, 33.1) * 1e-6;
    float H_R = param.RayleighScatteringScalarHeight;
    float rho_h = exp(-(h / H_R));
    return sigma * rho_h;
}

float RayleighPhase(AtmosphereParameter param, float cos_theta)
{
    return (3.0 / (16.0 * 3.14159265)) * (1.0 + cos_theta * cos_theta);
}

vec3 MieCoefficient(AtmosphereParameter param, float h)
{
    const vec3 sigma = vec3(3.996e-6);
    float H_M = param.MieScatteringScalarHeight;
    float rho_h = exp(-(h / H_M));
    return sigma * rho_h;
}

float MiePhase(AtmosphereParameter param, float cos_theta)
{
    float g = param.MieAnisotropy;

    float a = 3.0 / (8.0 * 3.14159265);
    float b = (1.0 - g * g) / (2.0 + g * g);
    float c = 1.0 + cos_theta * cos_theta;
    float d = pow(1.0 + g * g - 2.0 * g * cos_theta, 1.5);

    return a * b * (c / d);
}

vec3 MieAbsorption(AtmosphereParameter param, float h)
{
    const vec3 sigma = vec3(4.4e-6);
    float H_M = param.MieScatteringScalarHeight;
    float rho_h = exp(-(h / H_M));
    return sigma * rho_h;
}

vec3 OzoneAbsorption(AtmosphereParameter param, float h)
{
    const vec3 sigma_lambda = vec3(0.650, 1.881, 0.085) * 1e-6;
    float center = param.OzoneLevelCenterHeight;
    float width = param.OzoneLevelWidth;
    float rho = max(0.0, 1.0 - abs(h - center) / width);
    return sigma_lambda * rho;
}

vec3 Scattering(AtmosphereParameter param, vec3 p, vec3 lightDir, vec3 viewDir)
{
    float cos_theta = dot(lightDir, viewDir);

    float h = length(p) - param.PlanetRadius;
    vec3 rayleigh = RayleighCoefficient(param, h) * RayleighPhase(param, cos_theta);
    vec3 mie = MieCoefficient(param, h) * MiePhase(param, cos_theta);

    return rayleigh + mie;
}
// 计算两点之间的透光率
vec3 Transmittance(in AtmosphereParameter param, vec3 p1, vec3 p2)
{
    if(p1 == p2){
        return vec3(1);
    }
    const int N_SAMPLE = 16;
    vec3 dir = normalize(p2 - p1);
    float distance = length(p2 - p1);
    float ds = distance / float(N_SAMPLE);

    vec3 sum = vec3(0.0);
    vec3 p = p1 + dir * ds * 0.5;
    for(int i = 0; i < N_SAMPLE; i++)
    {
        float h = length(p) - param.PlanetRadius;
        vec3 scattering = RayleighCoefficient(param, h) + MieCoefficient(param, h);
        vec3 absorption = OzoneAbsorption(param, h) + MieAbsorption(param, h);
        vec3 extinction = scattering + absorption;
        sum += extinction * ds;
        p += dir * ds;
    }
    // 计算透光率
    return exp(-sum);
}
float RayIntersectSphere(vec3 center, float radius, vec3 rayStart, vec3 rayDir)
{
    float OS = length(center - rayStart);
    float SH = dot(center - rayStart, rayDir);
    float OH = sqrt(OS * OS - SH * SH);

    // 射线未击中球体
    if(OH > radius) return -1.0;

    float PH = sqrt(radius * radius - OH * OH);

    // 使用最小正距离
    float t1 = SH - PH;
    float t2 = SH + PH;
    float t = (t1 < 0.0) ? t2 : t1;

    return t;
}

vec3 singleScatterSkyColor(vec3 camPosInPlanet, AtmosphereParameter param, vec3 viewDir){
    vec3 lightDir = normalize(-lightPos);
    int N_SAMPLE = 16;
    float dstToPlanet = RayIntersectSphere(param.earthCenter, param.PlanetRadius, camPosInPlanet, viewDir);
    float dstToAtmosphere = RayIntersectSphere(param.earthCenter, param.PlanetRadius + param.AtmosphereHeight, camPosInPlanet, viewDir);
    // 还没实现别的渲染，纯黑的，先不判断
    //    if(dstToPlanet > 0.0 || dstToAtmosphere < 0.0){
    //        return vec3(0);
    //    }
    float stepSize = dstToAtmosphere / float(N_SAMPLE);
    vec3 testPoint = camPosInPlanet;
    vec3 color = vec3(0);
    for (int i =0; i<N_SAMPLE; i++) {
        // 太阳方向rayMarching的距离
        float disToLight = RayIntersectSphere(param.earthCenter, param.PlanetRadius + param.AtmosphereHeight, testPoint, lightDir);
        // 太阳到采样点之间的透光率
        vec3 tramsmittanceLightToTest = Transmittance(param,testPoint + lightDir * disToLight,testPoint);
        // 散射光比例
        vec3 scattering = Scattering(param, testPoint, lightDir, viewDir);
        // 摄像机到采样点之间的透光度
        vec3 tramsmittanceTestToCam = Transmittance(param,testPoint,camPosInPlanet);
        // 采样点的光照计算。// TODO：需要把太阳光放大很多倍才有比较亮的效果（因为步长的原因）
        vec3 inScattering = tramsmittanceLightToTest * scattering * tramsmittanceTestToCam  * stepSize * lightColor * 16;
        testPoint += viewDir * stepSize;
        color += inScattering;
    }
    float cosAngle = dot(viewDir, lightDir);
    if(cosAngle > cos(0.02)) {
        // 视线指向太阳圆盘
        color += Transmittance(param, lightPos + vec3(0,param.PlanetRadius + param.AtmosphereHeight,0), camPosInPlanet) * lightColor;
    }
    return color;
}
// -----------------------------大气渲染 END-------------------------------------------- //

// -----------------------------体积云 BEGIN-------------------------------------------- //
// 返回射线进入时间和在盒内的持续时间 AABB
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


float hash(vec3 p)
{
    // 简单 hash，返回 0-1
    return fract(sin(dot(p, vec3(12.9898,78.233,37.719))) * 43758.5453);
}

float saturate(float x) { return clamp(x, 0.0, 1.0); }

float remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + ((( original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}
//获取云类型密度
float getCloudTypeDensity(float heightFraction, float cloud_min, float cloud_max, float feather)
{
    //云的底部羽化需要弱一些，所以乘0.5
    return saturate(remap(heightFraction, cloud_min, cloud_min + feather * 0.5, 0, 1)) * saturate(remap(heightFraction, cloud_max - feather, cloud_max, 1, 0));
}

//float sampleNoise(vec3 testPos)
//{
//    vec3 UVW = (testPos - cloudBoxMin) / (cloudBoxMax - cloudBoxMin);
//    float normalizeHeight = UVW.y;
//
//    // 超参数
//    float kCoverage = 0.5;
//    vec3 windDirection = normalize(vec3(0.5, 0, 0));
//    float cloudSpeed = 0.01;
//    float kDensity = 1;
//    float cloudBasicNoiseScale =1;
//    float cloudDetailNoiseScale = 1;
//    // 坐标转到 [0,1]
//    vec3 windOffset = (windDirection + vec3(0.0, 0.0001, 0.0)) * time * cloudSpeed;
//    UVW += windDirection * normalizeHeight;
//    vec2 UV = UVW.xz;
//    // 天气图采样
//    float weatherValue = texture(weatherTexture, UV).r;
//    float localCoverage = texture(cloudCurlNoise, (time * cloudSpeed * 50.0 + UV) * 0.00001+ 0.5).x;
//    localCoverage = saturate(localCoverage * 3.0 - 0.75) * 0.2;
//    float coverage = saturate(kCoverage * (localCoverage + weatherValue));
//
//    float gradienShape = remap(normalizeHeight, 0.00, 0.10, 0.1, 1.0) * remap(normalizeHeight, 0.10, 0.80, 1.0, 0.2);
//    float basicNoise = pow(texture(basicNoiseTexture, (UVW + windOffset) * vec3(cloudBasicNoiseScale)).r,7);
//    float basicCloudNoise = gradienShape * basicNoise;
//    float basicCloudWithCoverage = coverage * remap(basicCloudNoise, 1.0 - coverage, 1, 0, 1);
//    vec3 sampleDetailNoise = UVW - windOffset * 0.15;
//
//    float detailNoiseComposite = texture(detailNoiseTexture, sampleDetailNoise * cloudDetailNoiseScale).r;
//    float detailNoiseMixByHeight = 0.2 * mix(detailNoiseComposite, 1 - detailNoiseComposite, saturate(normalizeHeight * 10.0));
//    float densityShape = saturate(0.01 + normalizeHeight * 1.15) * kDensity * remap(normalizeHeight, 0.0, 0.1, 0.0, 1.0) * remap(normalizeHeight, 0.8, 1.0, 1.0, 0.0);
//    float cloudDensity = remap(basicCloudWithCoverage, detailNoiseMixByHeight, 1.0, 0.0, 1.0);
//    return saturate(cloudDensity * densityShape);
//}
float sampleNoise(vec3 testPos)
{
    float shapeTiling = 1.0;
    float speedShape = 0.2;

    vec3 UVW = (testPos - cloudBoxMin) / (cloudBoxMax - cloudBoxMin);
    vec2 UV = UVW.xz;
    float heightPercent = UVW.y;
    float coverage = texture(weatherTexture,UV).r;
    // 让云上窄下宽
    vec3 uvwShape = UVW;
    float shapeNoise = texture(basicNoiseTexture, uvwShape).r;

    const float containerEdgeFadeDst = 10;
    float dstFromEdgeX = min(containerEdgeFadeDst, min(testPos.x - cloudBoxMin.x, cloudBoxMax.x - testPos.x));
    float dstFromEdgeZ = min(containerEdgeFadeDst, min(testPos.z - cloudBoxMin.z, cloudBoxMax.z - testPos.z));
    float edgeWeight = min(dstFromEdgeZ, dstFromEdgeX) / containerEdgeFadeDst;

    float gMin = remap(coverage, 0, 1, 0.1, 0.6);
    float heightGradient = saturate(remap(heightPercent, 0.0,coverage, 1, 0)) * saturate(remap(heightPercent, 0.0, gMin, 0, 1));
//    heightGradient *= edgeWeight;

    return coverage * pow(shapeNoise,2) * getCloudTypeDensity(heightPercent, 0.0, 1, cloudFeather);
}
//// 在球外无交点返回(0,0)  在球内返回(0,dstToSphere)
//vec2 RaySphereDst(vec3 sphereCenter, float sphereRadius, vec3 pos, vec3 rayDir)
//{
//    vec3 oc = pos - sphereCenter;
//    float b = dot(rayDir, oc);
//    float c = dot(oc, oc) - sphereRadius * sphereRadius;
//    float t = b * b - c; // t > 0 两个交点, =0 相切, <0 不相交
//
//    if (t < 0.0) {
//        return vec2(0.0, 0.0);
//    }
//
//    float delta = sqrt(t);
//    float dstToSphere = max(-b - delta, 0.0);
//    float dstInSphere = max(-b + delta - dstToSphere, 0.0);
//
//    return vec2(dstToSphere, dstInSphere);
//}
//
//vec2 RayCloudLayerDst(vec3 sphereCenter, float earthRadius, float heightMin, float heightMax, vec3 pos, vec3 rayDir, bool isShape)
//{
//    vec2 cloudDstMin = RaySphereDst(sphereCenter, heightMin + earthRadius, pos, rayDir);
//    vec2 cloudDstMax = RaySphereDst(sphereCenter, heightMax + earthRadius, pos, rayDir);
//    float dstToCloudLayer = 0.0;
//    float dstInCloudLayer = 0.0;
//    if (isShape)
//    {
//        // 在地表上
//        if (pos.y <= heightMin + earthRadius)
//        {
//            vec3 startPos = pos + rayDir * cloudDstMin.y;
//            // 到达内圈时，如果y值低于地表说明穿透地球了
//            if (startPos.y >= earthRadius)
//            {
//                dstToCloudLayer = cloudDstMin.y;
//                dstInCloudLayer = cloudDstMax.y - cloudDstMin.y;
//            }
//            return vec2(dstToCloudLayer, dstInCloudLayer);
//        }
//
//        // 在云层内
//        if (pos.y > heightMin + earthRadius && pos.y <= heightMax + earthRadius)
//        {
//            dstToCloudLayer = 0.0;
//            dstInCloudLayer = cloudDstMin.y > 0.0 ? cloudDstMin.x : cloudDstMax.y;
//            return vec2(dstToCloudLayer, dstInCloudLayer);
//        }
//
//        // 在云层外
//        dstToCloudLayer = cloudDstMax.x;
//        dstInCloudLayer = cloudDstMin.y > 0.0 ? cloudDstMin.x - dstToCloudLayer : cloudDstMax.y;
//    }
//    else // 光照步进，步进开始点一定在云层内
//    {
//        dstToCloudLayer = 0.0;
//        dstInCloudLayer = cloudDstMin.y > 0.0 ? cloudDstMin.x : cloudDstMax.y;
//    }
//    return vec2(dstToCloudLayer, dstInCloudLayer);
//}
float BeerPowder(float density, float absorptivity)
{
    return 2.0 * exp(-density * absorptivity) * (1.0 - exp(-2.0 * density));
}
// 计算一个采样点的光照
float rayMarchToLight(vec3 currentPos, vec3 dirToLight)
{
    float darknessThreeshold = 0.4;
    float SAMPLE_NUM = 16;
    float dstInsideBox = rayBoxDst(currentPos, dirToLight, cloudBoxMin, cloudBoxMax).y;
    float stepSize = dstInsideBox / SAMPLE_NUM;
//    float stepSize = 0.1;
    float traveled = 0;
    vec3 testPos = currentPos;
    // 计算当前采样点到光源之间的云层密度
    float transmittanceToLight = 1.0;
    for (int i = 0; i < SAMPLE_NUM; i++)
    {
        float stepDensity = sampleNoise(testPos);
        transmittanceToLight *= exp(-stepDensity * stepSize);
        testPos += dirToLight * stepSize;
    }
    // 透光率
    return transmittanceToLight * (1-darknessThreeshold) + darknessThreeshold;
}

float hgPhase(float g, float cosTheta)
{
    float numer = 1.0f - g * g;
    float denom = 1.0f + g * g + 2.0f * g * cosTheta;
    return numer / (4.0f * PI * denom * sqrt(denom));
}

float dualLobPhase(float g0, float g1, float w, float cosTheta)
{
    return mix(hgPhase(g0, cosTheta), hgPhase(g1, cosTheta), w);
}
vec4 cloudRayMarching(vec3 viewDir, vec3 lightDir, float depth)
{
    float SAMPLE_NUM = 16;
    vec2 rayBoxInfo = rayBoxDst(camPos, viewDir, cloudBoxMin, cloudBoxMax);
    // 相机距离云的距离
    float dstToBox = rayBoxInfo.x;
    // 相机在云内部的穿行距离
    float dstInsideBox = rayBoxInfo.y;
    // 图1取第一项，图二取第一项但是为负数（所以当dstLimit为负数表示云彻底被遮挡），图三也是第一项， 所以取第二项的情况就是没有遮挡的情况
    float dstLimit = min(depth - dstToBox, dstInsideBox);  // 步进距离限制
    // 视线被模型遮挡，没有云参与计算
    if (dstLimit <= 0.0) return vec4(0,0,0,1);

    // 摄像机到着色点的透射率
    float transmittance = 1.0f;
    // 光照结果
    vec3 scattering = vec3(0.0f);
    // RayMarching起点
    vec3 testPos = camPos + viewDir * dstToBox;
    // 步长
    float stepSize = dstInsideBox / SAMPLE_NUM;
//    float stepSize = 0.5;
    // 方向散射系数
//    float sunPhase = dualLobPhase(0.5, -0.5, 0.2, -dot(viewDir,lightDir));
//    float sunPhase = hgPhase(0.8, dot(viewDir,lightDir));
    float sunPhase = 1;

    for(int i = 0;i<SAMPLE_NUM;i++){
        // 当前密度
        float stepDensity = sampleNoise(testPos);
        if(stepDensity > 0.0){
            // 当前透射率
            float stepTransmittance = exp(-stepDensity * stepSize);

            // 寒霜引擎 光照计算
            //        vec3 stepScattering = lightColor * rayMarchToLight(testPos,lightDir) * sunPhase;
            //        vec3 sigmaS = vec3(stepDensity);
            //        const float sigmaA = 0.0;
            //        vec3 sigmaE = max(vec3(1e-8f), sigmaA + sigmaS);
            //        vec3 sactterLitStep = stepScattering * sigmaS;
            //        sactterLitStep = transmittance * (sactterLitStep - sactterLitStep * stepTransmittance);
            //        sactterLitStep /= sigmaE;

            vec3 sigmaS = vec3(stepDensity);
            //TODO：这里还缺大气透射率
            vec3 stepScattering = lightColor * rayMarchToLight(testPos,lightDir) * sunPhase * transmittance * (sigmaS * stepSize);
            // 累积光照
            scattering += stepScattering;
            // 累积透射率
            transmittance *= stepTransmittance;
            if(transmittance < 0.01) break;
        }
        // 步进
        testPos += viewDir * stepSize;
    }
    return vec4(scattering,transmittance);
}
// -----------------------------体积云 END-------------------------------------------- //

void main() {
    AtmosphereParameter params;
    params.RayleighScatteringScalarHeight = 8000.0;
    params.MieScatteringScalarHeight      = 1200.0;
    params.MieAnisotropy                  = 0.8;
    params.OzoneLevelCenterHeight         = 25000.0;
    params.OzoneLevelWidth                = 15000.0;
    params.PlanetRadius                   = 6360000.0;
    params.AtmosphereHeight               = 60000.0;
    params.cloudStart                     = 2500.0;
    params.cloudEnd                       = 2550.0;
    params.earthCenter                    = vec3(0.0, 0.0, 0.0);


    vec3 worldPos = reconstructWorldPos(TexCoords);
    vec3 viewDir = normalize(worldPos - camPos);


    vec3 worldPosInPlanet = worldPos + vec3(0, params.PlanetRadius, 0);
    vec3 camPosInPlanet = camPos + vec3(0, params.PlanetRadius, 0);
    // 视角方向
    vec3 viewDirInPlanet = normalize(worldPosInPlanet - camPosInPlanet);
    // 场景颜色
    vec3 preColor = vec3(0);
    // 像素深度
    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
    // 无模型位置 && 开启天空盒
    if(depth > 99) {
        if(skyMode == 1){
            // 从天空盒拿颜色
            preColor = texture(environmentMap, viewDir).rgb;
        }else if(skyMode == 2){
            // 实时大气渲染
            preColor = singleScatterSkyColor(camPosInPlanet,params,viewDirInPlanet);
        }
    }else{
        preColor = texture(preTexture, TexCoords).rgb;
    }
    if(showClouds){
        // lightPos存的是平行光照射方向
        vec3 lightDir = normalize(-lightPos);
        vec4 cloudInfo = cloudRayMarching(viewDir, lightDir, depth);
        float alpha = cloudInfo.w;
        vec3 cloudColor = cloudInfo.xyz;
        FragColor = vec4(mix(preColor,cloudColor,1-alpha), 1);
    }else{
        FragColor = vec4(preColor,1);
    }
}
