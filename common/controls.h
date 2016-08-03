#ifndef CONTROLS_H
#define CONTROLS_H
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
#include "../Camera.h"
#include "../Shader.h"
#include "../scene.h"
#include "../lightstate.h"

class Controls {
public:
    static void init(GLFWwindow* w, Scene* s, LightState* ls, glm::vec3 initPos){
        camera  = Camera(initPos);
        window = w;
        scene = s;
        Controls::ls = ls;
        glfwSetKeyCallback(window, Controls::key_callback);
        glfwSetCursorPosCallback(window, Controls::mouse_callback);
        glfwSetScrollCallback(window, Controls::scroll_callback);

    }

    static void Do_Movement();
    static void updateState(Shader shader);
    static Camera getCamera();
private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    ~Controls();
    static GLFWwindow* window;
    static GLfloat lastX, lastY;
    static bool firstMouse;
    static Camera camera;
    static GLfloat deltaTime;
    static GLfloat lastFrame;
    static bool keys[1024];
    static GLuint screenWidth, screenHeight;
    static Scene* scene;
    static LightState* ls;
    static unsigned activeMirror;

};



#endif // CONTROLS_H
