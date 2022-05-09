#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 instanceMatrix;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

out vec2 TexCoord;
out vec3 worldPos;
out vec3 attrNormal;
out vec3 attrTangent;
out vec3 attrBiTangent;

void main()
{	
	attrNormal = (instanceMatrix * vec4(aNormal, 0.0)).xyz;
	attrTangent = (instanceMatrix * vec4(aTangent, 0.0)).xyz;
	attrBiTangent = (instanceMatrix * vec4(aBitangent, 0.0)).xyz;

	TexCoord = aTexCoord;	

	gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
}