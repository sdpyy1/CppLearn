#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gBufferTexture;
uniform int visualizeMode;

// 线性化深度（假设你传入了near和far平面距离）
uniform float near;
uniform float far;

// 线性化深度函数
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // [0,1] -> [-1,1]
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    if (visualizeMode == 0) {
        vec3 tex = texture(gBufferTexture, TexCoords).rgb;
        FragColor = vec4(tex, 1.0);
    }
    else if (visualizeMode == 1) {
        vec3 tex = texture(gBufferTexture, TexCoords).rgb;
        FragColor = vec4(tex * 0.5 + 0.5, 1.0);
    }
    else if (visualizeMode == 2) {
        float depth = texture(gBufferTexture, TexCoords).r;

        // 如果想直接显示原始深度（非线性，黑白不直观）：
        // FragColor = vec4(vec3(depth), 1.0);

        float linearDepth = LinearizeDepth(depth) / far; // 归一化到[0,1]
        FragColor = vec4(vec3(linearDepth), 1.0);
    }
    else {
        FragColor = vec4(1, 0, 1, 1);
    }
}
