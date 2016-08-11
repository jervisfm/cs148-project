#include "scene.h"
#include <stdio.h>

using namespace std;


void Scene::drawScene() {
    // Draw the scene, update the stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    for (int i = 0; i < models.size(); i++) {
        SceneElement se = models[i];
        se.m->Draw(*(se.shader));
    }
}

bool Scene::nextTriangle(Vertex *A, Vertex *B, Vertex *C) {
    SceneElement se = models[modelIndex];
    Mesh mesh =  se.m->meshes[meshIndex];
    Vertex v;

    v = mesh.vertices[mesh.indices[indexIndex]];
    *A = se.m->transformPoint(v);

    v = mesh.vertices[mesh.indices[indexIndex+1]];
    *B = se.m->transformPoint(v);

    v = mesh.vertices[mesh.indices[indexIndex+2]];
    *C = se.m->transformPoint(v);
    //And now update the indices
    if (indexIndex + 3 < mesh.indices.size()) {
        indexIndex += 3;
    }
    else {
        indexIndex = 0;
        //Maybe next mesh
        if (meshIndex + 1 < se.m->meshes.size())
        {
            meshIndex += 1;
        }
        else
        {
            //This was the last mesh in the model. Get next model
            meshIndex = 0;
            if (modelIndex + 1 < models.size())
            {
                modelIndex += 1;
            }
            else
            {
                resetTriangleIterator(); // reset the iterator
                return false; //We have reached the end
            }
        }
    }
    return true;
}

void Scene::resetTriangleIterator() {
    indexIndex = meshIndex = modelIndex = 0; // reset the iterator
}

void Scene::addMirror(Model *m) {
    this->mirrors.push_back(m);
    m->hasMirror = true;
}

vector<Model*> Scene::getMirrors() {
    return this->mirrors;
}

void Scene::loadMap(const char* filename, Shader *s) {
    std::ifstream input_file_stream(filename);
    std::string file_line;
    while (std::getline(input_file_stream, file_line)) {
        // Skip over any lines in the map file that start with # -- these are comments.
        if (file_line[0] == '#') {
            continue;
        }

        char objPath[255];
        glm::vec3 pos, scale, rot;
        float degrees;
        if (sscanf(file_line.c_str(), "%s pos: (%f, %f, %f) scale: (%f, %f, %f) rot: (%f, %f, %f, %f)",
                      objPath, &pos.x, &pos.y, &pos.z, &scale.x, &scale.y, &scale.z,
                      &rot.x, &rot.y, &rot.z, &degrees) > 0)
        {
            loadOptions lOptions;
            lOptions.position = pos;
            lOptions.scale = scale;
            lOptions.rotAxis = rot;
            lOptions.rotDegrees = degrees;
            Model* model = new Model(objPath);
            model->lOptions = lOptions;
            model->setModelMatrix(model->getLoadedMatrix());
            this->addModel(model, s);
        }


    }
}

void Scene::loadMirrors(const char *filename, Shader* s)
{
    std::ifstream input_file_stream(filename);
    std::string file_line;
    while (std::getline(input_file_stream, file_line)) {
        // Skip over any lines in the map file that start with # -- these are comments.
        if (file_line[0] == '#') {
            continue;
        }

        char objPath[255];
        glm::vec3 pos, scale, rot;

        float degrees;
        if (sscanf(file_line.c_str(), "%s pos: (%f, %f, %f) scale: (%f, %f, %f) rot: (%f, %f, %f, %f)",
                      objPath, &pos.x, &pos.y, &pos.z, &scale.x, &scale.y, &scale.z,
                      &rot.x, &rot.y, &rot.z, &degrees) > 0)
        {
            loadOptions lOptions;
            lOptions.position = pos;
            lOptions.scale = scale;
            lOptions.rotAxis = rot;
            lOptions.rotDegrees = degrees;
            Model* model = new Model(objPath);
            model->meshes[0].isMirror = true;
            model->lOptions = lOptions;
            model->setModelMatrix(model->getLoadedMatrix());

            this->addModel(model, s);
            this->addMirror(model);
        }
    }
}

int Scene::totalTextures() {
    int res = 0;
    for(int i = 0; i < this->models.size(); i++)
    {
        Model* m = this->models[i].m;
        for(int j = 0; j < m->meshes.size(); j++)
        {
            Mesh mesh = m->meshes[j];
            res += mesh.textures.size();
        }
    }
    return res;
}
