// Std. Includes
#include <string>
#include <iostream>
#include <vector>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "common/model.h"
#include "common/setup.h"
#include "common/controls.h"
#include "lightstate.h"
#include "scene.h"
#define GLM_FORCE_RADIANS
// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>



// The MAIN function, from here we start our application and run our Game loop
int main()
{
    auto window = initWindow(1024, 768);

    // Setup and compile our shaders
    Shader shader("shader.vs", "shader.frag");
    Shader borderShader("shader.vs", "bordershader.frag");
    Shader lightShader("lightray.vs", "lightray.frag");
    Scene* scene = new Scene();

    //Load the scene
    scene->loadMap("map001.map", &shader);
    scene->loadMirrors("map001_mirrors.map", &shader);
    Model* cylinder = Model::genCylinder(40);
    cylinder->meshes[0].material.ambient = glm::vec3(0.2);
    Model* cylinderTop = Model::genCylinderTop(40);
    cylinderTop->meshes[0].material.ambient = glm::vec3(1.0);


    LightState* ls = new LightState(scene);
    DirectionalLight dl;
    dl.dir = glm::vec3(-1.5,-0.5,-.5);
    dl.startPos = glm::vec3(4.1, 2.0, 3.75);
    dl.radius = 1.55;
    ls->addPrimaryLight(dl);

    // Initialize controls
    Controls::init(window, scene, ls, glm::vec3(0.f,0.f,3.f));

    // Needs only to be called if the geometry changed, or if lights are added !
    ls->updateState();

    //Iterate over all triangles.
    // Game loop
    while(!glfwWindowShouldClose(window))
    {
        shader.Use();
        //ls->updateState();
        //bind the lights
        ls->bindLights(shader);


        //Update the view and projection matrices
        Controls::updateState();
        Controls::bindState(shader);
        borderShader.Use();
        Controls::bindState(borderShader);
        lightShader.Use();
        Controls::bindState(lightShader);

        // Move the camera
        Controls::Do_Movement();

        shader.Use();
        //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        scene->drawScene();

        lightShader.Use();
        //glBlendFunc (GL_SRC_ALPHA, GL_ONE);
        ls->drawTubes(lightShader, cylinder, cylinderTop);

        Controls::drawBorders(borderShader);


        // Swap the buffers
        glfwSwapBuffers(window);
    }
    //Output new maps
    std::cout << "Object map : " << std::endl;
    for (int i = 0; i < scene->models.size(); i++)
    {
        Model *m = scene->models[i].m;
        if (!m->hasMirror)
        {
            std::cout << m->genMapDirective() << std::endl;
        }
    }
    // And the mirrors
    std::cout << "==================" << std::endl << "Mirror map :" << std::endl;
    std::vector<Model*> mirrors = scene->getMirrors();
    for (int i = 0; i < mirrors.size(); i++) {
        std::cout << mirrors[i]->genMapDirective() << std::endl;
    }
    glfwTerminate();
    return 0;
}

