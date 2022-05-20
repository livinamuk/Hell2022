#version 420 core

//out vec4 FragColor;
layout (location = 0) out vec4 gBaseColor;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gRMA;

layout (binding = 0) uniform sampler2D depthTexture;
layout (binding = 1) uniform sampler2D normalTexture;
layout (binding = 2) uniform sampler2D BLOOD_TEXTURE;


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


float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

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
//	if (d > 0.075)	
//		discard;

        
	vec4 bloodTexture  = texture(BLOOD_TEXTURE, vec2(TexCoords.s, TexCoords.t));

    if (bloodTexture.a < 0.1)
        discard;

    gBaseColor.rgb = vec3(1,0,0);
    gBaseColor.a = bloodTexture.a;

    vec4 res = vec4(0); 
    res.a = saturate(bloodTexture.a * 2);

    vec4 mask = bloodTexture;

    vec3 _TintColor = vec3(0.32, 0, 0);
    float alphaMask = (mask.a  * 20) * res.a;
    alphaMask = clamp(alphaMask, 0, 1);
    float colorMask = (mask.a * 5) * res.a;
    colorMask = clamp(colorMask , 0, 1);
    colorMask = mask.a * 0.5;
    res.rgb = mix(_TintColor.rgb, _TintColor.rgb * 0.2, mask.z * colorMask * 0.75);


    gRMA.rgb = vec3(0.125 , 0.25, 1);
    gRMA.rgb = vec3(0.05 , 0.4, 1);

    gBaseColor.rgb = vec3(res);
    gBaseColor.a = 0.75;
    gBaseColor.r *= 1.5;
}