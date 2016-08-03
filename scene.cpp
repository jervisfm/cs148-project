#include "scene.h"

using namespace std;
void Scene::drawScene() {
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
}

vector<Model*> Scene::getMirrors() {
    return this->mirrors;
}
