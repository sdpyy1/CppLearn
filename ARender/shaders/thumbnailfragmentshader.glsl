
#version 430 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform Material material;

void main()
{
    // 实际使用的部分
    vec4 diffuse = texture(material.texture_diffuse1, TexCoords);

    // 假装使用specular和shininess（编译器无法优化掉）
    if (material.shininess > -1.0) {  // 永远为真但编译器无法静态分析
        vec4 specular = texture(material.texture_specular1, TexCoords);
        diffuse.rgb += specular.rgb * 0.000001;  // 微小到不影响视觉效果
    }

    FragColor = diffuse;
}
