#version 420 core

const int MAX_LIGHTS = 10;
const float EPS = 1e-4;
const float PI = 3.14159265359;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D ALB_Texture;
layout (binding = 1) uniform sampler2D NRM_Texture;
layout (binding = 2) uniform sampler2D RMA_Texture;
layout (binding = 3) uniform sampler2D Depth_Texture;
layout (binding = 4) uniform samplerCube ShadowMap;
layout (binding = 5) uniform sampler2D Env_LUT;
layout (binding = 6) uniform sampler2D BRDF_LUT;
layout (binding = 7) uniform samplerCube IndirectShadowMap;
layout (binding = 8) uniform samplerCube Env_map;

struct Light {
    vec4 position_radius;	// position xyz, radius in w
    vec4 color_strength;    // color xyz, strength in w
    vec4 magic_padding;     // magic in x
};

layout (binding = 0, std140) uniform Matrices {
    mat4 projection;
    mat4 view;
    mat4 inverseProjection;
    mat4 inverseView;
};

layout (binding = 1, std140) uniform Lights {
  Light lights[MAX_LIGHTS];
};

uniform int lightIndex;
uniform int player;
in vec2 TexCoords;
  
uniform vec3 colorTint;
uniform sampler2D fboAttachment;

uniform float shadow_map_far_plane;

uniform float screenWidth;
uniform float screenHeight;

uniform vec3 camPos;

in vec2 vertexShaderTexCoords;

  
// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(samplerCube depthTex, vec3 lightPos, vec3 fragPos, vec3 viewPos, float NdotL)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    // shadow /= (samples * samples * samples);
    float shadow = 0.0;
    float bias = 0.0215; // was 0.0215
    int samples = 10; // was 10
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / shadow_map_far_plane)) / 150.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthTex, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= shadow_map_far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
    
    return shadow;
}


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



/***********************************************************************/
// Spherical Harmonics Lib

// Constants, see here: http://en.wikipedia.org/wiki/Table_of_spherical_harmonics
#define k01 0.2820947918 // sqrt(  1/PI)/2
#define k02 0.4886025119 // sqrt(  3/PI)/2
#define k03 1.0925484306 // sqrt( 15/PI)/2
#define k04 0.3153915652 // sqrt(  5/PI)/4
#define k05 0.5462742153 // sqrt( 15/PI)/4
#define k06 0.5900435860 // sqrt( 70/PI)/8
#define k07 2.8906114210 // sqrt(105/PI)/2
#define k08 0.4570214810 // sqrt( 42/PI)/8
#define k09 0.3731763300 // sqrt(  7/PI)/4
#define k10 1.4453057110 // sqrt(105/PI)/4

// Y_l_m(s), where l is the band and m the range in [-l..l]
float SphericalHarmonic( in int l, in int m, in vec3 n )
{
	//----------------------------------------------------------
	if( l==0 )          return   k01;

	//----------------------------------------------------------
	if( l==1 && m==-1 ) return  -k02*n.y;
	if( l==1 && m== 0 ) return   k02*n.z;
	if( l==1 && m== 1 ) return  -k02*n.x;

	//----------------------------------------------------------
	if( l==2 && m==-2 ) return   k03*n.x*n.y;
	if( l==2 && m==-1 ) return  -k03*n.y*n.z;
	if( l==2 && m== 0 ) return   k04*(3.0*n.z*n.z-1.0);
	if( l==2 && m== 1 ) return  -k03*n.x*n.z;
	if( l==2 && m== 2 ) return   k05*(n.x*n.x-n.y*n.y);
	//----------------------------------------------------------

	return 0.0;
}

mat3 shEvaluate(vec3 n) {
	mat3 r;
	r[0][0] =  SphericalHarmonic(0,  0, n);
	r[0][1] = -SphericalHarmonic(1, -1, n);
	r[0][2] =  SphericalHarmonic(1,  0, n);
	r[1][0] = -SphericalHarmonic(1,  1, n);

	r[1][1] =  SphericalHarmonic(2, -2, n);
	r[1][2] = -SphericalHarmonic(2, -1, n);
	r[2][0] =  SphericalHarmonic(2,  0, n);
	r[2][1] = -SphericalHarmonic(2,  1, n);
	r[2][2] =  SphericalHarmonic(2,  2, n);
	return r;
}

