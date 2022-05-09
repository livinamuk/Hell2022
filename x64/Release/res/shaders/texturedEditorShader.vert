#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform int u_TEXCOORD_FLAG;

out vec2 TexCoord;

void main()
{
    vec4 worldPos = vec4(model * vec4(aPos, 1.0));
	gl_Position = projection * view * worldPos;

    // tex coords
	TexCoord = aTexCoord;
	if (u_TEXCOORD_FLAG == 1)								
		TexCoord = vec2(worldPos.x, worldPos.z) * 0.4;		// Floor Regular
	if (u_TEXCOORD_FLAG == 2)								
		TexCoord = vec2(worldPos.z, worldPos.x) * 0.4;		// Flooor Rotated 90 degrees

	//TexCoord = aTexCoord;
	//gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}