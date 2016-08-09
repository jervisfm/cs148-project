#include "mesh.h"
#include <iostream>
#include <float.h>

Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    this->setupMesh();
    this->boundingBox = this->computeAABB();

    material.ambient = glm::vec3(0.);
    material.diffuse = glm::vec3(0.);
    material.specular = glm::vec3(0.);
    material.shininess = 0.f;
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
                 &this->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
                 &this->indices[0], GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (GLvoid*)0);
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (GLvoid*)offsetof(Vertex, Normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (GLvoid*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::bindTextures(Shader shader) {
    GLuint diffuseNr = 1;
    GLuint specularNr = 1;
    for(GLuint i = 0; i < this->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
        // Retrieve texture number (the N in diffuse_textureN)
        stringstream ss;
        string number;
        string name = this->textures[i].type;
        if(name == "texture_diffuse")
            ss << diffuseNr++; // Transfer GLuint to stream
        else if(name == "texture_specular")
            ss << specularNr++; // Transfer GLuint to stream
        number = ss.str();

        glUniform1f(glGetUniformLocation(shader.Program, ("material." + name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
    }
    glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);
    glUniform3fv(glGetUniformLocation(shader.Program, "material.ambient"),1, glm::value_ptr(this->material.ambient));

}

void Mesh::unbindTextures(Shader shader)
{
    // Always good practice to set everything back to defaults once configured.
    for (GLuint i = 0; i < this->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Mesh::Draw(Shader shader)
{
    this->bindTextures(shader);

    // Draw mesh
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    this->unbindTextures(shader);
}

void Mesh::DrawInstanced(Shader shader, unsigned n)
{
    this->bindTextures(shader);
    glBindVertexArray(this->VAO);
    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, n);
    glBindVertexArray(0);
    this->unbindTextures(shader);
}

AABB Mesh::computeAABB(){
    glm::vec3 min, max, pos;
    min = glm::vec3(DBL_MAX, DBL_MAX, DBL_MAX);
    max = glm::vec3(-DBL_MAX, -DBL_MAX, -DBL_MAX);

    for (int i = 0; i < this->indices.size(); i++)
    {
        pos = this->vertices[i].Position;
        for (int j = 0; j < 3; j++)
        {
            if (pos[j] < min[j])
                min[j] = pos[j];
            if (pos[j] > max[j])
                max[j] = pos[j];
        }
    }

    AABB res;
    res.back = max;
    res.front = min;
    return res;
}

bool Mesh::nextTriangle(Vertex* A, Vertex* B, Vertex* C) {
    *A = this->vertices[this->indexIndex];
    *B = this->vertices[this->indexIndex+1];
    *C = this->vertices[this->indexIndex+2];

    if (this->indexIndex + 3 < this->vertices.size())
        this->indexIndex += 3;
    else {
        this->indexIndex = 0;
        return false;
    }
    return true;
}
