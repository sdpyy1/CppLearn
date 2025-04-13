#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 texCoords;
uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture1;
uniform sampler2D texture2;
void main()
{
    vec3 I = normalize(Position - cameraPos);
    FragColor = vec4(mix(texture(texture1,texCoords), texture(texture2,texCoords),0.5));
    if(dot(-I, Normal)>0){
        float ratio = 1.00 / 1.52;

        vec3 R = refract(I, normalize(Normal), ratio);
        FragColor = mix(texture(skybox, R), FragColor, 0.5);
    }

}