#version 420 core

layout (location = 0) out vec4 gAlbedo;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gRMA;
layout (location = 3) out vec4 gEmissive;

layout (binding = 0) uniform sampler2D ALB_Texture0;
layout (binding = 1) uniform sampler2D NRM_Texture0;
layout (binding = 2) uniform sampler2D RMA_Texture0;
layout (binding = 3) uniform sampler2D E_Texture;
layout (binding = 4) uniform sampler2D ALB_Texture1;
layout (binding = 5) uniform sampler2D NRM_Texture1;
layout (binding = 6) uniform sampler2D RMA_Texture1;

//in vec3 Normal;
in vec2 TexCoord;
in float MaterialID;
in vec3 attrNormal;
in vec3 attrTangent;
in vec3 attrBiTangent;
uniform float u_gunMask;
uniform bool u_hasEmissive;
uniform vec3 u_emissiveColor;

void main()
{
	//////////////////////
	// Read Texture Set //
	//////////////////////

	vec4 ALB;
	vec4 RMA;
	vec4 NRM;
	vec4 EMISSIVE = texture(E_Texture, vec2(TexCoord.x, TexCoord.y));

		if (round(MaterialID) == 1) {
		ALB =  texture(ALB_Texture1, TexCoord);
		RMA =  texture(RMA_Texture1, TexCoord);
		NRM =  texture(NRM_Texture1, TexCoord);
	}
	else {
		ALB =  texture(ALB_Texture0, TexCoord);
		RMA =  texture(RMA_Texture0, TexCoord);
		NRM =  texture(NRM_Texture0, TexCoord);
	}

	NRM.r = clamp(NRM.r, 0, 1);
	NRM.g = clamp(NRM.g, 0, 1);
	NRM.b = clamp(NRM.b, 0, 1);

	//if (ALB.a < 0.4)
	//	discard;


	// normals
	vec3 normal_map = NRM.xyz * 2.0 - 1.0;
	mat3 tbn = mat3(normalize(attrTangent), normalize(attrBiTangent), normalize(attrNormal));
	
	vec3 normal = (tbn * normalize(NRM.rgb * 2.0 - 1.0));
	//gNormal.a = ALB.a;
	gNormal.rgb = (normal) * 0.5 + 0.5;
	
	gAlbedo = ALB;

	gRMA.rgb = RMA.rgb;
	gRMA.a = u_gunMask;
	
	gEmissive.rgb = u_hasEmissive ? EMISSIVE.rgb * u_emissiveColor : vec3(0,0,0);
}  