#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 texCoords;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
out VS_OUT {
    vec2 texCoords;
} gs_in;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gs_in.texCoords = aTexCoords;
    texCoords = aTexCoords;
}