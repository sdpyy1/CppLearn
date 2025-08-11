#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D preTexture;

void main() {
    // 固定5x5高斯核权重（已归一化）
    float weights[25] = float[](
    0.003765, 0.015019, 0.023792, 0.015019, 0.003765,
    0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
    0.023792, 0.094907, 0.149361, 0.094907, 0.023792,
    0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
    0.003765, 0.015019, 0.023792, 0.015019, 0.003765
    );
    ivec2 texSize = textureSize(preTexture, 0);

    // 计算单个纹理像素的尺寸（uv坐标偏移量）
    vec2 texelSize = 1.0 / vec2(texSize);
    vec3 result = vec3(0.0);
    int index = 0;

    // 固定5x5范围采样（从-2到+2的偏移）
    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            // 计算采样坐标偏移
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            // 累加加权采样结果
            result += texture(preTexture, TexCoords + offset).rgb * weights[index];
            index++;
        }
    }

    FragColor = vec4(result, 1.0);
}