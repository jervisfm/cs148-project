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
} material;

void main()
{
    vec4 diffuse_texture_color = vec4(texture(texture_diffuse1, TexCoords));
    //diffuse_texture_color.a = 0.1;
    vec4 illuminance = vec4(0.3);
    vec4 ambient = vec4(0);

    color = diffuse_texture_color * illuminance + vec4(material.ambient, 0.) + ambient;

    // Color overide to check what color texture loads with. Handy for testing but should
    // be disabled otherwise.
    //vec4 full_light_color = diffuse_texture_color + vec4(material.ambient, 1.) + ambient;
    //color = full_light_color;
}

