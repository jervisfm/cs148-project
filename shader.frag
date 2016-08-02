#version 330 core

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform vec3 uniform_light_position;  // center position of the light source.
uniform vec3 uniform_light_direction; // should point towards light source.
uniform float uniform_light_radius;   // radius of light source.


// Implementation of directional lighting based on tutorial at:
// http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
vec4 ComputeDirectionalLight(vec3 light_direction, float light_radius, vec3 frag_position) {
  vec3 surface_to_light_vector = normalize(light_direction);
  // For directional lights, we assume that they do not lose any power.
  float light_reduction_coefficient = 1.0;

  float diffuse_light_coefficient = max(0.0, dot(Normal, surface_to_light_vector));
  return vec4(diffuse_light_coefficient, diffuse_light_coefficient, diffuse_light_coefficient, 1);
}

void main()
{
    vec4 diffuse_texture_color = vec4(texture(texture_diffuse1, TexCoords));
    vec4 directional_light_color = ComputeDirectionalLight(uniform_light_direction,
                                                           uniform_light_radius, FragPos);
    color = diffuse_texture_color * directional_light_color;
    color = diffuse_texture_color;
    //color = vec4(.0, 0.,0.,1.);
}

