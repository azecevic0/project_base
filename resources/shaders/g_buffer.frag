#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform Material material;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    vec4 albedo = texture(material.texture_diffuse1, TexCoords);
    // blending
    if (albedo.a < 0.1)
        discard;
    gAlbedoSpec.rgb = albedo.rgb;
    
    // store specular intensity in gAlbedoSpec's alpha component
    vec3 specular = texture(material.texture_specular1, TexCoords).rgb;
    if (specular.r != specular.g || specular.g != specular.b) {
        // if specular map is defaulted to diffuse map convert it to grayscale
        gAlbedoSpec.a = dot(specular, vec3(0.299,0.587,0.114));
    } else {
        gAlbedoSpec.a = specular.r;
    }
}