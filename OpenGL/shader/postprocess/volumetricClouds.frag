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
uniform int showSkyBox;
// 3D噪声图
uniform sampler3D basicNoiseTexture;
uniform sampler3D detailNoiseTexture;
uniform sampler2D weatherTexture;
// AABB盒
vec3 cloudBoxMin = vec3(200.0, 10.0, 200.0);
vec3 cloudBoxMax = vec3(-200.0, 4.0, -200.0);
//vec3 cloudBoxMin = vec3(-3.0, -3.0, -3.0);
//vec3 cloudBoxMax = vec3(3.0, 3.0, 3.0);
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
    float farPlane = 100;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}
// 比例缩放
float remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
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
//vec2 rayBoxDst(vec3 origin, vec3 direction, vec3 boxMin, vec3 boxMax){
//    vec3 invDirection = vec3(
//    abs(direction.x) < 1e-5 ? 1e5 : 1.0 / direction.x,
//    abs(direction.y) < 1e-5 ? 1e5 : 1.0 / direction.y,
//    abs(direction.z) < 1e-5 ? 1e5 : 1.0 / direction.z
//    );
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


float hash(vec3 p)
{
    // 简单 hash，返回 0-1
    return fract(sin(dot(p, vec3(12.9898,78.233,37.719))) * 43758.5453);
}

float sampleNoise(vec3 worldPos, AtmosphereParameter param)
{
    // 定义不同尺度的采样，用于生成多尺度噪声
    const int octaves = 3;
    float total = 0.0;
    float persistence = 0.5;
    float frequency = 1.0;
    float amplitude = 1.0;

    // 从天气纹理获取基础权重（使用世界位置的x/z分量作为2D坐标）
    vec2 weatherUV = worldPos.xz * 0.001; // 缩放因子控制天气纹理的影响范围
    vec4 weatherData = texture(weatherTexture, weatherUV);
    float weatherWeight = weatherData.r; // 使用r通道作为权重

    // 多尺度采样3D噪声，结合天气纹理的影响
    for(int i = 0; i < octaves; i++) {
        // 计算当前尺度的采样坐标
        vec3 samplePos = worldPos * frequency * 0.01; // 缩放因子控制噪声的细节程度

        // 采样3D噪声
        float noise = texture(basicNoiseTexture, samplePos).r;

        // 应用天气纹理的影响，使噪声分布更符合天气特征
        noise = mix(noise, noise * weatherWeight, 0.3);

        // 累加各尺度的噪声值
        total += noise * amplitude;

        // 更新频率和振幅（遵循分形噪声的特性）
        frequency *= 2.0;
        amplitude *= persistence;
    }

    // 将噪声值归一化到[0, 1]范围
    return clamp(total * 0.5 + 0.5, 0.3, 1.0);
}

// 在球外无交点返回(0,0)  在球内返回(0,dstToSphere)
vec2 RaySphereDst(vec3 sphereCenter, float sphereRadius, vec3 pos, vec3 rayDir)
{
    vec3 oc = pos - sphereCenter;
    float b = dot(rayDir, oc);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float t = b * b - c; // t > 0 两个交点, =0 相切, <0 不相交

    if (t < 0.0) {
        return vec2(0.0, 0.0);
    }

    float delta = sqrt(t);
    float dstToSphere = max(-b - delta, 0.0);
    float dstInSphere = max(-b + delta - dstToSphere, 0.0);

    return vec2(dstToSphere, dstInSphere);
}

