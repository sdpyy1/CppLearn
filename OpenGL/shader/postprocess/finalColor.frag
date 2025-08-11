#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D preTexture;
uniform sampler2D lightTexture;
uniform sampler2D gDepth;
// Tone Mapping
uniform int toneMappingType;

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}
vec3 ACESFilmToneMapping(vec3 color)
{
    // 适当的曝光缩放，可以视为手动曝光调整（可调参数）
    color *= 0.6;
    // ACES tone mapping 曲线
    color = RRTAndODTFit(color);
    // Clamp 到 [0, 1]
    return clamp(color, 0.0, 1.0);
}

void main() {
    vec3 lightColor = texture(preTexture, TexCoords).rgb;

    vec3 finalColor = lightColor;

    // toneMapping
    if (toneMappingType == 1) {
        finalColor = finalColor / (finalColor + vec3(1.0));
    } else if (toneMappingType == 2) {
        finalColor = ACESFilmToneMapping(finalColor);
    }

    // 伽马矫正
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    // 防止场景被天空盒覆盖
    gl_FragDepth = texture(gDepth, TexCoords).r;

    FragColor = vec4(finalColor, 1.0);
}
