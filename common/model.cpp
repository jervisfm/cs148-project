#include "model.h"
#include "controls.h"
#include <SOIL.h>
#include <iostream>
using namespace std;
#include <math.h>

GLint TextureFromFile(const char* path, string directory)
{
     //Generate texture ID and load texture data
    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;
}

void Model::Draw(Shader shader)
{
    if (Controls::getActiveMirror() == this)
        glStencilMask(0xFF);
    else
        glStencilMask(0x00);

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(this->modelMatrix));
    for(GLuint i = 0; i < this->meshes.size(); i++)
        this->meshes[i].Draw(shader);

}

void Model::loadModel(string path)
{
    Assimp::Importer import;
    // aiProcess_GenNormals - Specify that Assimp should generate Normals if the model is missing them.
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    this->directory = path.substr(0, path.find_last_of('/'));

    this->processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // Process all the node's meshes (if any)
    for(GLuint i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->processMesh(mesh, scene));
    }
    // Then do the same for each of its children
    for(GLuint i = 0; i < node->mNumChildren; i++)
    {
        this->processNode(node->mChildren[i], scene);
    }
}

Mesh Model::genVertices(float radius, unsigned precision)
{
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    float theta = 2*M_PI/(double)precision;
    for (int i = 0; i < precision; i++)
    {
        Vertex v;
        for (int j = 0; j <= 1; j++) //upper and lower circle
        {
            v.Position = glm::vec3(radius*cos(i*theta), j, radius*sin(i*theta));
            v.Normal = glm::vec3(cos(i*theta), 0., sin(i*theta));
            float x = (double)i/(double)precision;
            float u = 2*x < 1. ? 2*x : 2. - 2*x;
            v.TexCoords = glm::vec2(u, j); //TODO : change it
            std::cout << "UV : " << v.TexCoords[0] << ", " << v.TexCoords[1] << std::endl;
            vertices.push_back(v);
            for (int k = 0; k < 3; k++)
            {
                indices.push_back((2*i+j+k) % (2*precision));
            }
        }

    }
    // Texture
    Texture texture;
    texture.id = TextureFromFile("light_texture.png", "models");
    texture.type = "texture_diffuse";
    texture.path = "models/light_texture.png";
    cout << texture.path.C_Str() << endl;
    textures.push_back(texture);
    this->textures_loaded.push_back(texture);  // Add to loaded textures
    return Mesh(vertices, indices, textures);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // Data to fill
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;

    // Walk through each of the mesh's vertices
    for(GLuint i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // Positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // Normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        // Texture Coordinates
        if(mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(GLuint i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for(GLuint j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // Process materials
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // Diffuse: texture_diffuseN
        // Specular: texture_specularN
        // Normal: texture_normalN

        // 1. Diffuse maps
        vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. Specular maps
        vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    // Return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}
vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        GLboolean skip = false;
        for(GLuint j = 0; j < textures_loaded.size(); j++)
        {
            if(textures_loaded[j].path == str)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // If texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str;
            cout << str.C_Str() << endl;
            textures.push_back(texture);
            this->textures_loaded.push_back(texture);  // Add to loaded textures
        }
    }
    return textures;
}

void Model::setModelMatrix(glm::mat4 matrix) {
    this->modelMatrix = matrix;
    this->invTmodelMatrix = glm::transpose(glm::inverse(matrix));
}

Vertex Model::transformPoint(Vertex v)
{
    glm::vec3 Position, Normal;
    glm::vec4 Pos4, Norm4;
    Vertex P;

    Pos4 = this->modelMatrix*glm::vec4(v.Position,1.);
    Position = glm::vec3(Pos4) / Pos4.w;
    P.Position = Position;

    Norm4 = this->invTmodelMatrix*glm::vec4(v.Normal,0.);
    Normal = glm::vec3(Norm4);
    P.Normal = Normal;

    P.TexCoords = v.TexCoords;

    return P;
}

glm::mat4 Model::getLoadedMatrix() {
    glm::mat4 m(1.0);
    m = glm::scale(m, this->lOptions.scale);
    m = glm::rotate(m, this->lOptions.rotDegrees, this->lOptions.rotAxis);
    glm::vec4 displacement = glm::inverse(m)*glm::vec4(this->lOptions.position, 1.);
    m = glm::translate(m, glm::vec3(displacement)/displacement.w);
    //m = glm::translate(m, this->lOptions.position);
    return m;
}

void Model::cylinderTransform(glm::vec3 start, glm::vec3 end) {
    glm::vec3 bot = glm::vec3(0,0,0);
    glm::vec3 top = glm::vec3(0,1,0);

    glm::vec3 dir = glm::normalize(end-start);
    glm::vec3 axis = glm::cross(top-bot, dir);
    float angle = acos(glm::dot(top-bot, dir));
    float scaleY = glm::length(start-end);

    glm::mat4 m(1.0);
    m = glm::translate(m, (start-bot));
    m = glm::rotate(m, angle, axis);
    m = glm::scale(m, glm::vec3(1., scaleY, 1.));


    this->setModelMatrix(m);
 }
