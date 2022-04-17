#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 aBoneID;
layout (location = 6) in vec4 aBoneWeight;

uniform mat4 model;
uniform bool hasAnimation;
uniform mat4 skinningMats[64];

out vec3 FragPos;

void main()
{

		gl_Position = model * vec4(aPos, 1.0);
}