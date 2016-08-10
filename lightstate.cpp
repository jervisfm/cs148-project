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
    //std::cout << "Shooting ray" << std::endl;
    unsigned nModels = this->scene->models.size();
    for (int i = 0; i < nModels; i++)
    {
        Model* model = this->scene->models[i].m;
        if (model->isCylinder)
            continue;
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
        lr.radius = dl.radius;
        lr.depth = depth;
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
    vector<DirectionalLight> lights = this->getDirectionalLights();
    //std::cout << "Lights start pos : " << lights[0].startPos.x<< ", " << lights[0].startPos.y << ", " << lights[0].startPos.z <<std::endl;
    //std::cout << "Lights dir : " << lights[0].dir.x<< ", " << lights[0].dir.y << ", " << lights[0].dir.z <<std::endl;
    int numLights = lights.size();
    glUniform1i(glGetUniformLocation(shader.Program, "numLights"), numLights);


    for(int i = 0; i < numLights; i++)
    {
        std::ostringstream ss;
        ss << "allLights[" << i << "]";
        std::string name = ss.str();
        glUniform3f(glGetUniformLocation(shader.Program, (name+".position").c_str()), lights[i].startPos.x, lights[i].startPos.y, lights[i].startPos.z);
        glUniform3f(glGetUniformLocation(shader.Program, (name+".end_position").c_str()), lights[i].endPos.x, lights[i].endPos.y, lights[i].endPos.z);
        glUniform3f(glGetUniformLocation(shader.Program, (name+".direction").c_str()), lights[i].dir.x, lights[i].dir.y, lights[i].dir.z);
        glUniform1f(glGetUniformLocation(shader.Program, (name+".radius").c_str()), lights[i].radius);
    }
}

void LightState::drawTubes(Shader shader)
{
    vector<LightRay> lr = this->getLightRays();
    vector<glm::mat4> models;

    vector<DirectionalLight>pl = this->primaryLights;
    //First, the cylinder tops
    for(int i = 0; i < pl.size(); i++)
    {
        models.push_back(cylinderTop->cylinderTransform(pl[i].startPos, pl[i].endPos, pl[i].radius));
    }
    cylinderTop->DrawInstanced(shader, models);

    //And now, the same for the reflected cylinders themselves.
    vector<glm::mat4> reflectedModels;
    for(int i = 0; i < lr.size(); i++)
    {
        if(lr[i].depth > 0)
            reflectedModels.push_back(cylinderSecondary->cylinderTransform(lr[i].startPos, lr[i].endPos, lr[i].radius));
    }
    //Then, the cylinders for the primary lights and secondary lights
    //The order of the calls is important
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    cylinderPrimary->DrawInstanced(shader, models);
    cylinderSecondary->DrawInstanced(shader, reflectedModels);
    glCullFace(GL_BACK);
    cylinderPrimary->DrawInstanced(shader, models);
    cylinderSecondary->DrawInstanced(shader, reflectedModels);
    glDisable(GL_CULL_FACE);

}
