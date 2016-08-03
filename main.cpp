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
    Scene* scene = new Scene();
    // Load models
    //Model ourModel("nanosuit/nanosuit.obj");
    Model ourModel("pyramid_model/pyramid.obj");

    // Draw the loaded model
    glm::mat4 model(1.);
    model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
    ourModel.setModelMatrix(model);
    scene->addModel(&ourModel, &shader);

    Model mirror("mirror1.obj");
    mirror.meshes[0].isMirror = true;
    glm::mat4 model2(1.);
    model2 = glm::translate(model2, glm::vec3(-0.5f, -0.25f, 2.f));
    model2 = glm::scale(model2, glm::vec3(0.5f, 0.5f, 0.5f));
    mirror.setModelMatrix(model2);
    //scene->addModel(&mirror, &shader);


    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //ok, test the lightstate
    LightState ls(*scene);
    DirectionalLight dl;
    dl.dir = glm::vec3(0.,0.,-1.);
    dl.startPos = glm::vec3(0., 0.0, 2.);
    dl.radius = 0.55;
    ls.addPrimaryLight(dl);

    Model sphere("sphere.obj");
    //sphere.meshes[0].isMirror = true;
    glm::mat4 model3(1.);
    model3 = glm::translate(model3, dl.startPos);
    model3 = glm::scale(model3, glm::vec3(0.2f, 0.2f, 0.2f));
    sphere.setModelMatrix(model3);
    scene->addModel(&sphere, &shader);

    Model sphere2("sphere.obj");
    //sphere.meshes[0].isMirror = true;
    glm::mat4 model4(1.);
    model4 = glm::translate(model4, dl.startPos+dl.dir);
    model4 = glm::scale(model4, glm::vec3(0.1f, 0.1f, 0.1f));
    sphere2.setModelMatrix(model4);
    scene->addModel(&sphere2, &shader);



    // Needs only to be called if the geometry changed, or if lights are added !
    ls.updateState();

    //Iterate over all triangles.
    /*Vertex A, B, C;
    while(scene->nextTriangle(&A, &B, &C))
    {
        cout << "A : " << A.Position[0] << ", " << A.Position[1] << ", " << A.Position[2] << "; " <<
                "B : " << B.Position[0] << ", " << B.Position[1] << ", " << B.Position[2] << "; " <<
                "C : " << C.Position[0] << ", " << C.Position[1] << ", " << C.Position[2] << endl;
    }*/
    // Game loop
    while(!glfwWindowShouldClose(window))
    {
        shader.Use();
        //bind the lights
        ls.bindLights(shader);

        //Update the view and projection matrices
        updateState(shader);

        // Move the camera
        Do_Movement();


        scene->drawScene();
        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

