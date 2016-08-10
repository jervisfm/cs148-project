#ifndef LIGHTSTATE_H
#define LIGHTSTATE_H
#include "scene.h"
#include <vector>

#define GLM_FORCE_RADIANS
// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct DirectionalLight {
    glm::vec3 startPos, endPos, dir;
    double radius;
};

struct LightRay {
    glm::vec3 startPos, endPos;
    double radius;
    unsigned depth;
};

struct HitRecord {
    double t;
    glm::vec3 P, normal;
    bool reflected;
};
struct Ray {
    glm::vec3 o, d, normal;
};

class LightState
{
public:
    LightState(Scene* s) : scene(s) {
        //Create the models used for the cylinders
        unsigned precision = 40;

        this->cylinderPrimary  = Model::genCylinder(precision, "light_falloff.png");
        this->cylinderPrimary->meshes[0].material.ambient = glm::vec3(0.2);

        this->cylinderSecondary = Model::genCylinder(precision, "light_falloff2.png");
        this->cylinderSecondary->meshes[0].material.ambient = glm::vec3(0.4);

        this->cylinderTop = Model::genCylinderTop(precision);
        cylinderTop->meshes[0].material.ambient = glm::vec3(1.0);

    }

    std::vector<LightRay> getLightRays() {
        return this->lightRays;
    }

    std::vector<DirectionalLight>  getDirectionalLights() {
        return this->directionalLights;
    }

    void addPrimaryLight(DirectionalLight dl)
    {
        this->primaryLights.push_back(dl);
    }

    void updateState();
    void drawTubes(Shader shader);

    void bindLights(Shader shader);
    std::vector<DirectionalLight> primaryLights;


private:
    Scene* scene;
    std::vector<LightRay> lightRays;
    std::vector<DirectionalLight> directionalLights;

    DirectionalLight ShootRay(DirectionalLight dl, unsigned depth);
    Model *cylinderTop, *cylinderPrimary, *cylinderSecondary;
};

#endif // LIGHTSTATE_H
