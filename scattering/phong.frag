// CS148 Summer 2016 Homework 3 - Shaders

#version 330 core
out vec4 color;

in vec3 FragPos;  
in vec3 Normal;
in vec2 TextCoord;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform sampler2D myTexture;

//Citation: The equation that the Phong Shader was derived from was retrieved at https://en.wikipedia.org/wiki/Phong_reflection_model on 19 July 16. 


void main()
{
    /*vec3 Lm = lightPos - FragPos;
    vec3 V = viewPos - FragPos;
    //need to normalize the vectors
    //normalize vector pointing towards Light
    float length_Lm = (Lm[0]*Lm[0]+Lm[1]*Lm[1]+Lm[2]*Lm[2]);
    vec3 len_Lm = vec3(sqrt(length_Lm));
    Lm = Lm / len_Lm;
    //normalize vector pointing towards viewer
    float length_V = (V[0]*V[0]+V[1]*V[1]+V[2]*V[2]);
    vec3 len_V = vec3(sqrt(length_V));
    V = V/ len_V;
 
    //Diffuse	    (
    float dotLM_N = dot(Lm, Normal);
    float diffuseConstant = 0.85;
    //ambient
    float ambientConstant = 0.1;
    //specular
    vec3 Rm = 2*(dotLM_N)*Normal - Lm;
    float dotRm_V = dot(Rm, V);
    float shininess = 32.0;
    float specularConstant = 0.90;
    
    vec3 intensity;
    //if dot products are negative then don't include them (wikipedia, Phong Reflection Model)
    //Don't include specular if diffuse dot product is negative (wikipedia, Phong Reflection Model)
    if(dotLM_N < 0.0){ 
    	intensity = vec3(ambientConstant);
    } else if (dotRm_V < 0.0){
	intensity = ambientConstant + (diffuseConstant * dotLM_N*lightColor);
    } else{
	intensity = ambientConstant + (diffuseConstant * dotLM_N*lightColor) + specularConstant*pow(dotRm_V, shininess)*lightColor;
    }
    color = vec4(objectColor, 1.0f) * vec4(intensity,1.0f);
    */
    color = texture(myTexture, TextCoord);
    color.a = dot(color.rgb, vec3(0.33));          
    if(color.a < 0.9)
	discard;
    //color.a = clamp(color.a, 0.0, 1.0);s

} 
