#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out vec3 g_WorldNormal;

uniform mat4 u_MatrixProjection;
uniform mat4 u_MatrixView;


void main() {    
    
    vec3 a = gl_in[0].gl_Position.xyz;
    vec3 b = gl_in[1].gl_Position.xyz;
    vec3 c = gl_in[2].gl_Position.xyz;
    g_WorldNormal = normalize(cross(b-a, c-a));
    //g_WorldNormal = normalize(cross(pos1 - pos0, pos2 - pos0));

//    g_WorldNormal = vec3(1, 0, 0);
    
    

    gl_Position = u_MatrixProjection * u_MatrixView * gl_in[0].gl_Position ;//+ vec4(-0.1, 0.0, 0.0, 0.0); 
    EmitVertex();
    
    gl_Position = u_MatrixProjection * u_MatrixView * gl_in[1].gl_Position;// + vec4( 0.1, 0.0, 0.0, 0.0);
    EmitVertex();
    
    gl_Position = u_MatrixProjection * u_MatrixView * gl_in[2].gl_Position;// + vec4( 0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    
    EndPrimitive();
}  
