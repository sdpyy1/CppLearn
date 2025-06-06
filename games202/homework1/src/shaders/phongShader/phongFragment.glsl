#ifdef GL_ES
precision mediump float;
#endif

// Phong related variables
uniform sampler2D uSampler;
uniform vec3 uKd;
uniform vec3 uKs;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform vec3 uLightIntensity;

varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;

// Shadow map related variables
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

uniform sampler2D uShadowMap;

varying vec4 vPositionFromLight;

highp float rand_1to1(highp float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

void uniformDiskSamples( const in vec2 randomSeed ) {

  float randNum = rand_2to1(randomSeed);
  float sampleX = rand_1to1( randNum ) ;
  float sampleY = rand_1to1( sampleX ) ;

  float angle = sampleX * PI2;
  float radius = sqrt(sampleY);

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( radius * cos(angle) , radius * sin(angle)  );

    sampleX = rand_1to1( sampleY ) ;
    sampleY = rand_1to1( sampleX ) ;

    angle = sampleX * PI2;
    radius = sqrt(sampleY);
  }
}



float useShadowMap(sampler2D shadowMap, vec4 shadowCoord){
  // shadowmap中存储的深度
  float lightDepth = unpack(texture2D(shadowMap, shadowCoord.xy));
  // 着色点深度
  float shadowDepth = shadowCoord.z;
  float visibility = 1.0;
  float bias = 0.01;
  // 被挡住了
  if (shadowDepth  > lightDepth + bias) {
    visibility = 0.0;
  }
  return visibility;
}

float PCF(sampler2D shadowMap, vec4 coords,float Size) {
  uniformDiskSamples(coords.xy);
  // 采样数
  float numSamples = 0.0;
  // 没有遮挡的采样数
  float numUnBlock = 0.0;
  // 过滤核大小
  float filterSize = Size;
  float mapSize = 2048.0;
  // 过滤核范围
  float filterRange = filterSize / mapSize;
  for(int i = 0;i<NUM_SAMPLES;i++){
    vec2 samplexCoor = coords.xy + poissonDisk[i] * filterRange;
    // 采样时可能会越界
    if(samplexCoor.x > 0.0 && samplexCoor.x < 1.0 && samplexCoor.y > 0.0 && samplexCoor.y < 1.0) {
      numSamples++;
      if(useShadowMap(shadowMap,vec4(samplexCoor,coords.z,1.0)) == 1.0) {
        numUnBlock++;
      }
    }
  }
  return numUnBlock/numSamples;
}


float findBlocker( sampler2D shadowMap,  vec2 uv, float zReceiver ) {
	int numSamples = 0;
  float sumDepth = 0.0;
  float searchSize = 15.0;
  float mapSize = 2048.0;
  float searchRange = searchSize / mapSize;
  for( int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; i ++ ) {
    vec2 sampleCoor = uv + poissonDisk[i] * searchRange;
    // 采样时可能会越界
    if(sampleCoor.x > 0.0 && sampleCoor.x < 1.0 && sampleCoor.y > 0.0 && sampleCoor.y < 1.0) {
      float depth = unpack(texture2D(shadowMap, sampleCoor));
      if(depth < zReceiver) {
        sumDepth += depth;
        numSamples++;
      }
    }
  }
  if(numSamples > 0) {
    return sumDepth / float(numSamples);
  } else {
    return zReceiver;
  }
}

float PCSS(sampler2D shadowMap, vec4 coords){
  uniformDiskSamples(coords.xy);
  // STEP 1: avgblocker depth
  float avgBlockerDepth = findBlocker(shadowMap, coords.xy, coords.z);
  // STEP 2: penumbra size
  // 假设光源尺寸为20
  float Wlight = 20.0;
  float penumbraSize = (coords.z - avgBlockerDepth) * Wlight / avgBlockerDepth;
  // STEP 3: filtering
  // 把半影尺寸当做过滤核大小
  return PCF(shadowMap, coords,penumbraSize);
}




vec3 blinnPhong() {
  vec3 color = texture2D(uSampler, vTextureCoord).rgb;
  color = pow(color, vec3(2.2));

  vec3 ambient = 0.05 * color;

  vec3 lightDir = normalize(uLightPos);
  vec3 normal = normalize(vNormal);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 light_atten_coff =
      uLightIntensity / pow(length(uLightPos - vFragPos), 2.0);
  vec3 diffuse = diff * light_atten_coff * color;

  vec3 viewDir = normalize(uCameraPos - vFragPos);
  vec3 halfDir = normalize((lightDir + viewDir));
  float spec = pow(max(dot(halfDir, normal), 0.0), 32.0);
  vec3 specular = uKs * light_atten_coff * spec;

  vec3 radiance = (ambient + diffuse + specular);
  vec3 phongColor = pow(radiance, vec3(1.0 / 2.2));
  return phongColor;
}

void main(void) {

  float visibility = 1.0;
  // 透视投影时才需要
  // vec3 shadowCoord = vPositionFromLight.xyz / vPositionFromLight.w;
  vec3 shadowCoord = (vPositionFromLight.xyz+1.0)/2.0;
  // visibility = useShadowMap(uShadowMap, vec4(shadowCoord, 1.0));
  // visibility = PCF(uShadowMap, vec4(shadowCoord, 1.0),5);
  visibility = PCSS(uShadowMap, vec4(shadowCoord, 1.0));

  vec3 phongColor = blinnPhong();

  gl_FragColor = vec4(phongColor * visibility, 1.0);
  // gl_FragColor = vec4(phongColor, 1.0);
}