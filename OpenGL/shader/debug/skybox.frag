#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube environmentMap;

void main()
{
    // HDR图本身的线性空间无需pow2.2
    vec3 envColor = texture(environmentMap, localPos).rgb;
    // tonemapping
    envColor = envColor / (envColor + vec3(1.0));
    // 伽马矫正
    envColor = pow(envColor, vec3(1.0 / 2.2));

    FragColor = vec4(envColor, 1.0);
}