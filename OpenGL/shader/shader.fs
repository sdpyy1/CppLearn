#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D sampler1;
uniform sampler2D sampler2;

void main()
{
    FragColor = mix(texture(sampler1, TexCoord), texture(sampler2, TexCoord), 0.2);
}