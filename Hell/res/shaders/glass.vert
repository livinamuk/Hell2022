#version 440 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBinormal;

uniform mat4 model;
uniform mat4 viewMatrix;
uniform mat4 projection;

out vec3 WorldPos;
out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;

void main()
{
    TexCoord = aTexCoord;	
    WorldPos = vec4(model * vec4(aPosition, 1.0)).xyz;
    gl_Position = projection * viewMatrix * model * vec4(aPosition, 1.0);	

	mat3 normalMatrix = transpose(inverse(mat3(model))); 
	Normal= normalize(normalMatrix * aNormal);
}