#version 330

in vec3 in_position;
in vec2 in_texcoord;

uniform int u_splitscreen_mode;

out vec2 TexCoords;
out vec2 focus;

void main(void)
{
	TexCoords = in_texcoord;
	gl_Position = vec4(in_position, 1.0);

	focus = vec2(0.5,0.5);
	
	if (u_splitscreen_mode == 1)
		focus = vec2(0.5,0.75);
	if (u_splitscreen_mode == 2)
		focus = vec2(0.5,0.25);

}	