// Recovers the value of a SH function in the direction dir.
float shUnproject(mat3 functionSh, vec3 dir)
{
	mat3 sh = shEvaluate(dir);
	return
		dot(functionSh[0], sh[0]) +
		dot(functionSh[1], sh[1]) +
		dot(functionSh[2], sh[2]);
}

const vec3 convCoeff = vec3(1.0, 2.0/3.0, 1.0/4.0);
mat3 shDiffuseConvolution(mat3 sh) {
	mat3 r = sh;

	r[0][0] *= convCoeff.x;

	r[0][1] *= convCoeff.y;
	r[0][2] *= convCoeff.y;
	r[1][0] *= convCoeff.y;

	r[1][1] *= convCoeff.z;
	r[1][2] *= convCoeff.z;
	r[2][0] *= convCoeff.z;
	r[2][1] *= convCoeff.z;
	r[2][2] *= convCoeff.z;

	return r;
}

vec3 shToColor(mat3 shR, mat3 shG, mat3 shB, vec3 rayDir) {
	vec3 rgbColor = vec3(
		shUnproject(shR, rayDir),
		shUnproject(shG, rayDir),
		shUnproject(shB, rayDir));

	// A "max" is usually recomended to avoid negative values (can happen with SH)
	rgbColor = max(vec3(0.0), vec3(rgbColor));
	return rgbColor;
}

    
float minMax(float d) {
    return max(abs(d),0.0001);
}

float sqr(float x) {
    return x * x;
}



float CalculateAttenuation(
    in vec3  surfNorm,
    in vec3  vector,
    in float k)
{
    float d = max(dot(surfNorm, vector), 0.0);
 	return (d / ((d * (1.0 - k)) + k));
}

/**
 * GGX/Schlick-Beckmann attenuation for analytical light sources.
 */
float CalculateAttenuationAnalytical(
    in vec3  surfNorm,
    in vec3  toLight,
    in vec3  toView,
    in float roughness)
{
    float k = pow((roughness + 1.0), 2.0) * 0.125;

    // G(l) and G(v)
    float lightAtten = CalculateAttenuation(surfNorm, toLight, k);
    float viewAtten  = CalculateAttenuation(surfNorm, toView, k);

    // Smith
    return (lightAtten * viewAtten);
}

/**
 * Calculates the Fresnel reflectivity.
 */
vec3 CalculateFresnel(
    in vec3 surfNorm,
    in vec3 toView,
    in vec3 fresnel0)
{
	float d = max(dot(surfNorm, toView), 0.0);
    float p = ((-5.55473 * d) - 6.98316) * d;

    // Fresnel-Schlick approximation
    return fresnel0 + ((1.0 - fresnel0) * pow(1.0 - d, 5.0));
    // modified by Spherical Gaussian approximation to replace the power, more efficient
    return fresnel0 + ((1.0 - fresnel0) * pow(2.0, p));
}

/*float CaclulateAttenuation(vec3 worldPos, vec3 lightPosition, float radius, float magic) {
    float dist = length(worldPos - lightPosition);
    float num = saturate(1 - (dist / radius) * (dist / radius) * (dist / radius) * (dist / radius));
    return num * num / (dist * dist + magic);
}*/

float CaclulateAttenuation(vec3 worldPos, vec3 lightPosition, float radius, float magic) {


    float dist = length(worldPos - lightPosition);

	float a = 0.015;
	float b = 0.175;


	//return 1.0 / (1.0 + a*dist + b*dist*dist);

	
	//float att = clamp(1.0 - dist*dist/(radius*radius), 0.0, 1.0);
	//att *= att;
	//return att;

    float num = saturate(1 - (dist / radius) * (dist / radius) * (dist / radius) * (dist / radius));
    //return num * num / (dist * dist + magic);
	return num * num / (dist * dist);



}


// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs) {
	return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs) {
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
float microfacetDistribution(PBRInfo pbrInputs) {
	float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
	float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
	return roughnessSq / (PI * f * f);
}

vec3 diffuse(PBRInfo pbrInputs) {
	return pbrInputs.diffuseColor / PI;
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

vec3 T(float s) { // It doesn't matter, positive or negative value.
  return vec3(0.233, 0.455, 0.649) * exp(-s * s / 0.0064) +
         vec3(0.1,   0.336, 0.344) * exp(-s * s / 0.0484) +
         vec3(0.118, 0.198, 0.0)   * exp(-s * s / 0.187)  +
         vec3(0.113, 0.007, 0.007) * exp(-s * s / 0.567)  +
         vec3(0.358, 0.004, 0.0)   * exp(-s * s / 1.99)   +
         vec3(0.078, 0.0,   0.0)   * exp(-s * s / 7.41);
}


float SRGBToLinearVal(float sRGBVal) {
    return sRGBVal <= 0.04045f
        ? sRGBVal / 12.92f
        : pow((sRGBVal + 0.055f) / 1.055f, 2.4f);
}


float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	#ifdef MANUAL_SRGB
	#ifdef SRGB_FAST_APPROXIMATION
	vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
	#else //SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	#endif //SRGB_FAST_APPROXIMATION
	return vec4(linOut,srgbIn.w);;
	#else //MANUAL_SRGB
	return srgbIn;
	#endif //MANUAL_SRGB
}

vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

float uboParamsExposure = 1;
float uboParamsGamma = 1;

vec4 tonemap(vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * uboParamsExposure);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / uboParamsGamma)), color.a);
}

vec3 getIBLContribution(PBRInfo pbrInputs, vec3 n, vec3 reflection)
{
	/*float lod = (pbrInputs.perceptualRoughness * uboParams.prefilteredCubeMipLevels);
	// retrieve a scale and bias to F0. See [1], Figure 3
	vec3 brdf = (texture(samplerBRDFLUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
	vec3 diffuseLight = SRGBtoLINEAR(tonemap(texture(samplerIrradiance, n))).rgb;

	vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(prefilteredMap, reflection, lod))).rgb;

	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

	// For presentation, this allows us to disable IBL terms
	// For presentation, this allows us to disable IBL terms
	diffuse *= uboParams.scaleIBLAmbient;
	specular *= uboParams.scaleIBLAmbient;

	return diffuse + specular;*/

	vec3 brdf = (texture(BRDF_LUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
	vec3 diffuseLight = tonemap(texture(Env_map, n)).rgb;
	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	return diffuse;
}










void main()
{
   // get light uniform data
	Light light = lights[lightIndex];
	vec3 lightPos   = light.position_radius.xyz;
    vec3 lightColor = light.color_strength.xyz;
    float lightRadius = light.position_radius.w;
    float lightStrength = light.color_strength.w;
    float lightMagic = light.magic_padding.x;
	
	lightStrength *= 1.25;
	lightMagic = 0.5;
	lightRadius *= 1.125;
//	lightMagic *= 0.5;
	
    // Reconstruc pos from depth
    vec2 TexCoord = vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight);   
    float z = texture(Depth_Texture, vec2(TexCoord.s, TexCoord.t)).x * 2.0f - 1.0f;
    vec4 clipSpacePosition = vec4(vec2(TexCoord.s, TexCoord.t) * 2.0 - 1.0, z, 1.0);
    if (player == 1)      
        clipSpacePosition = vec4(vec2(TexCoord.s * 2.0 - 1.0, ((TexCoord.t * 2 + -1) * 2.0 - 1.0) ), z, 1.0);
    if (player == 2)      
        clipSpacePosition = vec4(vec2(TexCoord.s * 2.0 - 1.0, ((TexCoord.t * 2 + 0) * 2.0 - 1.0) ), z, 1.0);
    vec4 viewSpacePosition = inverse(projection) * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverse(view) * viewSpacePosition;    
    vec3 WorldPos = worldSpacePosition.xyz;

    // Get textures
	vec4 RMA = texture(RMA_Texture, TexCoords);
	vec3 albedo = pow(texture(ALB_Texture, TexCoords).rgb, vec3(2.2));	
    vec3 n = texture(NRM_Texture, TexCoords).rgb * 2 - 1;//normalize(Normal);

	float ao = RMA.a;

	float perceptualRoughness = RMA.r;
	float metallic = RMA.g;
	vec3 diffuseColor;
	vec3 baseColor = albedo;

	vec3 f0 = vec3(0.04);

	//metallic += 0.01;

	diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;


	//diffuseColor = vec3(1 -  metallic);

	//diffuseColor *= -1;
	//diffuseColor += 1;

	float alphaRoughness = perceptualRoughness * perceptualRoughness;
	vec3 specularColor = mix(f0, baseColor.rgb, metallic);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	n = normalize(n);
	vec3 v = normalize(camPos - WorldPos);    // Vector from surface point to camera
	vec3 l = normalize(lightPos - WorldPos);     // Vector from surface point to light
	vec3 h = normalize(l+v);                        // Half vector between both l and v
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
		perceptualRoughness,
		metallic,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor
	);

	// Calculate the shading terms for the microfacet specular shading model
	vec3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);

	// Calculation of analytical lighting contribution
	vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
	vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	vec3 color = NdotL * lightColor * (diffuseContrib + specContrib);

	// Calculate lighting contribution from image based lighting source (IBL)
	vec3 brdf = (texture(BRDF_LUT, vec2(pbrInputs.NdotV, 1.0 - metallic))).rgb;
	//vec3 diffuseLight = tonemap(texture(Env_map, n)).rgb;
	vec3 diffuseLight = (texture(Env_map, n)).rgb;
	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;

	int lod = 3;
	//vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(Env_map, reflection, lod))).rgb;
	vec3 specularLight = textureLod(Env_map, reflection, lod).rgb;
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

	vec3 indirectLighting = diffuse;
	
	lightStrength *= 1.05;
	lightRadius *= 1;
	lightMagic *= 0.5;
	float attenuation = CaclulateAttenuation(WorldPos, lightPos, lightRadius, lightMagic) * lightStrength;
	vec3 radiance =

	//lightStrength = 1;
	//lightRadius = 15;
 lightColor * attenuation;

	//vec3 indirectLighting = getIBLContribution(pbrInputs, n, reflection);
	//color += indirectLighting;

	//specular *= 0.25;

	//#ifdef SRGB_FAST_APPROXIMATION
	//color = pow(color.xyz,vec3(2.2));
	//specular = pow(specular.xyz,vec3(2.2));
	//indirectLighting = pow(indirectLighting.xyz,vec3(2.2));
	//#else //SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045),specular.xyz);
	//specular = mix( specular.xyz/vec3(12.92), pow((specular.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	//#endif //SRGB_FAST_APPROXIMATION


	vec3 diffuseColor2 = albedo * (vec3(1.0) - f0);
	//diffuseColor2 *= 1.0 - ao;
	//diffuseColor2 *= perceptualRoughness;

	//diffuseColor2 *= 0;// 0.125;


	float gunMask = RMA.a;
    float indirectShadow = 1 - ShadowCalculation(IndirectShadowMap, lightPos, WorldPos, camPos, NdotL);
	float shadow = 1 - ShadowCalculation(ShadowMap, lightPos, WorldPos, camPos, NdotL);
	shadow += indirectShadow * gunMask;
	
	//float shadow = (1 - ShadowCalculation(ShadowMap, lightPos, WorldPos, camPos, NdotL)) ;//* gunMask;

	//specular = SRGBtoLINEAR(specular.rgb);
	indirectLighting *= indirectShadow * 0.25;
