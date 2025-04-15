#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec2 texCoords;
} gs_in;
uniform sampler2D diffuse_texture1;

void main()
{
    FragColor = texture(diffuse_texture1, gs_in.texCoords);
}