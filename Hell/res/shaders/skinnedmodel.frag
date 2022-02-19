#version 420 core

layout (location = 0) out vec4 fragout;

layout (binding = 0) uniform sampler2D ALB_Texture;

in vec2 TexCoord;
in vec3 test;

void main()
{
	vec3 color = texture(ALB_Texture, vec2(TexCoord.x, TexCoord.y)).rgb;
	fragout = vec4(color, 1);	
}
