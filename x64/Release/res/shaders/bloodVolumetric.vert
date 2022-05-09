#version 420 core

layout (location = 0) in vec3 a_Position;
layout (location = 2) in vec2 a_Texcoord;

uniform mat4 u_MatrixProjection;
uniform mat4 u_MatrixView;
uniform mat4 u_MatrixWorld;
uniform mat4 u_MatrixInverseWorld;

uniform mat4 debugMatrix;

layout (binding = 0) uniform sampler2D u_PosTex;
layout (binding = 1) uniform sampler2D u_NormTex;

uniform vec3 u_WorldSpaceCameraPos;

out vec3 v_WorldNormal;
out vec3 v_ViewDir;
out vec3 v_fragPos;

 uniform float  u_Time;

float LinearToGammaSpaceExact (float value)
{
    if (value <= 0.0F)
        return 0.0F;
    else if (value <= 0.0031308F)
        return 12.92F * value;
    else if (value < 1.0F)
        return 1.055F * pow(value, 0.4166667F) - 0.055F;
    else
        return pow(value, 0.45454545F);
}

vec3 LinearToGammaSpace (vec3 linRGB)
{
   return vec3(LinearToGammaSpaceExact(linRGB.r), LinearToGammaSpaceExact(linRGB.g), LinearToGammaSpaceExact(linRGB.b));
}

vec3 ObjSpaceViewDir(vec3 v )
{
    vec3 objSpaceCameraPos = (u_MatrixWorld, vec4(u_WorldSpaceCameraPos.xyz, 1)).xyz;
    return objSpaceCameraPos - v.xyz;
}

void main() 
{
 // float  u_Time = 0.23;

    int u_NumOfFrames = 81;
    int u_Speed = 35;
    int u_BoundingMax = 144;
    int u_BoundingMin = 116;
    vec3 u_HeightOffset = vec3(-45.4, -26.17, 12.7);

    u_BoundingMax = 1;
    u_BoundingMin = -1;
    u_HeightOffset = vec3(0, 0, 0);

    float currentSpeed = 1.0f / (u_NumOfFrames / u_Speed);
    float timeInFrames = ((ceil(fract(-u_Time * currentSpeed) * u_NumOfFrames)) / u_NumOfFrames) + (1.0 / u_NumOfFrames);

    vec3 v = a_Position;
    vec2 uv = a_Texcoord;
    
    timeInFrames = 0.0;// u_Time;//0;//0.23; ;//u_Time;//
  timeInFrames = u_Time;//0;//0.23; ;//u_Time;//
   // timeInFrames = 0.166;

    vec4 texturePos = textureLod(u_PosTex, vec2(uv.x, (timeInFrames + uv.y)), 0);
    vec4 textureNorm = textureLod(u_NormTex, vec2(uv.x, (timeInFrames + uv.y)), 0);

    v_WorldNormal = textureNorm.xzy * 2.0 - 1.0;  
    mat4 modelMatrix = inverse(transpose(u_MatrixWorld));
   // v_WorldNormal =  (u_MatrixWorld * vec4(v_WorldNormal, 0.0)).xyz;

    //v_WorldNormal =  (u_MatrixWorld * vec4(v_WorldNormal.xzy, 0.0)).xyz;   // wrong
    //v_WorldNormal =  (modelMatrix * vec4(v_WorldNormal, 0.0)).xyz;         // right

    
   
  //  v_WorldNormal =  (normalMatrix * (v_WorldNormal)).xyz;         // right
    //v_WorldNormal = normalize(v_WorldNormal * normalMatrix);
    // You should also send your tranformed position to the fragment shader
   // f_position = vec3(mvp * vec4(position, 1.0));
//    v_WorldNormal.x  *= -1;

    mat3 m = mat3(u_MatrixWorld);
    mat3 t = mat3(cross(m[1], m[2]), cross(m[2], m[0]), cross(m[0], m[1])); // adjoint matrix
    v_WorldNormal = t * v_WorldNormal;

    mat3 normalMatrix = mat3(u_MatrixWorld);
    normalMatrix = inverse(normalMatrix);
    normalMatrix = transpose(normalMatrix);

    //v_WorldNormal.xyz = (v_WorldNormal * normalMatrix);

    v_ViewDir = u_WorldSpaceCameraPos.xyz;



   // v_WorldNormal = textureNorm.xzy * 2.0 - 1.0;
 //   v_WorldNormal.x *= -1;
   // v_WorldNormal =  (u_MatrixWorld * vec4(v_WorldNormal.xyz, 0.0)).xyz;
    
    gl_Position =  u_MatrixProjection * u_MatrixView * u_MatrixWorld * vec4(texturePos.xzy, 1.0);

}