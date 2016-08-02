#version 330 core

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform vec3 uniform_light_position;  // center position of the light source.
uniform vec3 uniform_light_direction; // should point towards light source.
uniform float uniform_light_radius;   // radius of light source.


vec4 ComputeDirectionalLight(vec3 light_position, vec3 light_direction, float light_radius, vec3 frag_position) {

 return vec4(0.,0.,0.,1);
}

void main()
{
    vec4 diffuse_texture_color = vec4(texture(texture_diffuse1, TexCoords));
    vec4 directional_light_color = ComputeDirectionalLight(uniform_light_position, uniform_light_direction,
                                                           uniform_light_radius, FragPos);
    color = diffuse_texture_color * directional_light_color;
    color = diffuse_texture_color;
    //color = vec4(.0, 0.,0.,1.);
}

