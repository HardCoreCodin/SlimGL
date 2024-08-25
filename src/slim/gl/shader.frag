#version 330

#define pi (3.14159265f)
#define TAU (2.0f*pi)
#define UNIT_SPHERE_AREA_OVER_SIX ((4.0f*pi)/6.0f)
#define ONE_OVER_PI (1.0f/pi)

#define HAS_ALBEDO_MAP 1
#define HAS_NORMAL_MAP 1
#define DRAW_DEPTH 4
#define DRAW_POSITION 8
#define DRAW_NORMAL 16
#define DRAW_UVS 32
#define DRAW_ALBEDO 64

#define EPS 0.0001f

in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;
in vec4 DirectionalLightSpacePos;

out vec4 color;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

struct Light
{
	vec3 color;
	float intensity;
};

struct DirectionalLight 
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	
	vec3 position;
	float constant;
	float linear;
	float exponent;
};

struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge;
};

struct OmniShadowMap
{
	samplerCube shadowMap;
	float farPlane;
};

struct Material
{
    vec3 albedo;
    float roughness;
    vec3 F0;
    float metalness;
    float normal_strength;
    uint flags;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform OmniShadowMap omniShadowMaps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D shadow_map;

uniform Material material;

uniform vec3 eyePosition;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ggxTrowbridgeReitz_D(float roughness, float NdotH) { // NDF
    float a = roughness * roughness;
    float denom = NdotH * NdotH * (a - 1.0f) + 1.0f;
    return (
        a
        /
        (pi * denom * denom)
    );
}

float ggxSchlickSmith_G(float roughness, float NdotL, float NdotV) {
    float k = roughness * roughness * 0.5f;
    return (
        NdotV / max(mix(NdotV, 1.0f, k), EPS) *
        NdotL / max(mix(NdotL, 1.0f, k), EPS)
    );
}

vec3 schlickFresnel(float HdotL, vec3 F0) {
    return F0 + (1.0f - F0) * pow(1.0f - HdotL, 5.0f);
}

vec3 cookTorrance(float roughness, float NdotL, float NdotV, float NdotH, vec3 F) {
    float D = ggxTrowbridgeReitz_D(roughness, NdotH);
    float G = ggxSchlickSmith_G(roughness, NdotL, NdotV);
    return (
        F * (D * G
        / (
        4.0f * NdotL * NdotV
    )));
}

vec3 BRDF(vec3 albedo, float roughness, vec3 F0, float metalness, vec3 V, vec3 N, float NdotL, vec3 L) {
    vec3 Fs = vec3(0.0);
    vec3 Fd = albedo * ((1.0f - metalness) * ONE_OVER_PI);
	float NdotV = dot(N, V);
    if (NdotV <= 0.0f ||
        roughness <= 0)
        return Fs + Fd;

    vec3 H = normalize(L + V);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float HdotL = clamp(dot(H, L), 0.0, 1.0);
    vec3 F = schlickFresnel(HdotL, F0);
    Fs = cookTorrance(roughness, NdotL, NdotV, NdotH, F);
    Fd *= 1.0f - F;
    return Fs + Fd;
}

vec3 shadeFromLight(Light light, vec3 L, vec3 N, vec3 albedo) {
    //vec3 L = light.position - P;
    
	float NdotL = dot(L, N);
    if (NdotL <= 0.0f)
        return vec3(0.0f);

    float Ld = length(L);
    NdotL /= Ld;
    vec3 V = normalize(eyePosition - FragPos);

    vec3 brdf = BRDF(albedo, material.roughness, material.F0, material.metalness, V, N, NdotL, L);
    return light.color * (brdf * NdotL * light.intensity);// / (Ld * Ld));
}
/*
vec4 CalcLightByDirection(Light light, vec3 direction, float shadowFactor, vec3 N)
{
	vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;
	
	float diffuseFactor = max(dot(N, normalize(direction)), 0.0f);
	vec4 diffuseColor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);
	
	vec4 specularColor = vec4(0, 0, 0, 0);
	
	if(diffuseFactor > 0.0f)
	{
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, N));
		
		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}

	return (ambientColor + (1.0 - shadowFactor) * (diffuseColor + specularColor));
}
*/
float CalcPointShadowFactor(PointLight light, int shadowIndex)
{
	vec3 fragToLight = FragPos - light.position;
	float currentDepth = length(fragToLight);
	
	float shadow = 0.0;
	float bias   = 0.15;
	int samples  = 20;
	float viewDistance = length(eyePosition - FragPos);
	float diskRadius = (1.0 + (viewDistance / omniShadowMaps[shadowIndex].farPlane)) / 25.0;
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(omniShadowMaps[shadowIndex].shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
		closestDepth *= omniShadowMaps[shadowIndex].farPlane;   // Undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);  
	
	return shadow;
}

float CalcShadowFactor(vec4 DirectionalLightSpacePos, vec3 N)
{
	vec3 projCoords = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	float closestDepth = texture(shadow_map, projCoords.xy).r;
	float currentDepth = projCoords.z;
	
	vec3 lightDir = normalize(directionalLight.direction);
	float bias = max(0.05 * (1.0 - dot(N, lightDir)), 0.0005);
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	
	shadow /= 9.0;
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}
	
	return shadow;
}

vec4 CalcDirectionalLight(vec4 DirectionalLightSpacePos, vec3 N, vec3 albedo)
{
	float shadow = CalcShadowFactor(DirectionalLightSpacePos, N);
	vec3 light = shadeFromLight(directionalLight.base, directionalLight.direction, N, albedo);
	return (1.0 - shadow) * vec4(light, 1.0);  //CalcLightByDirection(directionalLight.base, directionalLight.direction, shadow, N);
}

vec4 CalcPointLight(PointLight pLight, int shadowIndex, vec3 N, vec3 albedo)
{
	vec3 direction = pLight.position - FragPos;
	float distance = length(direction);
	direction = normalize(direction);
	
	float shadow = CalcPointShadowFactor(pLight, shadowIndex);
	vec3 light = shadeFromLight(pLight.base, direction, N, albedo);
	vec4 color = (1.0 - shadow) * vec4(light, 1.0);  // CalcLightByDirection(pLight.base, direction, shadowFactor, N);
	float attenuation = pLight.exponent * distance * distance +
						pLight.linear * distance +
						pLight.constant;
	
	return (color / attenuation);
}

vec4 CalcSpotLight(SpotLight sLight, int shadowIndex, vec3 N, vec3 albedo)
{
	vec3 rayDirection = normalize(FragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);
	
	if(slFactor > sLight.edge)
	{
		vec4 color = CalcPointLight(sLight.base, shadowIndex, N, albedo);
		
		return color * (1.0f - (1.0f - slFactor)*(1.0f/(1.0f - sLight.edge)));
		
	} else {
		return vec4(0, 0, 0, 0);
	}
}

vec4 CalcPointLights(vec3 N, vec3 albedo)
{
	vec4 totalColor = vec4(0, 0, 0, 0);
	for(int i = 0; i < pointLightCount; i++)
	{		
		totalColor += CalcPointLight(pointLights[i], i, N, albedo);
	}
	
	return totalColor;
}

vec4 CalcSpotLights(vec3 N, vec3 albedo)
{
	vec4 totalColor = vec4(0, 0, 0, 0);
	for(int i = 0; i < spotLightCount; i++)
	{		
		totalColor += CalcSpotLight(spotLights[i], i + MAX_POINT_LIGHTS, N, albedo);
	}
	
	return totalColor;
}

vec3 decodeNormal(const vec4 color) {
    return normalize(color.xyz * 2.0f - 1.0f);
}

vec3 toneMapped(vec3 color) {
    vec3 x = clamp(color - 0.004f, 0.0f, 1.0f);
    vec3 x2_times_sholder_strength = x * x * 6.2f;
    return (x2_times_sholder_strength + x*0.5f)/(x2_times_sholder_strength + x*1.7f + 0.06f);
}

void main()
{
	vec3 N = normalize(Normal);
	vec3 T = normalize(Tangent);
	vec3 B = cross(T, N);
	N = normalize(mat3(T, B, N) * decodeNormal(texture(normal_map, TexCoord)));
	vec3 albedo = texture(albedo_map, TexCoord).rgb * material.albedo;
	color = CalcDirectionalLight(DirectionalLightSpacePos, N, albedo);
	color += CalcPointLights(N, albedo);
	color += CalcSpotLights(N, albedo);
	//color.xyz = vec3(material.roughness);
}