#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneID;
layout (location = 6) in vec4 aBoneWeight;
layout (location = 7) in int aMaterialID;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform int u_TEXCOORD_FLAG;

out vec2 TexCoord;
out vec3 worldPos;
//out vec3 Normal;
out vec3 attrNormal;
out vec3 attrTangent;
out vec3 attrBiTangent;

uniform bool hasAnimation;
uniform mat4 skinningMats[128];
uniform bool instanced;

out vec3 test;

void main()
{
	TexCoord = aTexCoord;	

	vec4 worldPos;
	vec4 totalLocalPos = vec4(0.0);
	vec4 totalNormal = vec4(0.0);
	vec3 Normal;
	
	vec4 vertexPosition =  vec4(aPos, 1.0);
	vec4 vertexNormal = vec4(aNormal, 0.0);

	// Animated
	if (hasAnimation)
	{
		for(int i=0;i<4;i++) 
		{
			mat4 jointTransform = skinningMats[int(aBoneID[i])];
			vec4 posePosition =  jointTransform  * vertexPosition * aBoneWeight[i];
			vec4 worldNormal = jointTransform * vertexNormal * aBoneWeight[i];

			totalLocalPos += posePosition;		
			totalNormal += worldNormal;
		}
		worldPos = model * totalLocalPos;
		Normal = totalNormal.xyz;
		gl_Position = projection * view * worldPos;
	}
	else // Not animated
	{
		worldPos = model * vec4(aPos, 1.0);
		Normal = aNormal;
		gl_Position = projection * view * worldPos;
	}
	
    
	//worldPos = vec4(model * vec4(aPos, 1.0));
	//gl_Position = projection * view * worldPos;

    // tex coords
	TexCoord = aTexCoord;
	if (u_TEXCOORD_FLAG == 1)								
		TexCoord = vec2(worldPos.x, worldPos.z) * 0.4;		// Floor Regular
	if (u_TEXCOORD_FLAG == 2)								
		TexCoord = vec2(worldPos.z, worldPos.x) * 0.4;		// Flooor Rotated 90 degrees
		
	test = vec3(1, 0, 1);
	test = aBitangent + 0.5;
	//Normal = aNormal;
	//Normal = vec3(1, 0, 1);
	attrNormal = (model * vec4(Normal, 0.0)).xyz;
	attrTangent = (model * vec4(aTangent, 0.0)).xyz;
	attrBiTangent = (model * vec4(aBitangent, 0.0)).xyz;

	//TexCoord = aTexCoord;
	//gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}