#version 330 core

layout (location = 0) in vec2 position;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position, 0.0f, 1.0f);
}
 