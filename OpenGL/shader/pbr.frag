#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D lutMap;

// Material maps
uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;
uniform sampler2D texture_metallic;
uniform sampler2D texture_roughness;
uniform sampler2D texture_ao;
uniform sampler2D texture_emission;

const float PI = 3.14159265359;

// Convert normal map to world space
vec3 getNormalFromMap()
{
    vec3 normalTS = texture(texture_normal, TexCoords).rgb;
    normalTS = normalTS * 2.0 - 1.0;
    return normalize(TBN * normalTS);
}

// GGX NDF
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

// Geometry: Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Fresnel: Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    // Material inputs (gamma correct)
    vec3 albedo     = pow(texture(texture_albedo, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(texture_metallic, TexCoords).r;
    float roughness = texture(texture_roughness, TexCoords).r;
    float ao        = texture(texture_ao, TexCoords).r;
    vec3 emission   = vec3(0,0,0);

    // Geometry
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - FragPos);
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor;

    // Fresnel base reflectivity
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Cook-Torrance BRDF (Direct lighting)
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // IBL - Diffuse
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    // IBL - Specular
    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * 4.0).rgb;
    vec2 brdf  = texture(lutMap, vec2(max(dot(N, V), 0.0), roughness)).rg;

    vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

    // Ambient lighting
    vec3 ambient = (kD * diffuseIBL + specularIBL) * ao;

    // Combine
    vec3 color = ambient + Lo;
    color = emission + pow(color, vec3(1.0 / 2.2)); // gamma correction
    FragColor = vec4(color, 1.0);
}
