#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D finalTexture;
uniform sampler2D gDepth;


void main() {
    vec4 texColor = texture(finalTexture, TexCoords);
    // 防止场景被天空盒覆盖
    gl_FragDepth = texture(gDepth, TexCoords).r;
    FragColor = texColor;
}
