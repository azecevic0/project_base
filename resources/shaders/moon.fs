#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
};

in vec2 TexCoords;

uniform Material material;

void main()
{
    FragColor = texture(material.texture_diffuse1, TexCoords) * 1.5;
}