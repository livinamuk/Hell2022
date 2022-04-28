#version 420 core

//out vec4 FragColor;
layout (location = 0) out vec4 gBaseColor;
layout (location = 1) out vec4 gNormal;

layout (binding = 0) uniform sampler2D depthTexture;
layout (binding = 1) uniform sampler2D normalTexture;
layout (binding = 2) uniform sampler2D diffuseTexture;
layout (binding = 3) uniform sampler2D maskTexture;


in vec2 SplitscreenAdjustedCoords;

uniform mat4 inverseProjectionMatrix;
uniform mat4 inverseViewMatrix;
uniform mat4 model;
uniform float screenWidth;
uniform float screenHeight;
uniform float fullscreenWidth;
uniform float fullscreenHeight;
uniform vec3 targetPlaneSurfaceNormal;
uniform int writeRoughnessMetallic;

uniform vec3 u_CameraFront;
uniform vec3 u_ViewPos;

uniform vec3 u_decalNormal;
uniform vec3 u_decalPos;

uniform int u_playerIndex;

in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

const float EPS = 1e-4;
const float PI = 3.14159265359;

in vec2 TexCoords;
in vec3 worldPosition;

uniform float u_alphaModifier;

void main()
{    
    // Get the Fragment Z position (from the depth buffer)
    vec2 depthCoords = gl_FragCoord.xy / vec2(fullscreenWidth, fullscreenHeight);
    float z = texture(depthTexture, vec2(depthCoords.s, depthCoords.t)).x * 2.0f - 1.0f;
    vec4 clipSpacePosition = vec4(vec2(depthCoords.s, depthCoords.t) * 2.0 - 1.0, z, 1.0);
    
    if (u_playerIndex == 1)      
       clipSpacePosition = vec4(vec2(depthCoords.s * 2.0 - 1.0, ((depthCoords.t * 2 + -1) * 2.0 - 1.0) ), z, 1.0);
    else if (u_playerIndex == 2)      
        clipSpacePosition = vec4(vec2(depthCoords.s * 2.0 - 1.0, ((depthCoords.t * 2 + 0) * 2.0 - 1.0) ), z, 1.0);
    
    vec4 viewSpacePosition = inverseProjectionMatrix * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverseViewMatrix * viewSpacePosition;
    vec3 WorldPos = worldSpacePosition.xyz;
	
	float d = length(WorldPos - u_decalPos);
	if (d > 0.075)	
		discard;
        
	gBaseColor  = texture(diffuseTexture, vec2(TexCoords.s, TexCoords.t));
    gBaseColor.a *= u_alphaModifier;

    
	vec4 mask  = texture(maskTexture, vec2(TexCoords.s, TexCoords.t));


    
   // gBaseColor.rgb *= vec3(1-mask);
  //  gBaseColor.a = mask.a;
 //   gBaseColor.a = mask.a;

  //  gNormal.rgb = vec3(mask.rgb);//
 //   gNormal.a = 1;;//, 1);

gNormal.rgb = vec3(0,0,0);
gNormal.a = mask.a;

//layout (binding = 3) uniform sampler2D maskTexture;
}