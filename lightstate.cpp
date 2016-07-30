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

HitRecord intersectTriangle(Ray ray, Mesh &mesh, Vertex A, Vertex B, Vertex C);

void LightState::updateState()
{
    this->directionalLights.clear();
    this->lightRays.clear();
    DirectionalLight dl;
    for (int i = 0; i < this->primaryLights.size(); i++) {
        dl = this->ShootRay(this->primaryLights[i]);
        this->directionalLights.push_back(dl);
    }
}

DirectionalLight LightState::ShootRay(DirectionalLight dl) {
    HitRecord result, r;
    result.t = -1;
    std::cout << "Shooting ray" << std::endl;
    unsigned nModels = this->scene.models.size();
    for (int i = 0; i < nModels; i++)
    {
        Model* model = this->scene.models[i].m;
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
                r = intersectTriangle(ray, mesh, A, B, C);
                if (r.t > 0.000001 && (r.t < result.t || result.t < 0.))
                {
                    std::cout << "Intersection" << std::endl;
                    result = r;
                }
            }
        }
    }

    // At the end
    if (result.t > -1.) {
        LightRay lr;
        lr.startPos = dl.startPos;
        lr.endPos = result.P;
        this->lightRays.push_back(lr);
        if (result.reflected)
        {
            DirectionalLight dlReflected;
            dlReflected.dir = normalize(dl.dir - 2*(dot(dl.dir,result.normal))*result.normal);
            dlReflected.startPos = result.P+mat3(0.00001)*result.normal;
            dlReflected.radius = dl.radius;
            return ShootRay(dlReflected);
        }
        return dl;
    }
    else
    {
        return dl;
    }
}

HitRecord intersectTriangle(Ray ray, Mesh &mesh, Vertex A, Vertex B, Vertex C)
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
    //std::cout << "(" << P.x << ","<< P.y << "," << P.z << ")"<<std::endl;
    //std::cout << "t:"<<t<<std::endl;

    if(dot(cross(Bp-Ap, P-Ap), normal) >= 0 &&
       dot(cross(P-Ap, Cp-Ap), normal) >= 0 &&
       dot(cross(Cp-Bp, P-Bp), normal) >= 0)
    {

        r.P = P;
        r.t = t;
        r.reflected = mesh.isMirror;
        if (dot(r.normal, ray.d) > 0)
        {
            normal = mat3((-1.))*normal;
        }
        r.normal = normal;
        return r;
    }
    else
    {
        return r;
    }
}
