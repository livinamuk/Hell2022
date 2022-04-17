#version 420 core

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D inputTexture;
layout (binding = 1) uniform sampler2D LIGHTING_TEXTURE;
layout (binding = 2) uniform sampler2D test;

in vec2 TexCoords;

uniform float u_timeSinceDeath;
  
void contrastAdjust( inout vec4 color, in float c) {
    float t = 0.5 - c * 0.5; 
    color.rgb = color.rgb * c + t;
}

void main()
{
	vec4 color = vec4(u_timeSinceDeath, u_timeSinceDeath, u_timeSinceDeath, 0);
	
	color.r =  min(u_timeSinceDeath, 0.5);
	color.g =  min(u_timeSinceDeath, 0.05);
	color.b =  min(u_timeSinceDeath, 0.05);

	FragColor = texture(LIGHTING_TEXTURE, TexCoords) + color;
	vec4 LightingColor = texture(LIGHTING_TEXTURE, TexCoords);

	FragColor = LightingColor;   

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

} 