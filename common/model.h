#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <iostream>
#include <sstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// GLM Mathemtics
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Shader.h"
#include "mesh.h"

using namespace std;

struct loadOptions {
    glm::vec3 position, scale, rotAxis;
    float rotDegrees;
};

class Model
{
    public:
        /*  Functions   */
        Model(string path)
        {
            this->loadModel(path);
            this->modelMatrix = glm::mat4(1.);
            this->pathName = path;
        }
        void Draw(Shader shader);
        void setModelMatrix(glm::mat4 matrix);
        glm::mat4 getModelMatrix() {
            return this->modelMatrix;
        }
        // Transforms the position and the normal according to the model matrix
        Vertex transformPoint(Vertex);


        vector<Mesh> meshes;
        glm::mat4 modelMatrix;
        glm::mat4 invTmodelMatrix;
        loadOptions lOptions;
        string pathName;
        string genMapDirective() {
            std::stringstream ss;
            ss << this->pathName << " pos: (" << this->lOptions.position.x << ", "
              << this->lOptions.position.y << ", " << this->lOptions.position.z << ") scale: ("
              << this->lOptions.scale.x << ", "  << this->lOptions.scale.y << ", "
              << this->lOptions.scale.z << ") rot: (" << this->lOptions.rotAxis.x << ", "
              << this->lOptions.rotAxis.y << ", " << this->lOptions.rotAxis.z << ", "
              << this->lOptions.rotDegrees << ")";
            return ss.str();
        }
        bool hasMirror = false;
        glm::mat4 getLoadedMatrix();

    private:
        /*  Model Data  */
        string directory;
        /*  Functions   */
        void loadModel(string path);

        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                             string typeName);
        vector<Texture> textures_loaded;
};
#endif // MODEL_H
