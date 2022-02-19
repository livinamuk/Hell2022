#version 330 core
layout (location = 0) in vec3 position;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec3 outColor;
out vec4 worldPos;

void main()
{
    worldPos = vec4(model * vec4(position, 1.0));
    gl_Position = projection * view * worldPos;
}