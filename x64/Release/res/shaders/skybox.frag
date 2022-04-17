#version 420 core

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform samplerCube cubeMap;

in vec3 TexCoords;

void main()
{		
    FragColor = texture(cubeMap, TexCoords);
    
    //vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    // HDR tonemap and gamma correct
    //envColor = envColor / (envColor + vec3(1.0));
    //envColor = pow(envColor, vec3(1.0/2.2)); 
    //FragColor = vec4(envColor, 1.0);

	
// FragColor = vec4(vec3(textureLod(environmentMap, WorldPos, 2).rgb), 1);
 //FragColor = vec4(vec3(textureLod(cubeMap, WorldPos, 0.5).rgb), 1);
    
	//FragColor = vec4(1, 0, 0, 1);
}
