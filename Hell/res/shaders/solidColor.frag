#version 330 core
out vec4 FragColor;

in vec4 worldPos;
uniform vec3 u_color;
uniform vec3 u_cameraPos;
uniform bool u_renderFog;

float Distance(vec3 point1, vec3 point2)
{
    return sqrt( (point1.x - point2.x) * (point1.x - point2.x) +
                        (point1.y - point2.y) * (point1.y - point2.y) +
                        (point1.z - point2.z) * (point1.z - point2.z) );
}

void main()
{
    float dist = Distance(worldPos.xyz, u_cameraPos);   

    vec3 finalColor = u_color;

    if (u_renderFog)
    {    
        float darkness = dist / 8.5;
	
	    vec3 fogColor = vec3(0.18, 0.262, 0.455);
	    fogColor = vec3(0, 0, 0);
	    darkness = clamp(darkness, 0, 1);
	    finalColor = mix(u_color, fogColor, darkness);
    }


    FragColor = vec4(finalColor, 1.0);
}