#version 420 core

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D FINAL_TEXTURE; // ligthing + DOF
layout (binding = 1) uniform sampler2D blur0;
layout (binding = 2) uniform sampler2D blur1;
layout (binding = 3) uniform sampler2D blur2;
layout (binding = 4) uniform sampler2D blur3;

//layout (binding = 1) uniform sampler2D ALB_TEXTURE;  // actually not used    
//layout (binding = 2) uniform sampler2D NRM_TEXTURE;
//layout (binding = 3) uniform sampler2D EMISSIVE_TEXTURE;    

in vec2 TexCoords;
in vec2 SplitscreenAdjustedCoords;

uniform float u_timeSinceDeath;
uniform float u_time;
  
void contrastAdjust( inout vec4 color, in float c) {
    float t = 0.5 - c * 0.5; 
    color.rgb = color.rgb * c + t;
}



#define HASHSCALE1 443.8975


float hash13(vec3 p3)
{
	p3  = fract(p3 * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 ApplyGrain( vec2 vUV, vec3 col, float amount )
{
    float h = hash13( vec3(vUV, u_time) );    
    col *= (h * 2.0 - 1.0) * amount + (1.0f -amount);
    return col;
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
	// Chromatic Abz
    vec2 destCoord = TexCoords;
	int sampleCount = 25;
    float blur = 0.25; 
    float falloff = 05.0; 
    vec2 direction = normalize(SplitscreenAdjustedCoords - 0.5);
    vec2 velocity = direction * blur * pow(length(SplitscreenAdjustedCoords - 0.5), falloff);
	float inverseSampleCount = 1.0 / float(sampleCount);     
    mat3x2 increments = mat3x2(velocity * 1.0 * inverseSampleCount,
                               velocity * 2.0 * inverseSampleCount,
                               velocity * 4.0 * inverseSampleCount);
    vec3 accumulator = vec3(0);
    mat3x2 offsets = mat3x2(0); 
    
    for (int i = 0; i < sampleCount; i++) {
        accumulator.r += texture(FINAL_TEXTURE, destCoord + offsets[0]).r; 
        accumulator.g += texture(FINAL_TEXTURE, destCoord + offsets[1]).g; 
        accumulator.b += texture(FINAL_TEXTURE, destCoord + offsets[2]).b; 
        
        offsets -= increments;
    }
	vec3 chromo_ABZ_color = vec3(accumulator / float(sampleCount));



    // Film grain
    float amount = 0.075;
	chromo_ABZ_color = ApplyGrain(destCoord, chromo_ABZ_color, amount); 




	vec4 color = vec4(u_timeSinceDeath, u_timeSinceDeath, u_timeSinceDeath, 0);
	
	color.r =  min(u_timeSinceDeath, 0.5);
	color.g =  min(u_timeSinceDeath, 0.05);
	color.b =  min(u_timeSinceDeath, 0.05);
	
	//vec4 baseColor = texture(ALB_TEXTURE, TexCoords) + color;
	//FragColor = texture(LIGHTING_TEXTURE, TexCoords) + color;

	FragColor.xyz = chromo_ABZ_color.xyz; 
	//FragColor += baseColor * 0.1;


	// Make it red
	if (u_timeSinceDeath > 0) {	
		FragColor.g *= 0.25;
		FragColor.b *= 0.25;
		contrastAdjust(FragColor, 1.2);
		//FragColor.r = FragColor.r * FragColor.r;
	}
	// Darken it after 3 seconds
	float waitTime = 3;
	if (u_timeSinceDeath > waitTime) {
		float val = (u_timeSinceDeath - waitTime) * 10; 
		FragColor.r -= val;
		//FragColor.g = 0;
		//FragColor.b = 0;
	}

    
    vec3 color2 = FragColor.rgb;


    // HDR tonemapping
//   color2 = color2 / (color2 + vec3(1.0));
    // gamma correct
 // color2 = pow(color2, vec3(1.0/2.2)); 

    FragColor.rgb = color2;

    


     // Add light bulbs	
    vec3 blurBulbs = texture(blur0, SplitscreenAdjustedCoords).rgb;
	blurBulbs += texture(blur1, SplitscreenAdjustedCoords).rgb;
	blurBulbs += texture(blur2, SplitscreenAdjustedCoords).rgb;
	blurBulbs += texture(blur3, SplitscreenAdjustedCoords).rgb;
    FragColor.rgb += blurBulbs;


} 