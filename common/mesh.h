#ifndef MESH_H
#define MESH_H

#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
#include "../Shader.h"
#include <iostream>
#include <vector>

// GLM Mathematics
#define GLM_FORCE_RADIANS // force everything in radian
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint id;
    string type;
    aiString path;  // We store the path of the texture to compare with other textures
};

class Mesh {
    public:
        /*  Mesh Data  */
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;
        /*  Functions  */
        Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
        void Draw(Shader shader);
    private:
        /*  Render data  */
        GLuint VAO, VBO, EBO;
        /*  Functions    */
        void setupMesh();
};

#endif // MESH_H
