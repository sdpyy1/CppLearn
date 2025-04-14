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

    if(!gl_FrontFacing){
        FragColor = vec4(mix(texture(texture1,texCoords), texture(texture2,texCoords),0.5));
    }else{
        vec3 R = reflect(I, normalize(Normal));
        FragColor = texture(skybox, R);
    }
}