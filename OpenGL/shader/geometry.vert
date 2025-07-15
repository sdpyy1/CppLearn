#version 330 core
layout(location = 0) in vec3 aPos;      // 顶点位置
layout(location = 1) in vec3 aNormal;   // 法线
layout(location = 2) in vec2 aTexCoords; // UV

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // 世界空间位置
    Normal = mat3(transpose(inverse(model))) * aNormal; // 世界空间法线
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

