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
uniform sampler2D preTexture;
float LinearizeDepth(float d) {
    float nearPlane = 0.1;
    float farPlane = 100;
    float z = d * 2.0 - 1.0; // back to NDC z in [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}
// AABB盒
vec3 minPos = vec3(-3.0, -3.0, -3.0);
vec3 maxPos = vec3(3.0, 3.0, 3.0);

// 返回射线进入时间和在盒内的持续时间
vec2 rayBox(vec3 origin, vec3 invDirection, vec3 boxMin, vec3 boxMax){
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

// 一条射线穿越云层次数越多，表示这个角度云层越厚
float cloudRayMarching(vec3 startPoint, vec3 direction)
{
    vec3 testPoint = startPoint;
    float sum = 0.0;
    direction *= 0.5;//每次步进间隔
    // 在3*3*3的立方体内部才会叠加sum
    for (int i = 0; i < 256; i++)//步进总长度
    {
        testPoint += direction;
        if (testPoint.x < 3.0 && testPoint.x > -3.0 &&
        testPoint.z < 3.0 && testPoint.z > -3.0 &&
        testPoint.y < 3.0 && testPoint.y > -3.0)
        sum += 0.01;
    }
    return sum;
}

void main() {
    // 世界坐标
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    // raymarching方向
    vec3 worldViewDir = normalize(camPos - worldPos);
    // 计算的是当前着色点云的密度
    float cloud = cloudRayMarching(worldPos, worldViewDir);
    vec3 preColor = texture(preTexture, TexCoords).rgb;
    float alpha = clamp(cloud, 0.0, 1.0);
    vec3 cloudColor = vec3(1.0);

    vec3 finalColor = mix(preColor, cloudColor, alpha);
    FragColor = vec4(finalColor, 1.0);
}
