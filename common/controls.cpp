// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
#include "../Camera.h"
#include "../Shader.h"
#include "controls.h"
// GLM Mathemtics
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLfloat Controls::lastX = 400, Controls::lastY = 300;
GLFWwindow* Controls::window;
Scene* Controls::scene;
bool Controls::firstMouse = true;
GLuint Controls::screenWidth = 800, Controls::screenHeight = 600;
GLfloat Controls::deltaTime = 0.0f, Controls::lastFrame = 0.0f;
bool Controls::keys[1024];
Camera Controls::camera;
LightState* Controls::ls;
int Controls::activeMirror = -1, Controls::activeEditing = -1;
bool Controls::editMode = false, Controls::modelSelected = false;



void Controls::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

void Controls::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void Controls::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// Moves/alters the camera positions based on user input
void Controls::Do_Movement()
{
    glm::vec3 displacement = glm::vec3(0.);
    vector<SceneElement> models = scene->models;
    // Camera controls
    if(keys[GLFW_KEY_W])
        displacement += camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        displacement += camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        displacement += camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
         displacement += camera.ProcessKeyboard(RIGHT, deltaTime);


    if(keys[GLFW_KEY_E] && !modelSelected) { // Toggle edit mode
        editMode = !editMode;
        keys[GLFW_KEY_E] = false;
        //std::cout << "Pressed E !"<<std::endl;
        if(!editMode)
        {
            if (activeEditing != -1)
            {
                for (int i = 0; i < models[activeEditing].m->meshes.size(); i++)
                    models[activeEditing].m->meshes[i].material.ambient = glm::vec3(0.,0.,0.);
            }
            activeEditing = -1;
        }
    }
    if(keys[GLFW_KEY_ENTER] && activeEditing > -1)
    {
        modelSelected = !modelSelected;
        keys[GLFW_KEY_ENTER] = false;
        //std::cout << "Pressed Enter !"<<std::endl;
    }

    // Compute the new active mirror, and active object
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_S] || keys[GLFW_KEY_A] || keys[GLFW_KEY_D])
    {
        //std::cout << "Current position : (" << camera.Position.x << ", "<< camera.Position.y<< ", "<< camera.Position.z << ")" << std::endl;
        int newActiveMirror = -1;
        vector<Model*> mirrors = scene->getMirrors();
        for(int i = 0; i < mirrors.size(); i++)
        {
            // Compute the distance from the camera to the mirror. Do this by supposing that the center of the model
            // is at (0,0,0)
            if (distanceToCamera(mirrors[i]) < 5.)
            {
                newActiveMirror = i;
            }
        }

        if (newActiveMirror != activeMirror) { //something changed
            if (activeMirror != -1)
            {
                mirrors[activeMirror]->meshes[0].material.ambient = glm::vec3(0.,0.,0.);
            }
            if (newActiveMirror != -1)
            {
                mirrors[newActiveMirror]->meshes[0].material.ambient = glm::vec3(.5,0.5,0.5);
            }
        }

        activeMirror = newActiveMirror;


        //If in edit mode, and not currently moving an object : select the nearest object for
        //active editing.

        if (editMode && !modelSelected)
        {
            int oldSelected = activeEditing;
            activeEditing = -1;
            float nearest = 10.;
            for(int i = 0; i < models.size(); i++)
            {
                float distance = distanceToCamera(models[i].m);
                if (distance < 9. && distance < nearest)
                {
                    activeEditing = i;
                    nearest = distance;
                }
            }
            if (oldSelected != activeEditing) { //something changed
                if (oldSelected != -1)
                {
                    for (int i = 0; i < models[oldSelected].m->meshes.size(); i++)
                        models[oldSelected].m->meshes[i].material.ambient = glm::vec3(0.,0.,0.);
                }
                if (activeEditing != -1)
                {
                    for (int i = 0; i < models[activeEditing].m->meshes.size(); i++)
                        models[activeEditing].m->meshes[i].material.ambient = glm::vec3(.5,0,0);
                }
            }

        }
        else if(editMode && modelSelected && activeEditing != -1) {
            glm::mat4 mat = models[activeEditing].m->getModelMatrix();
            glm::mat3 correction = glm::inverse(glm::mat3(mat));
            glm::vec3 corrDisplacement = correction*displacement;
            mat = glm::translate(mat, corrDisplacement);
            models[activeEditing].m->setModelMatrix(mat);
            models[activeEditing].m->lOptions.position += displacement;
            //std::cout << models[activeEditing].m->genMapDirective();
        }


    }

    //Also, change the position of the active mirror
    if((keys[GLFW_KEY_F1] || keys[GLFW_KEY_F2]) && activeMirror != -1)
    {
        float degrees = keys[GLFW_KEY_F1] ? 3.f : -3.f;
        degrees *= deltaTime;
        Model *mirror = scene->getMirrors()[activeMirror];
        glm::mat4 mat = mirror->getModelMatrix();
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0), degrees, glm::vec3(0.f, 1.0f, 0.f));
        mirror->setModelMatrix(mat*rotate);
        ls->updateState();
    }
}

float Controls::distanceToCamera(Model* m)
{
    glm::vec4 center4 = m->getModelMatrix()*glm::vec4(0.,0.,0.,1.);
    glm::vec3 center = glm::vec3(center4)/center4.w;
    center.y = camera.Position.y;
    return glm::distance(center, camera.Position);
}

void Controls::updateState(Shader shader){
    // Set frame time
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Transformation matrices
    glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // Clear the colorbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Check and call events
    glfwPollEvents();
}

Camera Controls::getCamera()
{
    return camera;
}
