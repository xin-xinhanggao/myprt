#version 330 core

in vec2 TexCoords;
in vec3 Prtcolor;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main()
{    
    color = vec4(texture(texture_diffuse1, TexCoords) * vec4(Prtcolor, 1.0f));
}