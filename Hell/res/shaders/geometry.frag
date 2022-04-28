#version 420 core

layout (location = 0) out vec4 gAlbedo;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gRMA;
layout (location = 3) out vec4 gEmissive;

layout (binding = 0) uniform sampler2D ALB_Texture;
layout (binding = 1) uniform sampler2D NRM_Texture;
layout (binding = 2) uniform sampler2D RMA_Texture;

//in vec3 Normal;
in vec2 TexCoord;
in float MaterialID;
in vec3 attrNormal;
in vec3 attrTangent;
in vec3 attrBiTangent;
uniform float u_gunMask;

void main()
{
	//////////////////////
	// Read Texture Set //
	//////////////////////

	vec4 ALB = texture(ALB_Texture, vec2(TexCoord.x, TexCoord.y));
	vec4 RMA = texture(RMA_Texture, vec2(TexCoord.x, TexCoord.y));
	vec4 NRM = texture(NRM_Texture, vec2(TexCoord.x, TexCoord.y));

	NRM.r = clamp(NRM.r, 0, 1);
	NRM.g = clamp(NRM.g, 0, 1);
	NRM.b = clamp(NRM.b, 0, 1);

	//if (ALB.a < 0.4)
	//	discard;

	// normals
	vec3 normal_map = NRM.xyz * 2.0 - 1.0;
	mat3 tbn = mat3(normalize(attrTangent), normalize(attrBiTangent), normalize(attrNormal));
	
	vec3 normal = (tbn * normalize(NRM.rgb * 2.0 - 1.0));
	gNormal.a = ALB.a;
	gNormal.rgb = (normal) * 0.5 + 0.5;
	
	gAlbedo = ALB;

	gRMA.rgb = RMA.rgb;
	gRMA.a = u_gunMask;
}  