#version 330 core

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform vec3 uniform_light_position;  // center position of the light source.
uniform vec3 uniform_light_end_position;  // center position of the light source.
uniform vec3 uniform_light_direction; // should point towards light source.
uniform float uniform_light_radius;   // radius of light source.


#define MAX_LIGHTS 16

uniform int numLights;

uniform struct Light {
    vec3 position;
    vec3 end_position;
    vec3 direction;
    float radius;
} allLights[MAX_LIGHTS];

uniform struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

// Implementation of directional + spot lighting based on tutorial at:
// http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
vec4 ComputeDirectionalTubeLight(Light light, vec3 frag_position, vec3 frag_normal) {
  vec3 surface_to_light_vector = -normalize(light.direction);
  float d = (-1)*dot(light.direction, light.position);
  float denom = dot(light.direction, light.direction);
  float t = (d + dot(light.direction, frag_position))/denom;
  vec3 P = frag_position-t*light.direction;

  float distance = length(P-light.position);
  float light_reduction_coefficient = max(0., min(1.0, 1.-pow(distance/light.radius,0.5)));
  if (t < 0.)
      light_reduction_coefficient = 0.;
  if (length(frag_position-light.end_position) > 3*light.radius)
      light_reduction_coefficient = 0.;
  if(dot(P-frag_position, light.direction) > 0.)
      light_reduction_coefficient = 0.;


  float diffuse_light_coefficient = light_reduction_coefficient ;//* max(0.0, dot(normalize(Normal), surface_to_light_vector));
  return vec4(diffuse_light_coefficient, diffuse_light_coefficient, diffuse_light_coefficient, 1.0);
}

vec4 ComputeSpotLight(vec3 light_position, vec3 light_direction, vec3 light_cone_direction,
                      float max_light_cone_angle, vec3 frag_position) {
  vec3 surface_to_light_vector = normalize(light_position - frag_position);
  float distance_to_light = length(light_position - frag_position);
  // Reduce light by approximately 1/distance^2
  float light_reduction_coefficient = 1.0 / (1.0 + pow(distance_to_light, 2));
  // Normalize direction just in case.
  light_cone_direction = normalize(light_cone_direction);
  // Direction for the light ray would be opposite of the ray going from surface to the light.
  vec3 light_ray_direction = -surface_to_light_vector;
  // Compute angle between center of cone and the light ray.
  float light_to_surface_angle = degrees(acos(dot(light_ray_direction, light_cone_direction)));
  // If light ray is outside max angle, then it's outside the cone and so make it invisible.
  if (light_to_surface_angle > max_light_cone_angle) {
     light_reduction_coefficient = 0;
  }

  float diffuse_light_coefficient = light_reduction_coefficient *  max(0.0, dot(Normal, surface_to_light_vector));
  return vec4(diffuse_light_coefficient, diffuse_light_coefficient, diffuse_light_coefficient, 1.0);
}

void main()
{
    vec4 diffuse_texture_color = vec4(texture(texture_diffuse1, TexCoords));

    vec4 illuminance = vec4(0.01);
    vec4 ambient = vec4(0);
    for(int i = 0; i < numLights; ++i)
    {
        illuminance += ComputeDirectionalTubeLight(allLights[i], FragPos, Normal);
       //if (length(FragPos - allLights[i].position) < allLights[i].radius)
       //{
       //   illuminance += vec4(0.3,0.3,0.3,1);
       //}
    }
    color = diffuse_texture_color * illuminance + vec4(material.ambient, 1.) + ambient;

    // Color overide to check what color texture loads with. Handy for testing but should
    // be disabled otherwise.
    vec4 full_light_color = diffuse_texture_color + vec4(material.ambient, 1.) + ambient;
    color = full_light_color;
}

