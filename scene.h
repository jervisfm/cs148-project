#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "common/model.h"
#include "Shader.h"

using namespace std;

struct SceneElement {
    Model* m;
    bool mirror;
    Shader* shader;
};

class Scene
{
public:
    Scene() {}

    //Attributes
    vector<SceneElement> models;
    void addModel(Model *m, bool mirror, Shader *s) {
        SceneElement se;
        se.m = m;
        se.mirror = mirror;
        se.shader = s;
        this->models.push_back(se);
    }
    void drawScene();
    bool nextTriangle(Vertex*, Vertex*, Vertex*);
    void resetTriangleIterator();
private:
    unsigned modelIndex=0, meshIndex=0, indexIndex=0;
};

#endif // SCENE_H
