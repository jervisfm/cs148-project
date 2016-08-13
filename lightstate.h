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
        this->cylinderPrimary->meshes[0].material.alphaTest = -1.;
        this->cylinderSecondary = Model::genCylinder(precision, "light_falloff2.png");
        this->cylinderSecondary->meshes[0].material.ambient = glm::vec3(0.4);
        this->cylinderSecondary->meshes[0].material.alphaTest = 0.1;

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

    // Loads primary lights from the given |light_map_file|. |light_map_file|
    // should be a text file that has lines of the format:
    // pos: (X, X, X) direction: (X, X, X) radius: X
    // where 'X' is a placeholder for a numeric (float) value. A valid sample
    // line is given below:
    // pos: (4.1, 2.0, 3.75) direction: (-1.5, -0.5, -0.5) radius: 1.55
    void loadLights(const std::string&  light_map_file);
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
