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
    bool reflected;
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
    LightState(Scene* s) : scene(s) {}

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

    void bindLights(Shader shader);
    std::vector<DirectionalLight> primaryLights;

private:
    Scene* scene;
    std::vector<LightRay> lightRays;
    std::vector<DirectionalLight> directionalLights;

    DirectionalLight ShootRay(DirectionalLight dl, unsigned depth);
};

#endif // LIGHTSTATE_H
