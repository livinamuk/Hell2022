#version 420 core

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

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;

out vec3 attrNormal;
out vec3 attrTangent;
out vec3 attrBiTangent;

uniform bool hasAnimation;
uniform mat4 skinningMats[128];
uniform bool instanced;

out vec3 test;

out mat4 modelOut;

void main()
{
	TexCoord = aTexCoord;	

	vec4 worldPos;
	vec4 totalLocalPos = vec4(0.0);
	vec4 totalNormal = vec4(0.0);
   // mat3 normalMatrix = transpose(inverse(mat3(modelMatrix))); // do this on the cpu. its fucking slow.
	
	vec4 vertexPosition =  vec4(aPos, 1.0);
	vec4 vertexNormal = vec4(aNormal, 0.0);


	float ttest = float(aBoneID[0]);
	test = vec3(ttest / 10.0);

	// Animated
	if (true)
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
		FragPos = (worldPos).xyz;
		gl_Position = projection * view * vec4(FragPos, 1);
	}

	attrNormal = (model * vec4(Normal, 0.0)).xyz;
	attrTangent = (model * vec4(aTangent, 0.0)).xyz;
	attrBiTangent = (model * vec4(aBitangent, 0.0)).xyz;
}