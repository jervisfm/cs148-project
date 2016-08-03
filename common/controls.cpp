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
unsigned Controls::activeMirror = -1;

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
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Compute the new active mirror
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_S] || keys[GLFW_KEY_A] || keys[GLFW_KEY_D])
    {
        activeMirror = -1;
        vector<Model*> mirrors = scene->getMirrors();
        for(int i = 0; i < mirrors.size(); i++)
        {
            // Compute the distance from the camera to the mirror. Do this by supposing that the center of the model
            // is at (0,0,0)
            glm::vec4 center4 = mirrors[i]->getModelMatrix()*glm::vec4(0.,0.,0.,1.);
            glm::vec3 center = glm::vec3(center4)/center4.w;
            center.y = camera.Position.y;
            float distance = glm::distance(center, camera.Position);
            if (distance < 5.)
            {
                activeMirror = i;
            }
        }
    }

    //Also, change the position of the mirror (only one for the moment)
    if((keys[GLFW_KEY_F1] || keys[GLFW_KEY_F2]) && activeMirror != -1)
    {
        float degrees = keys[GLFW_KEY_F1] ? 3.f : -3.f;
        Model *mirror = scene->getMirrors()[activeMirror];
        glm::mat4 mat = mirror->getModelMatrix();
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0), degrees*deltaTime, glm::vec3(0.f, 1.0f, 0.f));
        mirror->setModelMatrix(mat*rotate);
        ls->updateState();
    }
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
