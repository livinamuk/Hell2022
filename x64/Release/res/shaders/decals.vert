#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 instanceModelMatrix;

uniform mat4 pv;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

out vec3 attrNormal;
out vec3 attrTangent;
out vec3 attrBiTangent;

out vec3 DecalCenterPosition;
out vec3 DecalNormal;

void main()
{
	mat4 model = instanceModelMatrix;

	// Normal
	attrNormal = (model * vec4(aNormal, 0.0)).xyz;
	attrTangent = (model * vec4(aTangent, 0.0)).xyz;
	attrBiTangent = (model * vec4(aBitangent, 0.0)).xyz;	
	mat3 tbn = mat3(normalize(attrTangent), normalize(attrBiTangent), normalize(attrNormal));
	Normal = normalize(tbn * vec3(0, 0, 1));
	
	// Tex coords
	TexCoords = aTexCoord;	

	// Position
	DecalCenterPosition = vec3(model[3][0],model[3][1],model[3][2]);
	gl_Position = pv * model * vec4(aPos, 1.0);
}


