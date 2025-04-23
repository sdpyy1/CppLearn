const LightCubeVertexShader = `
attribute vec3 aVertexPosition;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;


void main(void) {

  gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);

}
`;

const LightCubeFragmentShader = `
#ifdef GL_ES
precision mediump float;
#endif

uniform float uLigIntensity;
uniform vec3 uLightColor;

void main(void) {
    
  //gl_FragColor = vec4(1,1,1, 1.0);
  gl_FragColor = vec4(uLightColor, 1.0);
}
`;
const VertexShader = `
attribute vec3 aVertexPosition;
attribute vec3 aNormalPosition;
attribute vec2 aTextureCoord;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

varying highp vec3 vFragPos;
varying highp vec3 vNormal;
varying highp vec2 vTextureCoord;

void main(void) {

  vFragPos = aVertexPosition;
  vNormal = aNormalPosition;

  gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);

  vTextureCoord = aTextureCoord;

}
`;

const FragmentShader = `
#ifdef GL_ES
precision mediump float;
#endif

uniform int uTextureSample;
uniform vec3 uKd;
uniform sampler2D uSampler;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;

varying highp vec3 vFragPos;
varying highp vec3 vNormal;
varying highp vec2 vTextureCoord;

void main(void) {
  
  if (uTextureSample == 1) {
    gl_FragColor = texture2D(uSampler, vTextureCoord);
  } else {
    gl_FragColor = vec4(uKd,1);
  }

}
`;

const PhongVertexShader = `
attribute vec3 aVertexPosition;
attribute vec3 aNormalPosition;
attribute vec2 aTextureCoord;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;
void main(void) {
vFragPos = aVertexPosition;
vNormal = aNormalPosition;
gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition , 1.0);
vTextureCoord = aTextureCoord;
}`;

const PhongFragmentShader = `
#ifdef GL_ES
precision mediump float; 
#endif
// Uniforms
uniform sampler2D uSampler;      // 纹理
uniform vec3 uKd;                // 漫反射系数(*)
uniform vec3 uKs;                // 镜面反射系数(*)
uniform vec3 uLightPos;          // 光源坐标
uniform vec3 uCameraPos;         // 摄像机坐标
uniform float uLightIntensity;   // 光源强度
uniform int uTextureSample;      // 纹理采样标志
 
varying highp vec2 vTextureCoord;   // 纹理坐标
varying highp vec3 vFragPos;        // 片段位置
varying highp vec3 vNormal;         // 法线
void main(void) {
    vec3 color;
    // 若使用纹理，则color赋值为gamma校正后的纹理颜色，否则使用漫反射系数颜色
    if (uTextureSample == 1) {
        color = pow(texture2D(uSampler, vTextureCoord).rgb, vec3(2.2));
    } else {
        color = uKd;
    }
 
    // 环境光颜色计算： K_a*L_a
    vec3 ambient = 0.05 * color;
    
    // 单位化光线/法线方向，并计算漫反射各分量
    vec3 lightDir = normalize(uLightPos - vFragPos);
    vec3 normal = normalize(vNormal);
    float diff = max(dot(lightDir, normal), 0.0);
    float light_atten_coff = uLightIntensity / length(uLightPos - vFragPos);
    // 计算漫反射颜色  L_d=k_d*(I/r^2)*max(0,n·l)
    vec3 diffuse = diff * light_atten_coff * color;
 
    // 计算视线方向，反射方向，并计算镜面反射各分量
    vec3 viewDir = normalize(uCameraPos - vFragPos);
    float spec = 0.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 35.0);
    // 计算镜面反射颜色 L_s=k_s*(I/r^2)*(max(0,n·h)^p)    
    vec3 specular = uKs * light_atten_coff * spec;
 
    // 计算最终颜色进行gamma校正
    gl_FragColor = vec4(pow((ambient + diffuse + specular), vec3(1.0 / 2.2)), 1.0);
}
`