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

uniform float far_plane;

uniform float screenWidth;
uniform float screenHeight;

uniform vec3 camPos;

  
// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 lightPos, vec3 fragPos, vec3 viewPos, float NdotL)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    // shadow /= (samples * samples * samples);
    float shadow = 0.0;
    float bias = 0.0215;
    int samples = 10;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 150.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(ShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
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

float CaclulateAttenuation(vec3 worldPos, vec3 lightPosition, float radius, float magic) {
    float dist = length(worldPos - lightPosition);
    float num = saturate(1 - (dist / radius) * (dist / radius) * (dist / radius) * (dist / radius));
    return num * num / (dist * dist + magic);
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







void main2()
{
	vec3 albedo = pow(texture(ALB_Texture, TexCoords).rgb, vec3(2.2));
	float roughness = texture(RMA_Texture, TexCoords).r;	
	float metallic  = texture(RMA_Texture, TexCoords).g;
	float ao = texture(RMA_Texture, TexCoords).b;
	vec3 n = normalize(texture(NRM_Texture, TexCoords).rgb);

	vec2 gScreenSize = vec2(screenWidth, screenHeight);
    vec2 TexCoord = gl_FragCoord.xy / gScreenSize;
	vec2 TexCoords = TexCoord;

    

    // Get the Fragment Z position (from the depth buffer)
    float z = texture(Depth_Texture, vec2(TexCoord.s, TexCoord.t)).x * 2.0f - 1.0f;
    vec4 clipSpacePosition = vec4(vec2(TexCoord.s, TexCoord.t) * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverseProjection * clipSpacePosition;
    
	// Get the Fragment XYZ position (perspective division, via it's depth value)
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverseView * viewSpacePosition;
    vec3 WorldPos = worldSpacePosition.xyz;

    // Light data
	Light light = lights[lightIndex];

	vec3 lightPosition = light.position_radius.xyz;
	float lightRadius = light.position_radius.w;
	vec3 lightColor = light.color_strength.xyz;
	float lightStrength = light.color_strength.w;
	float lightMagic = light.magic_padding.x;

	float attenuation = CaclulateAttenuation(WorldPos, lightPosition, lightRadius, lightMagic) * lightStrength;
	vec3 radiance = lightColor * attenuation;

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

    vec3 viewPos = vec3(inverseView * vec4(0, 0, 0, 1));

	vec3 v = normalize(viewPos - WorldPos);			// Vector from surface point to camera
	
    vec3 l = normalize(lightPosition - WorldPos);  // Vector from surface point to light
	vec3 h = normalize(v + l);						// Half vector between both l and v            
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

    // Spherical Harmonics
	mat3 shR, shG, shB;
    #define SH_FILL(x, y) \
    { \
        vec3 samp = texelFetch(Env_LUT, ivec2(x, y), 0).rgb; \
        shR[x][y] = samp.r; \
        shG[x][y] = samp.g; \
        shB[x][y] = samp.b; \
    }
    SH_FILL(0, 0)
    SH_FILL(0, 1)
    SH_FILL(0, 2)
    SH_FILL(1, 0)
    SH_FILL(1, 1)
    SH_FILL(1, 2)
    SH_FILL(2, 0)
    SH_FILL(2, 1)
    SH_FILL(2, 2)
    #undef SH_FILL		
	mat3 shRD = shDiffuseConvolution(shR);
    mat3 shGD = shDiffuseConvolution(shG);
    mat3 shBD = shDiffuseConvolution(shB);

	// Indirect diffuse
	vec3 irradiance = shToColor(shRD, shGD, shBD, n); 
	vec3 indirectDiffuse = irradiance * diffuseColor;
    
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;	
    //vec3 indirectLighting = (kD * indirectDiffuse + indirectSpecular) * ao;
	vec3 indirectLighting = (kD * indirectDiffuse) * ao ; // no specular!!!!!!!!!!!!

	float shadowFactor = ShadowCalculation(lightPosition, WorldPos, viewPos, NdotL); 

	float shadow = (1 - shadowFactor) ;// (NdotL);
		
	indirectLighting *= attenuation;// * 10 ;
	
	indirectLighting *= vec3(shadow+0.1);// * NdotL;

	color = ((shadow * directLighting) + indirectLighting) * attenuation;
	
//	vec3 v = normalize(viewPos - WorldPos);    // Vector from surface point to camera
	
   // vec3 l = normalize(lightPosition - WorldPos);   // Vector from surface point to light
//	vec3 h = normalize(v + l);    // Half vector between both


	float _Power = 3;
	float _Scale = 1;
	float _Distortion = -0.1;


	vec3 L = l;//gi.light.dir;
    vec3 V = v;//viewDir;
    vec3 N = n;//float3 N = s.Normal;
    vec3 H = normalize(L + N * _Distortion);
    
	
	float I = pow(saturate(dot(V, -H)), _Power) * _Scale;
   
   // Final add

   // color =  color +  (lightColor * I);

   
	vec3 transmit = vec3(0.0);

	vec3 light_pos = lightPosition;
	vec3 light_dir = normalize(WorldPos - light_pos); 
	light_dir = normalize(light_pos - WorldPos); 

	//uvec2 shadow_handle = lightList[i].shadow_handle;

	//ShadowResult sr = shadowCalculation(frag_pos, light_pos, samplerCube(shadow_handle));     

	float distance_scale = 4;
	float translucency = 10;

//	float S = sr.delta * distance_scale;
	float S = distance_scale;
	float irradiance2 = max(0.3 + dot(N, light_dir), 0.0);
	vec3 transmittance = T(S) * lightColor * translucency * irradiance2;

		transmit += transmittance;
	
	//color += (transmit + specContrib) * albedo;

//	color = lights[3].position_radius.rgb;

	//color *= albedo;
	//color = vec3(shadow);
    FragColor = vec4(color, 1.0);
   // FragColor = vec4(WorldPos, 1.0);
} 









float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


// your inverse view and inverse projection aren't being sent

void main()
{
    // get light uniform data
	Light light = lights[0];
	vec3 lightPos   = light.position_radius.xyz;
    vec3 lightColor = light.color_strength.xyz;
    float lightRadius = light.position_radius.w;
    float lightStrength = light.color_strength.w;
    float lightMagic = light.magic_padding.w;

	vec3 albedo = pow(texture(ALB_Texture, TexCoords).rgb, vec3(2.2));
	float roughness = texture(RMA_Texture, TexCoords).r;	
	float metallic  = texture(RMA_Texture, TexCoords).g;
	float ao = texture(RMA_Texture, TexCoords).b;
	//vec3 Normal = texture(NRM_Texture, TexCoords).rgb;
    	
    vec2 TexCoord = vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight);
    

   // if (player == 1)
    //    TexCoord.y = TexCoord.y / 2;

    // Get the Fragment Z position (from the depth buffer)
    float z = texture(Depth_Texture, vec2(TexCoord.s, TexCoord.t)).x * 2.0f - 1.0f;

    vec4 clipSpacePosition = vec4(vec2(TexCoord.s, TexCoord.t) * 2.0 - 1.0, z, 1.0);

    if (player == 1)      
        clipSpacePosition = vec4(vec2(TexCoord.s * 2.0 - 1.0, ((TexCoord.t * 2 + -1) * 2.0 - 1.0) ), z, 1.0);
    if (player == 2)      
        clipSpacePosition = vec4(vec2(TexCoord.s * 2.0 - 1.0, ((TexCoord.t * 2 + 0) * 2.0 - 1.0) ), z, 1.0);


/*       

        -1 to 1
        looks like its doing
        -1 to 2

*/

    vec4 viewSpacePosition = inverse(projection) * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inverse(view) * viewSpacePosition;
    
    vec3 WorldPos = worldSpacePosition.xyz;

    vec3 testt = clipSpacePosition.xyz;

  /*  vec3 clip_space_position = vec3(TexCoord, depth) * 2.0 - vec3(1.0);

    mat4 biased_inverse_projection_matrix = inverse(projection);
    vec4 view_position = vec4(biased_inverse_projection_matrix[0][0] * TexCoords.x + biased_inverse_projection_matrix[3][0],
                          biased_inverse_projection_matrix[1][1] * TexCoords.y + biased_inverse_projection_matrix[3][1],
                          -1.0,
                          biased_inverse_projection_matrix[2][3] * depth + biased_inverse_projection_matrix[3][3]);
*/

    //vec3 WorldPos = (view_position.xyz / view_position.w);
   // WorldPos = WorldPos2;
    
    vec3 test = vec3(WorldPos);

    vec3 N = texture(NRM_Texture, TexCoords).rgb * 2 - 1;//normalize(Normal);
    //vec3 viewPos = view[3].xyz;
    vec3 V = normalize(camPos - WorldPos);
   // vec3 V = normalize(viewPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);


    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 1; ++i) 
    {
        // calculate per-light radiance
        
        vec3 L = normalize(lightPos - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos - WorldPos);

        
        float attenuation = CaclulateAttenuation(WorldPos, lightPos, lightRadius, lightMagic) * lightStrength;
        vec3 radiance = lightColor * attenuation; 
     //   radiance *= 10;



       // vec3 L = normalize(lightPositions[i] - WorldPos);
       // vec3 H = normalize(V + L);
       // float distance = length(lightPositions[i] - WorldPos);
       //float attenuation = 1.0 / (distance * distance);
        //vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
           

        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        


        // add to outgoing radiance Lo
        Lo = (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
   
   
        float shadow = ShadowCalculation(lightPos, WorldPos, camPos, NdotL);

       // Lo *= shadow;
   	    //vec3 directLighting = NdotL * (diffuseContrib + specContrib) * radiance;
   
   }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 
    
    FragColor = vec4(color, 1.0);
 //FragColor = vec4(clipSpacePosition.y, 0, 0, 1.0);
}