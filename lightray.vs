#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

#define MAX_INSTANCES 100
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    // Convert normal vector and fragment position into world-space coordinates.
    vec4 FragPos4 = model * vec4(position, 1.0f);
    FragPos = vec3(FragPos4 / FragPos4.w);

    // For the normal, we have to be careful to use a Normal matrix
    // to do the conversion. Normal matrix is defined as inverse of
    // of transpose of model matrix. Also, Normal vector only have direction,
    // so downscale normal matrix to 3x3 to get rid of translation
    // component.
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    float scaleY = length(model[1]);
    Normal = normalMatrix * normal;

    TexCoords = texCoords;
    /*if (TexCoords[1] == 1.)
    {
        TexCoords[1] *= scaleY/5.;
    }*/
}