//	indirectLighting += diffuseColor2 * indirectShadow * lightColor * ( metallic)*  0.125;
	specular *= indirectShadow * 0.1;



	color *= shadow;

	FragColor.rgb = vec3(color + indirectLighting + specular) * attenuation;





	 vec3 vLightDir = lightPos - WorldPos;
     vec3 L = normalize(vLightDir);
	 vec3 N = n;
	 float thicknessDistortion = 0.1;
	 int thicknessPower = 2;
	 float thicknessScale = 16;
	 vec3 V = v;
	 float thicknessAmbient = 0.04;

	vec3 thicknessColor = vec3(1.0, 1.0, 1.0);
    vec3 thickness = thicknessColor * 1;//texture2D(thicknessMap, vUv * thicknessRepeat).r;

     float lightDist = length(vLightDir);
     float lightAtten = attenuation;//punctualLightIntensityToIrradianceFactor(lightDist, pointLight.distance, pointLight.decay);
        
     vec3 LTLight = normalize(L + (N * thicknessDistortion));
     float LTDot = pow(saturate(dot(V, -LTLight)), thicknessPower) * thicknessScale;
     vec3 LT = lightAtten * (LTDot + thicknessAmbient) * thickness;
	// FragColor.rgb += lightColor * LT * shadow;
	// FragColor.rgb = vec3( diffuseColor2  );// * attenuation;
	// FragColor.rgb = vec3(specular);
//	 FragColor.rgb = RMA.rgb;
}