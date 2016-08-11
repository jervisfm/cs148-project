// CS148 Summer 2016 Homework 3 - Shaders

#version 330 core
out vec4 color;

in vec3 FragPos;  
in vec3 Normal;
in vec2 TextCoord;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 endLightPos;
uniform float lightRadius;

uniform sampler2D myTexture;

//Citation: The equation that the Phong Shader was derived from was retrieved at https://en.wikipedia.org/wiki/Phong_reflection_model on 19 July 16. 

bool withinLight(vec3 pos){
    vec3 vecStart2End = endLightPos - lightPos;
    vec3 vecPoint2End = endLightPos - pos;
    vec3 vecStart2Point = pos - lightPos; 
    if(length(cross(vecStart2End, vecStart2Point))/length(vecStart2End) <= lightRadius){
	if(dot(vecStart2Point, vecStart2End) >= 0 && dot(vecPoint2End, vecStart2End) >= 0){
            return true;
	}
    }
    return false;
}

void main()
{
    bool lightCheck = withinLight(FragPos);
    //color = texture(myTexture, TextCoord);
    float distance = 1/pow(length(FragPos-lightPos),2);
    color = lightCheck ? vec4(1.0) : vec4(0.0);
    color.rgb = distance*color.rgb;
    color.a = dot(color.rgb, vec3(0.33));          
    
    if(color.a < 0.1)
	discard;
    //color.rgb = distance*color.rgb;
    color.a = clamp(color.a, 0.0, 1.0);
}

