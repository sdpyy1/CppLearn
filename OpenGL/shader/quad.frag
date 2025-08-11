#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D debugTexture;
uniform int channel; // 0: 完整纹理, 1: R, 2: G, 3: B, 4: A, 5: 法线贴图, 6: 深度贴图(线性化)

uniform float near;
uniform float far;

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}


vec3 visualizeNormal(vec3 normal) {
    return (normal + 1.0) * 0.5;
}

void main() {
    vec4 texColor = texture(debugTexture, TexCoords);

    switch (channel) {
        case 0: // 完整纹理
            FragColor = texColor;
            break;
        case 1: // R通道
            FragColor = vec4(vec3(texColor.r), 1.0);
            break;
        case 2: // G通道
            FragColor = vec4(vec3(texColor.g), 1.0);
            break;
        case 3: // B通道
            FragColor = vec4(vec3(texColor.b), 1.0);
            break;
        case 4: // A通道
            FragColor = vec4(vec3(texColor.a), 1.0);
            break;
        case 5: // 法线贴图专用（转换为可视范围）
            FragColor = vec4(visualizeNormal(texColor.rgb), 1.0);
            break;
        case 6: // 深度贴图专用（线性化处理）
            {
                float depthValue = texColor.r;
                float linearDepth = linearizeDepth(depthValue) / far;
                FragColor = vec4(vec3(linearDepth), 1.0);
                break;
            }
        default:
            FragColor = texColor;
            break;
    }
}
