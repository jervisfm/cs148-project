
// CS148 Summer 2016 Homework 3 - Shaders
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;

out vec3 Normal;
out vec3 FragPos;
out vec2 TextCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos; 
uniform vec3 endLightPos;

void main()
{
    //OTHER BUILT IN VARIABLES Colors, texture coordinates, and other per-fragment data
    //user-defined varying variables
    //compute normal for each vertex. 
    //compute NBC coordinate (gl_position)
    gl_Position = projection*view*model*vec4(position, 1.0);
    //vec3 pos = position*lightPos + endLightPos*(1-position);
    
    //gl_Position = projection*view*vec4(pos, 1.0);
    Normal = normal;
    FragPos = vec3(model*vec4(position, 1.0));
//    TextCoord = vec2(textCoord.x, 1.0 - textCoord.y);          
    TextCoord = textCoord;          

} 
