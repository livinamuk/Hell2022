#version 420 core

layout (location = 0) out vec4 fragout;

layout (binding = 0) uniform sampler2D ALB_Texture0;
layout (binding = 1) uniform sampler2D NRM_Texture0;
layout (binding = 2) uniform sampler2D RMA_Texture0;
layout (binding = 3) uniform sampler2D gBufferFinalLighting_Texture;
layout (binding = 4) uniform sampler2D Dust0_Texture;
layout (binding = 5) uniform sampler2D Dust1_Texture;

in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoord;
in mat3 TBN;

uniform mat4 inverseViewMatrix;
uniform mat4 viewMatrix;
uniform mat4 model;

uniform float screenWidth;
uniform float screenHeight;

vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3333333333333333) * direction;
  color += texture2D(image, uv) * 0.29411764705882354;
  color += texture2D(image, uv + (off1 / resolution)) * 0.35294117647058826;
  color += texture2D(image, uv - (off1 / resolution)) * 0.35294117647058826;
  return color; 
}

const float EPS = 1e-4;
const float PI = 3.14159265359;

uniform vec3 lightPosition[20];
uniform float lightRadius[20];
uniform float lightMagic[20];
uniform float lightStrength[20];
uniform vec3 lightColor[20];

float saturate(float value) {
	return clamp(value, 0.0, 1.0);
}

float CaclulateAttenuation(vec3 worldPos, vec3 lightPosition, float radius, float magic)
{
    float dist = length(worldPos - lightPosition);
    float num = saturate(1 - (dist / radius) * (dist / radius) * (dist / radius) * (dist / radius));
    return num * num / (dist * dist + magic);
}

struct PBRInfo {
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

vec3 specularReflection(PBRInfo pbrInputs) {	
return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

float geometricOcclusion(PBRInfo pbrInputs) {
	float NdotL = pbrInputs.NdotL;
	float NdotV = pbrInputs.NdotV;
	float r = pbrInputs.alphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

float microfacetDistribution(PBRInfo pbrInputs) {
	float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
	float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
	return roughnessSq / (PI * f * f);
}

vec3 diffuse(PBRInfo pbrInputs) {
	return pbrInputs.diffuseColor / PI;
}




void main()
{   
	vec2 TexCoords = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
	
	// Agressive blur
	vec3 gBufferFinalLighting = texture(gBufferFinalLighting_Texture, TexCoords).rgb;
	vec2 direction = vec2(2.25);
	vec4 blurredImage = blur5(gBufferFinalLighting_Texture, TexCoords,  vec2(screenWidth, screenHeight), direction);
	blurredImage.r -= 0.007;
	blurredImage.g	 -= 0.0035;






	/////////
	// PBR //
	/////////	

	vec3 albedo = texture(ALB_Texture0, TexCoord).rgb;
	float roughness = texture(RMA_Texture0, TexCoord).r;	
	float metallic  = texture(RMA_Texture0, TexCoord).g;
	float ao = texture(RMA_Texture0, TexCoord).b;	

	float DUST0 = texture(Dust0_Texture, TexCoord * 5).r;
	float DUST1 = texture(Dust1_Texture, TexCoord * 5).r;

	//vec3 dust = vec3(texture(Dust0_Texture, TexCoord * 5).r) * 0.5;
	//float dust2 = 1 - texture(Dust1_Texture, TexCoord * 5).r * 0.5;
	roughness += DUST0 * 0.25;
	metallic += DUST1 * 0.5;
	fragout.a = 1;

    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = albedo * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;

	float alphaRoughness = roughness * roughness;
	vec3 specularColor = mix(f0, albedo, metallic);

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

    vec3 viewPos = vec3(inverseViewMatrix * vec4(0, 0, 0, 1));

    vec3 n = Normal;
	vec3 v = normalize(viewPos - WorldPos);    // Vector from surface point to camera

	vec3 l;
	int light_count = 10;

	vec3 color = vec3(0);

	// warning these lights don't check their shadow maps
	for (int i=0; i < light_count; i++)	{
	
		float attenuation = CaclulateAttenuation(WorldPos, lightPosition[i], lightRadius[i], lightMagic[i]) * lightStrength[i];
		vec3 radiance = lightColor[i] * attenuation;
		l = normalize(lightPosition[i] - WorldPos);
		
		vec3 h = normalize(v + l);            
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
		vec3 directLighting = NdotL * (diffuseContrib + specContrib) * radiance;
	
	//	fragout = vec4(directLighting.xyz, 1);
		
		//vec3 DUST0 = vec3(texture(Dust_Texture0, TexCoord * 5).r) * 0.025 * radiance;
		//vec3 DUST1 = texture(Dust_Texture1, TexCoord).rgb;

		color += (directLighting + DUST0 * 0.025) * radiance;

		//color =  directLighting + DUST0;
	}
		
//	fragout.rgb =  texture(Dust1_Texture, TexCoord).rgb ;
	fragout.rgb = color + blurredImage.rgb;
	
	//fragout.rgb = color;// + blurredImage.rgb;

	//fragout.rgb =  vec3(color	);//blurredImage.rgb;//vec3(1, 0, 0);



	//color.r = 1;
	//vec= vec4(blur.xyz, 1);
//	fragout = vec4(1,1,0,1);



	//fragout = vec4(blurredImage.rgb, 1);//texture(Dust1_Texture, TexCoords);
}