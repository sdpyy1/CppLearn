#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D preTexture;
uniform sampler2D gDepth;


void main() {
    vec4 texColor = texture(preTexture, TexCoords);
//    // 防止场景被天空盒覆盖(天空盒已经继承到了体积云中，不需要了)
//    gl_FragDepth = texture(gDepth, TexCoords).r;
    FragColor = texColor;
}
