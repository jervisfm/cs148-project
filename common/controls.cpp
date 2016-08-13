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

GLfloat Controls::lastX = 512, Controls::lastY = 384;
GLFWwindow* Controls::window;
Scene* Controls::scene;
bool Controls::firstMouse = true;
GLuint Controls::screenWidth = 1024, Controls::screenHeight = 768;
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
        std::cout << "Current position : (" << camera.Position.x << ", "<< camera.Position.y<< ", "<< camera.Position.z << ")" << std::endl;
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
        float degrees = keys[GLFW_KEY_F1] ? 1.f : -1.f;
        degrees *= deltaTime;
        Model *mirror = scene->getMirrors()[activeMirror];
        glm::mat4 mat = mirror->getModelMatrix();
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0), degrees, glm::vec3(0.f, 1.0f, 0.f));
        mirror->setModelMatrix(mat*rotate);
        ls->updateState();
    }

    /* Rotate all mummies towards the closest light source */
    /*vector<DirectionalLight> dl = ls->getDirectionalLights();
    for(int i = 0; i < models.size(); i++)
    {
        Model* m = models[i].m;
        if(m->hasMirror || m->pathName.find("pyramid") < string::npos) {
            continue;
        }
        glm::vec4 pos4 = m->getModelMatrix()*glm::vec4(0.,0.,0.,1.);
        glm::vec3 pos = glm::vec3(pos4) / pos4.w;
        // m is supposed to be a mummy
        int nearestLight = -1;
        float nearestDist = 1e10;
        for (int j = 0; j < dl.size(); j++)
        {
            float dist = glm::length(pos-dl[j].endPos);
            if (dist < nearestDist)
            {
                nearestLight = j;
                nearestDist = dist;
            }
        }

        //We have found the nearest light. Now, rotate towards it
        // by default : model looks in direction (0,0,1)
        glm::vec3 viewDirection = glm::normalize(glm::vec3(m->getModelMatrix()*glm::vec4(0.,0.,1.,0.)));
        glm::vec3 crossProd = glm::cross(viewDirection, dl[nearestLight].endPos);
        if (glm::length(crossProd) > 0.00001)
        {
            float angle = 0.3f*deltaTime;
            //if(crossProd.y < 0.)
            //    angle *= (-1.);
            glm::mat4 rotate = glm::rotate(glm::mat4(1.0), angle, crossProd);
            m->setModelMatrix(m->getModelMatrix()*rotate);
        }

    }*/
    //Turn towards the camera and go towards the player
    for(int i = 0; i < models.size(); i++)
    {
        Model* m = models[i].m;
        if(m->hasMirror || m->pathName.find("pyramid") < string::npos) {
            continue;
        }
        glm::vec4 pos4 = m->getModelMatrix()*glm::vec4(0.,0.,0.,1.);
        glm::vec3 pos = glm::vec3(pos4) / pos4.w;
        pos.y = 0.;
        //Turn to the camera
        glm::vec3 viewDirection = glm::normalize(glm::vec3(m->getModelMatrix()*glm::vec4(0.,0.,1.,0.)));
        glm::vec3 toCamera = (camera.Position - pos);
        toCamera.y = 0;
        toCamera = glm::normalize(toCamera);
        float angle = acos(glm::dot(viewDirection, toCamera));
        glm::vec3 crossProd = glm::cross(viewDirection, toCamera);
        if(abs(crossProd.y) > 0.01)
        {
            if(crossProd.y < 0)
                angle *= -1;
            //std::cout << "Angle : " << angle << std::endl;
            glm::mat4 rotate = glm::rotate(glm::mat4(1.0), angle, glm::vec3(0.,1,0));
            m->setModelMatrix(m->getModelMatrix()*rotate);
        }

        //Go towards the player
        //glm::mat4 translate = glm::translate(glm::mat4(1.0), toCamera*deltaTime);
        //m->setModelMatrix(translate*m->getModelMatrix());

    }

}

float Controls::distanceToCamera(Model* m)
{
    glm::vec4 center4 = m->getModelMatrix()*glm::vec4(0.,0.,0.,1.);
    glm::vec3 center = glm::vec3(center4)/center4.w;
    center.y = camera.Position.y;
    return glm::distance(center, camera.Position);
}

void Controls::updateState(){
    // Set frame time
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // Clear the colorbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // Check and call events
    glfwPollEvents();

}


void Controls::bindState(Shader shader)
{
    glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

void Controls::drawBorders(Shader borderShader) {
    //Now, draw the border - if necessary
    if (Controls::getActiveMirror() != nullptr)
    {
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        borderShader.Use();
        float scale = 1.08;
        Model* mirror = Controls::getActiveMirror();
        glm::mat4 oldModel = mirror->getModelMatrix();
        glm::mat3 linearMatrix = glm::mat3(scale)* glm::mat3(oldModel);
        glm::mat4 newModel = glm::translate(glm::mat4(linearMatrix), glm::inverse(linearMatrix)*mirror->lOptions.position);

        mirror->setModelMatrix(newModel);

        mirror->Draw(borderShader);
        mirror->setModelMatrix(oldModel);
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
    }
}

Camera Controls::getCamera()
{
    return camera;
}