vec2 RayCloudLayerDst(vec3 sphereCenter, float earthRadius, float heightMin, float heightMax,
                      vec3 pos, vec3 rayDir, bool isShape)
{
    vec2 cloudDstMin = RaySphereDst(sphereCenter, heightMin + earthRadius, pos, rayDir);
    vec2 cloudDstMax = RaySphereDst(sphereCenter, heightMax + earthRadius, pos, rayDir);

    float dstToCloudLayer = 0.0;
    float dstInCloudLayer = 0.0;

    if (isShape)
    {
        // 在地表上
        if (pos.y <= heightMin + earthRadius)
        {
            vec3 startPos = pos + rayDir * cloudDstMin.y;
            // 到达内圈时，如果y值低于地表说明穿透地球了
            if (startPos.y >= earthRadius)
            {
                dstToCloudLayer = cloudDstMin.y;
                dstInCloudLayer = cloudDstMax.y - cloudDstMin.y;
            }
            return vec2(dstToCloudLayer, dstInCloudLayer);
        }

        // 在云层内
        if (pos.y > heightMin + earthRadius && pos.y <= heightMax + earthRadius)
        {
            dstToCloudLayer = 0.0;
            dstInCloudLayer = cloudDstMin.y > 0.0 ? cloudDstMin.x : cloudDstMax.y;
            return vec2(dstToCloudLayer, dstInCloudLayer);
        }

        // 在云层外
        dstToCloudLayer = cloudDstMax.x;
        dstInCloudLayer = cloudDstMin.y > 0.0 ? cloudDstMin.x - dstToCloudLayer : cloudDstMax.y;
    }
    else // 光照步进，步进开始点一定在云层内
    {
        dstToCloudLayer = 0.0;
        dstInCloudLayer = cloudDstMin.y > 0.0 ? cloudDstMin.x : cloudDstMax.y;
    }
    return vec2(dstToCloudLayer, dstInCloudLayer);
}
float BeerPowder(float density, float absorptivity)
{
    return 2.0 * exp(-density * absorptivity) * (1.0 - exp(-2.0 * density));
}
// 计算一个采样点的光照
float rayMarchToLight(AtmosphereParameter param, vec3 currentPos, vec3 dirToLight)
{
    float SAMPLE_NUM = 16;
    float dstInsideBox = RayCloudLayerDst(param.earthCenter, param.PlanetRadius, param.cloudStart, param.cloudEnd, currentPos, dirToLight, false).y;
    float stepSize = dstInsideBox / SAMPLE_NUM;
    float traveled = 0;
    // 计算当前采样点到光源之间的云层密度
    float totalDensity = 0.0;
    for (int i = 0; i < SAMPLE_NUM; i++)
    {
        traveled += stepSize;
        vec3 pos = currentPos + dirToLight * traveled;
        totalDensity += sampleNoise(pos,param) * stepSize;
    }
    // 透光率
    return BeerPowder(totalDensity,1);
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
// 球体体积云
vec4 cloudRayMarching(AtmosphereParameter param, vec3 camPosInPlanet, vec3 viewDir, float depth)
{
    float SAMPLE_NUM = 100;
    vec3 lightDir = normalize(-lightPos);
    vec2 rayBoxInfo = RayCloudLayerDst(param.earthCenter, param.PlanetRadius, param.cloudStart, param.cloudEnd, camPosInPlanet, viewDir, true);
    float dstToCloud = rayBoxInfo.x;
    float dstInsideCloud = rayBoxInfo.y;
    if(dstInsideCloud <= 0) return vec4(0,0,0,1);
    float transmittance = 1.0f;
    vec3 scattering = vec3(0.0f);
    vec3 startPos = camPosInPlanet + viewDir * dstToCloud;
    float stepSize =dstInsideCloud / SAMPLE_NUM;
    float sunPhase = dualLobPhase(0.5, -0.5, 0.2, -dot(viewDir,lightDir));

    vec3 testPos = startPos;
    for(int i = 0;i<SAMPLE_NUM;i++){
        testPos += viewDir * stepSize;
        float density = sampleNoise(testPos,param);
        float stepTransmittance = exp(-density * stepSize);
        // 寒霜引擎 光照计算
        vec3 stepScattering = rayMarchToLight(param,testPos,lightDir) * lightColor * sunPhase;
        vec3 sigmaS = vec3(density);
        const float sigmaA = 0.0;
        vec3 sigmaE = max(vec3(1e-8f), sigmaA + sigmaS);
        vec3 sactterLitStep = stepScattering * sigmaS;
        sactterLitStep = transmittance * (sactterLitStep - sactterLitStep * stepTransmittance);
        sactterLitStep /= sigmaE;
        scattering += sactterLitStep;
        // 累积透射率
        transmittance *= stepTransmittance;
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

    vec3 worldPosInPlanet = reconstructWorldPos(TexCoords) + vec3(0, params.PlanetRadius, 0);
    vec3 camPosInPlanet = camPos + vec3(0, params.PlanetRadius, 0);
    // 视角方向
    vec3 viewDirInPlanet = normalize(worldPosInPlanet - camPosInPlanet);
    // 场景颜色
    vec3 preColor = texture(preTexture, TexCoords).rgb;
    // 像素深度
    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
    // 无模型位置 && 开启天空盒
    if(depth > 99 && showSkyBox == 1) {
        // 从天空盒拿颜色
//      preColor = texture(environmentMap, viewDirInPlanet).rgb;
        // 实时大气渲染
        preColor = singleScatterSkyColor(camPosInPlanet,params,viewDirInPlanet);
    }
// TODO:体积云放弃
    vec4 cloudInfo = cloudRayMarching(params, camPosInPlanet, viewDirInPlanet, depth);
    float alpha = cloudInfo.w;
    vec3 cloudColor = cloudInfo.xyz;
    FragColor = vec4(mix(preColor,cloudColor,1-alpha), 1);
//    FragColor = vec4(preColor,1);
}
