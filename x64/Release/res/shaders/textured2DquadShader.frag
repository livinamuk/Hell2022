// fragment shader
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
  
uniform vec3 colorTint;
uniform sampler2D fboAttachment;
  
void main()
{
    FragColor = texture(fboAttachment, TexCoords);
    FragColor *= vec4(colorTint, 1);
} 