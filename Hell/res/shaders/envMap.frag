#version 420 core

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D ALB_Texture0;
layout (binding = 1) uniform sampler2D NRM_Texture0;
layout (binding = 2) uniform sampler2D RMA_Texture0;
layout (binding = 3) uniform sampler2D ALB_Texture1;
layout (binding = 4) uniform sampler2D NRM_Texture1;
layout (binding = 5) uniform sampler2D RMA_Texture1;

in float MaterialID;
in mat3 TBN;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 emissiveColor;
uniform bool hasEmissiveMap;
uniform int TEXTURE_FLAG;
uniform vec2 TEXTURE_SCALE;

uniform int i;

uniform vec3 lightPosition;
uniform float lightAttenuationConstant;
uniform float lightAttenuationLinear;
uniform float lightAttenuationExp;
uniform float lightStrength;
uniform vec3 lightColor;

uniform vec2 texOffset;

uniform sampler2D ALB_Texture;
uniform sampler2D NRM_Texture;
uniform sampler2D RMA_Texture;

const float PI = 3.14159265359;
const vec3 noColor = vec3(0.0, 0.0, 0.0);

struct PBRInfo
{
	float NdotL;                  // cos angle between normal and light direction
	float NdotV;                  // cos angle between normal and view direction
	float NdotH;                  // cos angle between normal and half vector
	float LdotH;                  // cos angle between light direction and half vector
	float VdotH;                  // cos angle between view direction and half vector
	float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
	float metalness;              // metallic value at the surface
	vec3 reflectance0;            // full reflectance color (normal incidence angle)
	vec3 reflectance90;           // reflectance color at grazing angle
	float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;            // color contribution from diffuse lighting
	vec3 specularColor;           // color contribution from specular lighting
};


float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

float CaclulateAttenuation(vec3 worldPos, vec3 lightPosition, float lightRadius)
{
    float magic = lightAttenuationExp; // was 1
    float dist = length(worldPos - lightPosition);
    float radius = lightAttenuationConstant;
    float num = saturate(1 - (dist / radius) * (dist / radius) * (dist / radius) * (dist / radius));
    return num * num / (dist * dist + magic);
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
	return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
	float NdotL = pbrInputs.NdotL;
	float NdotV = pbrInputs.NdotV;
	float r = pbrInputs.alphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
	float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
	float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
	return roughnessSq / (PI * f * f);
}

vec3 diffuse(PBRInfo pbrInputs)
{
	return pbrInputs.diffuseColor / PI;
}

vec3 Tonemap_ACES(const vec3 x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 OECF_sRGBFast(const vec3 linear) {
    return pow(linear, vec3(1.0 / 2.2));
}

void main()
{
	vec4 ALB;
	vec4 RMA;
	vec4 NRM;

	if (round(MaterialID) == 1) {
		ALB =  texture(ALB_Texture1, TexCoord);
		RMA =  texture(RMA_Texture1, TexCoord);
		NRM =  texture(NRM_Texture1, TexCoord);
	}
	else {
		ALB =  texture(ALB_Texture0, TexCoord);
		RMA =  texture(RMA_Texture0, TexCoord);
		NRM =  texture(NRM_Texture0, TexCoord);
	}

	float roughness = RMA.r + 0.001; 
	float metallic = RMA.g; 
	vec3 albedo = pow(ALB.rgb, vec3(2.2));

	roughness =RMA.r;	
	metallic = RMA.g;
	//ao = RMA.g;
	
	/////////
	// PBR //
	/////////
	
	vec3 color = vec3(0);
    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = albedo * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;
		
	float alphaRoughness = roughness * roughness;
	vec3 specularColor = mix(f0, albedo, metallic);

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	// For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	vec3 worldNormal = normalize(TBN * (NRM.rgb * 2.0 - 1.0));	
    vec3 n = worldNormal;
	vec3 v = normalize(lightPosition - FragPos);    // Vector from surface point to camera
	
    vec3 l = normalize(lightPosition - FragPos);   // Vector from surface point to light
	vec3 h = normalize(v + l);    // Half vector between both l and v            
	vec3 reflection = -normalize(reflect(v, n));
	reflection.y *= -1.0f;

	float NdotL = clamp(dot(n, l), 0.001, 1.0);
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float LdotH = clamp(dot(l, h), 0.0, 1.0);
	float VdotH = clamp(dot(v, h), 0.0, 1.0);

    PBRInfo pbrInputs = PBRInfo(
		NdotL,
		NdotV,
		NdotH,
		LdotH,
		VdotH,
		roughness,
		metallic,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor
	);

    vec3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);

	// Calculation of analytical lighting contribution
	vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
	vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	float LightStrength = lightStrength * 1;
	float LightRadius = lightAttenuationConstant * 2;
	vec3 radiance = CaclulateAttenuation(FragPos, lightPosition, LightRadius) * lightColor * LightStrength;
	//color = NdotL * (diffuseContrib + specContrib) * radiance;
	color = NdotL * (diffuseContrib + specContrib) * lightColor;
			
	// Tone mapping
	color = Tonemap_ACES(color);
    // Gamma compressionlighting
	color = OECF_sRGBFast(color);

	FragColor = vec4(vec3(color), 1);

	//mat3 tbn = mat3(normalize(attrTangent), normalize(attrBiTangent), normalize(attrNormal));
	
	//vec3 worldNormal = normalize(TBN * (NRM.rgb * 2.0 - 1.0));
	//FragColor = vec4(vec3(RMA), 1);
	//gNormal.a = ALB.a;
	//gNormal.rgb = attrNormal ;

}