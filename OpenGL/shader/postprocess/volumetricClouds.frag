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

// 3D噪声图
uniform sampler3D noiseTexture;
// AABB盒
//vec3 cloudBoxMin = vec3(20.0, 2.0, 20.0);
//vec3 cloudBoxMax = vec3(-20.0, 0.0, -20.0);
vec3 cloudBoxMin = vec3(-3.0, -3.0, -3.0);
vec3 cloudBoxMax = vec3(3.0, 3.0, 3.0);

float LinearizeDepth(float d) {
    float nearPlane = 0.1;
    float farPlane = 100;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

// 返回射线进入时间和在盒内的持续时间
vec2 rayBoxDst(vec3 origin, vec3 invDirection, vec3 boxMin, vec3 boxMax){
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
    vec3 regionSize = cloudBoxMax - cloudBoxMin;
    vec3 uvw = (testPoint - cloudBoxMin) / regionSize;
    return texture(noiseTexture, uvw).x;
}
// 一条射线穿越云层次数越多，表示这个角度云层越厚
float cloudRayMarching( vec3 direction, float depth)
{
    vec3 safeInvDir = vec3(
    abs(direction.x) < 1e-5 ? 1e5 : 1.0 / direction.x,
    abs(direction.y) < 1e-5 ? 1e5 : 1.0 / direction.y,
    abs(direction.z) < 1e-5 ? 1e5 : 1.0 / direction.z
    );
    vec2 rayBoxInfo = rayBoxDst(camPos, safeInvDir, cloudBoxMin, cloudBoxMax);
    // 相机距离云的距离
    float dstToBox = rayBoxInfo.x;
    // 相机在云内部的穿行距离
    float dstInsideBox = rayBoxInfo.y;
    // 图1取第一项，图二取第一项但是为负数（所以当dstLimit为负数表示云彻底被遮挡），图三也是第一项， 所以取第二项的情况就是没有遮挡的情况
    float dstLimit = min(depth - dstToBox, dstInsideBox);  // 步进距离限制
    // 视线被模型遮挡，没有云参与计算
    if (dstLimit <= 0.0) return 0.0;

    // 从AABB盒表面开始RM
    vec3 startPos = camPos + direction * dstToBox;
    float sum = 0.0;
    float stepSize = 0.1;
    float travelled = 0.0;

    for (int i = 0; i < 32; i++) {
        if(travelled < dstLimit){
            vec3 testPoint = startPos + (direction * travelled);
            sum += sampleNoise(testPoint) * stepSize;
            if(sum > 1){
                return 1.0;
            }
        }

        travelled += stepSize;
    }

    return clamp(sum, 0.0, 1.0);
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
    viewPos /= viewPos.w;

    // Step 4: 观察空间 → 世界坐标
    return (inverseView * vec4(viewPos.xyz, 1.0)).xyz;
}
void main() {

    // 世界坐标
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    // raymarching方向
    vec3 worldViewDir = normalize(worldPos - camPos);
    // 像素zBuffer深度
    float depth = LinearizeDepth(texture(gDepth, TexCoords).r);
    vec3 preColor = texture(preTexture, TexCoords).rgb;
    if(depth > 99) {
        worldPos = reconstructWorldPos(TexCoords);
        worldViewDir = normalize(worldPos - camPos);
        preColor = texture(environmentMap, worldViewDir).rgb;
    }
    // 计算的是当前着色点云的密度
    float cloud = cloudRayMarching(worldViewDir,depth);

    float alpha = clamp(cloud, 0.0, 1.0);
    vec3 cloudColor = vec3(1.0);

    FragColor = vec4(mix(preColor, cloudColor, alpha), 1);

    // TODO: （暂时）传递信息 体积云的地方写入深度，防止被天空盒挡住
    if(depth > 99 && alpha > 0.0) {
        FragColor = vec4(mix(preColor, cloudColor, alpha), 0.1);
    }
}
