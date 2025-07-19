#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec3 WorldPos;
out vec2 TexCoords;
out vec3 T;  // 单独输出
out vec3 B;
out vec3 N;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    WorldPos = vec3(model * vec4(aPos, 1.0));

    // 将 TBN 向量变换到世界空间
    T = normalize(mat3(model) * aTangent);
    B = normalize(mat3(model) * aBitangent);
    N = normalize(mat3(model) * aNormal);

    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
