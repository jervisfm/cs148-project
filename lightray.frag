#version 330 core

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform sampler2D texture_diffuse1;


uniform struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float alphaTest;
} material;
/*
uniform struct lightSource {
    vec3 pos;
    float radius;
}sources[16];

uniform int nSources;
*/
void main()
{
    vec4 diffuse_texture_color = vec4(texture(texture_diffuse1, TexCoords));
    //diffuse_texture_color.a = 0.1;
    vec4 illuminance = vec4(material.ambient, 1.);
    vec4 ambient = vec4(0);
    /*for(int i = 0; i < nSources; i++)
    {
        float d = length(FragPos-sources[i].pos);
        illluminance += max(0., 1-d/li)

    }*/

    color = diffuse_texture_color * illuminance + /*vec4(material.ambient, 0.)*/ + ambient;
    if(color.a <= material.alphaTest)
        discard;

}

