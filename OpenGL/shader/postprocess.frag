#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D lightTexture;
uniform sampler2D gDepth;

void main() {
    vec3 hdrColor = texture(lightTexture, TexCoords).rgb;
    gl_FragDepth = texture(gDepth, TexCoords).r;

    FragColor = vec4(hdrColor, 1.0);
}
