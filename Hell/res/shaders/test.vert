#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec2 TexCoord;

void main()
{
	TexCoord = aTexCoord;
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}