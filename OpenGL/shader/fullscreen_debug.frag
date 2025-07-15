#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gBufferTexture;
uniform int visualizeMode;

void main() {
    vec3 tex = texture(gBufferTexture, TexCoords).rgb;

    if (visualizeMode == 0) // default, show as is
    FragColor = vec4(tex, 1.0);
    else if (visualizeMode == 1) // visualize normals [-1,1] -> [0,1]
    FragColor = vec4(tex * 0.5 + 0.5, 1.0);
    else if (visualizeMode == 2) // visualize depth or other special
    FragColor = vec4(vec3(tex.r), 1.0); // example
    else
    FragColor = vec4(1, 0, 1, 1); // fallback: magenta
}
