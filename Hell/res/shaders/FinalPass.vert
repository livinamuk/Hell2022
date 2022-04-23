// vertex shader
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 model;

out vec2 TexCoords;
out vec2 SplitscreenAdjustedCoords;

uniform int u_CHROM_ABZ_mode;

void main()
{
    gl_Position = model * vec4(position, 0.0f, 1.0f);
    TexCoords = texCoords;

    
	SplitscreenAdjustedCoords = texCoords;
	
	// get from 1 to 0.5 range to 1 to 0
	if (u_CHROM_ABZ_mode == 1)
		SplitscreenAdjustedCoords.y = (texCoords.y * 2)  - 1;
	// get from 0.5 to 0 range to 1 to 0
	if (u_CHROM_ABZ_mode == 2)
		SplitscreenAdjustedCoords.y = (texCoords.y * 2);

}
 