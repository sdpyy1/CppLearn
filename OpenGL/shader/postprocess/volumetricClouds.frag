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
uniform sampler2D weatherTexture;
// AABB盒
vec3 cloudBoxMin = vec3(20.0, 10.0, 20.0);
vec3 cloudBoxMax = vec3(-20.0, 4.0, -20.0);
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
};

// ------------------------------------------------------------------------- //

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

// ------------------------------------------------------------------------- //
// 散射
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
    float dstToPlanet = RayIntersectSphere(vec3(0,0,0), param.PlanetRadius, camPosInPlanet, viewDir);
    float dstToAtmosphere = RayIntersectSphere(vec3(0,0,0), param.PlanetRadius + param.AtmosphereHeight, camPosInPlanet, viewDir);
    // 还没实现别的渲染，纯黑的，先不判断
//    if(dstToPlanet > 0.0 || dstToAtmosphere < 0.0){
//        return vec3(0);
//    }
    float stepSize = dstToAtmosphere / float(N_SAMPLE);
    vec3 testPoint = camPosInPlanet;
    vec3 color = vec3(0);
    for (int i =0; i<N_SAMPLE; i++) {
        // 太阳方向rayMarching的距离
        float disToLight = RayIntersectSphere(vec3(0,0,0), param.PlanetRadius + param.AtmosphereHeight, testPoint, lightDir);
        // 太阳到采样点之间的透光率
        vec3 tramsmittanceLightToTest = Transmittance(param,testPoint + lightDir * disToLight,testPoint);
        // 散射光比例
        vec3 scattering = Scattering(param, testPoint, lightDir, viewDir);
        // 摄像机到采样点之间的透光度
        vec3 tramsmittanceTestToCam = Transmittance(param,testPoint,camPosInPlanet);
        // 采样点的光照计算。// TODO：需要把太阳光放大很多倍才有比较亮的效果（因为ToneMapping压黑的）
        vec3 inScattering = tramsmittanceLightToTest * scattering * tramsmittanceTestToCam  * stepSize * lightColor * 4;
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
    vec3 boundsCentre = (cloudBoxMax + cloudBoxMin) * 0.5;
    vec3 regionSize = cloudBoxMax - cloudBoxMin;
    vec3 uvw = (testPoint - cloudBoxMin) / regionSize;
    vec2 uv = (regionSize.xz * 0.5f + (testPoint.xz - boundsCentre.xz)) / max(regionSize.x, regionSize.z);

    // 天气图
    float coverage = texture(weatherTexture, uv).r;

    // 采样点在体积云的高度占比
    float heightPercent = (testPoint.y - cloudBoxMin.y) / regionSize.y;
    float heightGradient = clamp(remap(heightPercent, 0.0, coverage, 0.0, 1.0), 0.0, 1.0);

    // 3D 基础噪声
//    float3 uvwShape  = rayPos * _shapeTiling + float3(speedShape, speedShape * 0.2,0);
//    float4 shapeNoise = tex3D(_noiseTex, uvwShape);
//    float4 normalizedShapeWeights = _shapeNoiseWeights / dot(_shapeNoiseWeights, 1);
//    float shapeFBM = dot(shapeNoise, normalizedShapeWeights) ;
//    float baseShapeDensity = shapeFBM + _densityOffset * 0.01;
//    return baseShapeDensity;


    // 最终密度
    float density = 1 * heightGradient;
    return density;
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
    AtmosphereParameter a = AtmosphereParameter(
    8000,   // RayleighScatteringScalarHeight
    1200,   // MieScatteringScalarHeight
    0.8,     // MieAnisotropy
    25000,  // OzoneLevelCenterHeight
    15000,  // OzoneLevelWidth
    6360000,// PlanetRadius
    60000  // AtmosphereHeight
    );
    // 世界坐标（这里也可以从gBuffer的位置贴图获取，但是没有模型的地方就会出错（返回的是clearColor）,所以统一都用重建来完成）
    vec3 worldPosInPlanet = reconstructWorldPos(TexCoords) + vec3(0, a.PlanetRadius, 0);
    vec3 camPosInPlanet = camPos + vec3(0, a.PlanetRadius, 0);
    // 视角方向
    vec3 viewDir = normalize(worldPosInPlanet - camPosInPlanet);
    // 场景颜色
    vec3 preColor = texture(preTexture, TexCoords).rgb;

    // 像素zBuffer深度（线性）
    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
    // 无模型位置 && 开启天空盒
    if(depth > 99 && showSkyBox == 1) {
        // 从天空盒拿颜色
//      preColor = texture(environmentMap, rayMarchDir).rgb;
        preColor = singleScatterSkyColor(camPosInPlanet,a,viewDir);
    }

    FragColor = vec4(preColor, 1);
}
