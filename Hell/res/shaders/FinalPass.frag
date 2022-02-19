#version 420 core

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D inputTexture;

in vec2 TexCoords;

uniform float u_timeSinceDeath;
  
void main()
{
	vec4 color = vec4(u_timeSinceDeath, u_timeSinceDeath, u_timeSinceDeath, 0);
	
	color.r =  min(u_timeSinceDeath, 0.5);
	color.g =  min(u_timeSinceDeath, 0.05);
	color.b =  min(u_timeSinceDeath, 0.05);


	//color.g =  1 -  u_timeSinceDeath;
//	color.b =  1 -  u_timeSinceDeath;

	FragColor = texture(inputTexture, TexCoords) + color;
   //FragColor = vec4(1, 0, 0, 1);

	float waitTime = 3;
	if (u_timeSinceDeath > waitTime) 
	{
		float val = (u_timeSinceDeath - waitTime) * 10; 
		FragColor *= vec4(1 - val, 1 - val, 1 - val, 0);
	}
} 