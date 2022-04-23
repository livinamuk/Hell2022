#version 400 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

//uniform int index;
uniform mat4 captureViewMatrix[6];

in vec3 gNormal[];
in vec2 gTexCoord[];
in float gMaterialID[];
in mat3 gTBN[];

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;
out float MaterialID;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; 
		// for each triangle's vertices
        for(int i = 0; i < 3; ++i)         
		{
            FragPos = gl_in[i].gl_Position.rgb;
			
			TexCoord = gTexCoord[i];
			Normal = gNormal[i];
			MaterialID = gMaterialID[i];
			TBN = gTBN[i];

            gl_Position = captureViewMatrix[face] * vec4(FragPos, 1);
            EmitVertex();
        }    
        EndPrimitive();
    }
} 