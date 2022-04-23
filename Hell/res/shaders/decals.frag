#version 420 core

//out vec4 FragColor;
layout (location = 0) out vec3 gAlbedo;
//layout (location = 1) out vec3 gNormal;
//layout (location = 2) out vec3 gRMA;

layout (binding = 0) uniform sampler2D depthTexture;
layout (binding = 1) uniform sampler2D normalTexture;
layout (binding = 2) uniform sampler2D diffuseTexture;
layout (binding = 3) uniform sampler2D maskTexture;


in vec2 SplitscreenAdjustedCoords;

uniform mat4 inverseProjectionMatrix;
uniform mat4 inverseViewMatrix;
uniform mat4 model;
uniform float screenWidth;
uniform float screenHeight;
uniform vec3 targetPlaneSurfaceNormal;
uniform int writeRoughnessMetallic;

uniform vec3 u_CameraFront;
uniform vec3 u_ViewPos;

uniform vec3 lightPosition[20];
uniform float lightRadius[20];
uniform float lightMagic[20];
uniform float lightStrength[20];
uniform vec3 lightColor[20];

uniform vec3 norm;

uniform int _playerIndex;

in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

const float EPS = 1e-4;
const float PI = 3.14159265359;

in vec2 TexCoords;

// PBR SHIT
float saturate(float value) {
	return clamp(value, 0.0, 1.0);
}
float CaclulateAttenuation(vec3 worldPos, vec3 lightPosition, float radius, float magic) {
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
// END PBR SHIT

void main()
{    
    // Get the Fragment Z position (from the depth buffer)
    vec2 depthCoords = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    float z = texture(depthTexture, vec2(depthCoords.s, depthCoords.t)).x * 2.0f - 1.0f;
    
	vec4 clipSpacePosition;
	if (_playerIndex == 1)      
        clipSpacePosition = vec4(vec2(TexCoords.s * 2.0 - 1.0, ((TexCoords.t * 2 + -1) * 2.0 - 1.0) ), z, 1.0);
    if (_playerIndex == 2)      
        clipSpacePosition = vec4(vec2(TexCoords.s * 2.0 - 1.0, ((TexCoords.t * 2 + 0) * 2.0 - 1.0) ), z, 1.0);
		
    vec4 viewSpacePosition = inverseProjectionMatrix * clipSpacePosition;
    // Get the Fragment XYZ position (perspective division, via it's depth value)
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverseViewMatrix * viewSpacePosition;
    vec3 WorldPos = worldSpacePosition.xyz;
	
	vec4 objectPosition = inverse(model) * vec4(WorldPos, 1.0);

    if (abs(objectPosition.x) > 0.5)
        discard;
    if (abs(objectPosition.y) > 0.5)
        discard;

    //vec3 normal = texture(normalTexture, vec2(depthCoords.s, depthCoords.t)).rgb;

	// Add 0.5 to get texture coordinates.
	vec2 decalTexCoord = objectPosition.xy + 0.5;
    vec3 diffuseColor = texture(diffuseTexture, decalTexCoord).rgb;
  //  vec3 diffuseColor = texture(normalTexture, decalTexCoord).rgb;
    vec4 mask = texture(maskTexture, decalTexCoord);




	
	float roughness =0;
	float metallic = 0;

	/////////
	// PBR //
	/////////
	
	//vec3 color = vec3(0);
    vec3 f0 = vec3(0.04);
	float alphaRoughness = roughness * roughness;
	vec3 specularColor = mix(f0, diffuseColor, metallic);

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

    vec3 viewPos = vec3(inverseViewMatrix * vec4(0, 0, 0, 1));
	
    vec3 n = targetPlaneSurfaceNormal;	
	vec3 v = normalize(u_ViewPos - FragPos); 

	vec3 l;
	int light_count = 6;

	vec3 color = vec3(0);

	for (int i=0; i < light_count; i++)	
	{	
		float attenuation = CaclulateAttenuation(WorldPos, lightPosition[i], lightRadius[i], lightMagic[i]) * lightStrength[i];
		vec3 radiance = lightColor[i] * attenuation;
		l = normalize(lightPosition[i] - WorldPos);

		// flip to face the light if required
		if (dot(n, l) < 0)
			n *= -1;
	
		float NdotL = clamp(abs(dot(n, l)), 0.001, 1.0);
						
		vec3 h = normalize(v + l);            
		vec3 reflection = -normalize(reflect(v, n));
		reflection.y *= -1.0f;

		float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
		float NdotH = clamp(dot(n, h), 0.0, 1.0);
		float LdotH = clamp(dot(l, h), 0.0, 1.0);
		float VdotH = clamp(dot(v, h), 0.0, 1.0);

		PBRInfo pbrInputs = PBRInfo(NdotL, NdotV, NdotH, LdotH, VdotH, roughness, metallic,	specularEnvironmentR0, specularEnvironmentR90, alphaRoughness, diffuseColor, specularColor);

		vec3 F = specularReflection(pbrInputs);
		float G = geometricOcclusion(pbrInputs);
		float D = microfacetDistribution(pbrInputs);

		vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
		vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
		vec3 directLighting = NdotL * (diffuseContrib + specContrib) ;
	
		color += (directLighting) * radiance;
	}


 //  FragColor = vec4(color.rgb, mask.r );//* 0.35);
   
//	FragColor = vec4(vec3(n), 1);
//	fragout.rgb = color;
	//fragout.rgb = n;

//	FragColor.rgb = diffuseColor;
	//	vec2 decalTexCoord = objectPosition.xy + 0.5;
  //  vec3 diffuseColor = texture(diffuseTexture, decalTexCoord).rgb;
  //  vec4 mask = texture(maskTexture, decalTexCoord);


//	FragColor = vec4(n.rgb, 1 );
//	FragColor = vec4(0, 0, 1, 1 );
	gAlbedo.rgb = (norm * 0.5 + 0.5);	
}