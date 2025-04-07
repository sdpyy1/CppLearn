#version 330 core
out vec4 FragColor;

// 材质结构体
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};
// 光源结构体
struct Light {
    vec3  position;
    // 这个方向是指手电筒指向的方向
    vec3  direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

// 法线
in vec3 Normal;
// 像素对应空间位置
in vec3 FragPos;
// 纹理坐标
in vec2 TexCoords;

// 摄像机位置
uniform vec3 viewPos;
// 材质
uniform Material material;
// 光源属性
uniform Light light;

void main()
{
    // 环境光
    vec3 ambient  = light.ambient * texture(material.diffuse, TexCoords).rgb;

    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    // 手电筒与当前像素的夹角
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = (light.cutOff - light.outerCutOff);
    // 内圆之内为1，内外之间为0-1,外圆之外为0
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // 手电筒范围内外
    diffuse  *= intensity;
    specular *= intensity;

    // 衰减
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // 手电筒的距离
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    // 混合
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);

}