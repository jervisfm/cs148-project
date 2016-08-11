#version 330 core
out vec4 color;

in vec2 TextCoord;


uniform sampler2D lightTexture;
uniform sampler2D sceneTexture;

void main()
{            
    vec4 scenecolor = texture(sceneTexture,TextCoord);
    vec4 lightColor = texture(lightTexture, TextCoord);
    color = scenecolor+ lightColor;
}
