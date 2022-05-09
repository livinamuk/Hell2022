#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in int aMaterialID;

uniform mat4 model;
uniform int u_TEXCOORD_FLAG;

out vec3 WorldPos;
out vec2 gTexCoord;
out vec3 gNormal;
out float gMaterialID;
out mat3 gTBN;

void main()
{
	gMaterialID = aMaterialID;
	WorldPos = (model * vec4(aPos, 1.0)).xyz;
	gNormal = aNormal;

	gTexCoord = aTexCoord;
	if (u_TEXCOORD_FLAG == 1)								
		gTexCoord = vec2(WorldPos.x, WorldPos.z) * 0.4;		// Floor Regular
	else if (u_TEXCOORD_FLAG == 2)								
		gTexCoord = vec2(WorldPos.z, WorldPos.x) * 0.4;		// Flooor Rotated 90 degrees				
	else if (u_TEXCOORD_FLAG == 4)								
		gTexCoord = vec2(WorldPos.x / 2.6, aTexCoord.y);
	else if (u_TEXCOORD_FLAG == 3)	
		gTexCoord = vec2(WorldPos.z / 2.6, aTexCoord.y);
	
	vec3 attrNormal = (model * vec4(gNormal, 0.0)).xyz;
	vec3 attrTangent = (model * vec4(aTangent, 0.0)).xyz;
	vec3 attrBiTangent = (model * vec4(aBitangent, 0.0)).xyz;

	gTBN = mat3(normalize(attrTangent), normalize(attrBiTangent), normalize(attrNormal));

	gl_Position = vec4(WorldPos, 1);
}