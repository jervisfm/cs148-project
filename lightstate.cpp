#include "lightstate.h"
#include "scene.h"
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

HitRecord intersectTriangle(Ray ray, Model *model, Mesh &mesh, Vertex A, Vertex B, Vertex C);

void LightState::updateState()
{
    this->directionalLights.clear();
    this->lightRays.clear();
    DirectionalLight dl;
    for (int i = 0; i < this->primaryLights.size(); i++) {
        dl = this->ShootRay(this->primaryLights[i], 0);
        this->directionalLights.push_back(dl);
    }
}

DirectionalLight LightState::ShootRay(DirectionalLight dl, unsigned depth) {
    HitRecord result, r;
    result.t = -1;
    std::cout << "Shooting ray" << std::endl;
    unsigned nModels = this->scene->models.size();
    for (int i = 0; i < nModels; i++)
    {
        Model* model = this->scene->models[i].m;
        mat4 invM = transpose(model->invTmodelMatrix);
        vec4 op = invM*(vec4(dl.startPos, 1.));
        vec3 o = vec3(op)/op.w;
        vec3 d = vec3(invM*(vec4(dl.dir, 0)));
        Ray ray;
        ray.o = o; ray.d = d;
        unsigned nMeshes = model->meshes.size();
        for (int j = 0; j < nMeshes; j++)
        {
            Mesh mesh = model->meshes[j];
            Vertex A, B, C;
            mesh.resetTriangleIterator();
            while (mesh.nextTriangle(&A, &B, &C))
            {
                r = intersectTriangle(ray, model, mesh, A, B, C);
                if (r.t > 0.000001 && (r.t < result.t || result.t < 0.))
                {
                    //std::cout << "Intersection" << std::endl;
                    result = r;
                }
            }
        }
    }

    // At the end
    if (result.t > -1.) {
        //std::cout << "(" << result.P.x << ","<< result.P.y << "," << result.P.z << ")"<<std::endl;
        LightRay lr;
        lr.startPos = dl.startPos;
        lr.endPos = result.P;
        this->lightRays.push_back(lr);
        if (result.reflected && depth < 6)
        {
            DirectionalLight dlReflected;
            dlReflected.dir = normalize(dl.dir - 2*(dot(dl.dir,result.normal))*result.normal);
            //std::cout << "Reflected direction : (" << dlReflected.dir.x << ","<< dlReflected.dir.y << "," << dlReflected.dir.z << ")"<<std::endl;
            dlReflected.startPos = result.P+mat3(0.00001)*result.normal;
            dlReflected.radius = dl.radius;
            return ShootRay(dlReflected, depth+1);
        }
        dl.endPos = result.P;

        return dl;
    }
    else
    {
        return dl;
    }
}

HitRecord intersectTriangle(Ray ray, Model *model, Mesh &mesh, Vertex A, Vertex B, Vertex C)
{
    HitRecord r;
    r.t = -1.;
    vec3 Ap = A.Position, Bp = B.Position, Cp = C.Position;
    vec3 normal = normalize(cross(Bp-Ap, Cp-Ap)); // normal vector
    vec3 o = ray.o, d = ray.d;

    double constant = -dot(Ap, normal);
    float denom = dot(normal, d);
    if(denom == 0.)
        return r;

    float t = (-1)*(constant+dot(normal, o))/denom;
    vec3 P = o+t*d; //the intersection point

    if(t < 0)
        return r;

    //std::cout << "t:"<<t<<std::endl;

    if(dot(cross(Bp-Ap, P-Ap), normal) >= 0 &&
       dot(cross(P-Ap, Cp-Ap), normal) >= 0 &&
       dot(cross(Cp-Bp, P-Bp), normal) >= 0)
    {
        glm::vec4 v4 = model->modelMatrix*glm::vec4(P, 1.);
        r.P = glm::vec3(v4)/v4.w;
        r.t = t;
        r.normal = glm::vec3(model->invTmodelMatrix*glm::vec4(normal, 0.));
        r.reflected = mesh.isMirror;
        if (dot(normal, ray.d) > 0)
        {
            r.normal = mat3((-1.))*normal;
        }
        return r;
    }
    else
    {
        return r;
    }
}

void LightState::bindLights(Shader shader) {
    // TODO : need to support more ligths, to be done after Milestone
    vector<DirectionalLight> lights = this->getDirectionalLights();
    //std::cout << "Lights start pos : " << lights[0].startPos.x<< ", " << lights[0].startPos.y << ", " << lights[0].startPos.z <<std::endl;
    //std::cout << "Lights dir : " << lights[0].dir.x<< ", " << lights[0].dir.y << ", " << lights[0].dir.z <<std::endl;

    glUniform3f(glGetUniformLocation(shader.Program, "uniform_light_position"), lights[0].startPos.x, lights[0].startPos.y, lights[0].startPos.z);
    glUniform3f(glGetUniformLocation(shader.Program, "uniform_light_end_position"), lights[0].endPos.x, lights[0].endPos.y, lights[0].endPos.z);
    glUniform3f(glGetUniformLocation(shader.Program, "uniform_light_direction"), lights[0].dir.x, lights[0].dir.y, lights[0].dir.z);
    glUniform1f(glGetUniformLocation(shader.Program, "uniform_light_radius"), lights[0].radius);
}
