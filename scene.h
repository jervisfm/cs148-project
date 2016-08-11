#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "common/model.h"
#include "Shader.h"

using namespace std;

struct SceneElement {
    Shader* shader;
    Model* m;
};

class Scene
{
public:
    Scene() {}

    //Attributes
    vector<SceneElement> models;
    void addModel(Model* m, Shader *s) {
        SceneElement se = {s,m};
        this->models.push_back(se);
    }
    void drawScene();
    bool nextTriangle(Vertex*, Vertex*, Vertex*);
    void resetTriangleIterator();
    void addMirror(Model *m);
    vector<Model*> getMirrors();
    void loadMap(const char* filename, Shader *s);
    void loadMirrors(const char* filename, Shader *s);
    int totalTextures();
private:
    unsigned modelIndex=0, meshIndex=0, indexIndex=0;
    vector<Model*> mirrors;
};

#endif // SCENE_H
