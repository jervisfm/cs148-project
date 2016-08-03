#version 330 core

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform vec3 uniform_light_position;  // center position of the light source.
uniform vec3 uniform_light_direction; // should point towards light source.
uniform float uniform_light_radius;   // radius of light source.


// Implementation of directional + spot lighting based on tutorial at:
// http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
vec4 ComputeDirectionalLight(vec3 light_direction, float light_radius, vec3 frag_position) {
  vec3 surface_to_light_vector = normalize(light_direction);
  // For directional lights, we assume that they do not lose any power.
  float light_reduction_coefficient = 1.0;


  float diffuse_light_coefficient = light_reduction_coefficient * max(0.0, dot(normalize(Normal), surface_to_light_vector));
  return vec4(diffuse_light_coefficient, diffuse_light_coefficient, diffuse_light_coefficient, 1.0);
}

vec4 ComputeDirectionalTubeLight(vec3 light_position, vec3 light_direction, float light_radius, vec3 frag_position, vec3 frag_normal) {
  vec3 surface_to_light_vector = -normalize(light_direction);
  float d = (-1)*dot(light_direction, light_position);
  float denom = dot(light_direction, light_direction);
  float t = (-1)*(d + dot(light_direction, frag_position))/denom;
  vec3 P = frag_position+t*light_direction;

  float distance = length(P-light_position);
  float light_reduction_coefficient = max(0., min(1.0, 1.-pow(distance/light_radius,4)));
  //if (distance > light_radius)
  //    light_reduction_coefficient = 0.;


  float diffuse_light_coefficient = light_reduction_coefficient * max(0.0, dot(normalize(Normal), surface_to_light_vector));
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
    //vec3 dir = vec3(0.,0.,-1.);
    //vec3 pos = vec3(0., 0., 2.);
    //vec4 directional_light_color = ComputeDirectionalLight(uniform_light_direction, uniform_light_radius, FragPos);
    vec4 directional_light_color  = ComputeDirectionalTubeLight(uniform_light_position, uniform_light_direction, uniform_light_radius, FragPos, Normal);
    //vec4 directional_light_color  = ComputeDirectionalTubeLight(pos, dir, uniform_light_radius, FragPos, Normal);
    color = diffuse_texture_color * directional_light_color;
    //color = diffuse_texture_color;
    //color = vec4(.0, 0.,0.,1.);
}

