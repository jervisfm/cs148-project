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
#include "scattering/scattering.h"
#define GLM_FORCE_RADIANS
// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

void RenderQuad(){
    GLuint quadVAO, quadVBO;
    GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    	};

   	glGenVertexArrays(1, &quadVAO);
    	glGenBuffers(1, &quadVBO);
    	glBindVertexArray(quadVAO);
   	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    	glEnableVertexAttribArray(0);
    	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    	glEnableVertexAttribArray(1);
    	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0); 
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    auto window = initWindow(1024, 768);

    // Setup and compile our shaders
    Shader shader("shader.vs", "shader.frag");
    Shader borderShader("shader.vs", "bordershader.frag");
    Shader lightShader("lightray.vs", "lightray.frag");
    Shader finalShader("final.vs", "final.frag");
    Shader lightingShader("scattering/lightMap.vs", "scattering/lightMap.frag");
    Shader gaussianShader("scattering/gaussian.vs", "scattering/gaussian.frag"); 
    Shader finalPass("scattering/finalPass.vs", "scattering/finalPass.frag");	
    Scene* scene = new Scene();
    LightState* ls = new LightState(scene);
    ls->loadLights("map001_lights.map");
    //Load the scene
    scene->loadMap("map001.map", &shader);
    scene->loadMirrors("map001_mirrors.map", &shader);

    Scattering* lScattering = new Scattering();





    // Initialize controls
    Controls::init(window, scene, ls, glm::vec3(0.f,.75f,3.f));

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

    	// Move the camera
        Controls::Do_Movement();

        //Update the view and projection matrices
        Controls::updateState();
        Controls::bindState(shader);
        borderShader.Use();
        Controls::bindState(borderShader);
        lightShader.Use();
        Controls::bindState(lightShader);

    

        //shader.Use();
        //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//scene texture in gl_TEXTURE1        
	//GLuint sceneTexture = 
	//scene->drawScene();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //lightShader.Use();
        //glBlendFunc (GL_SRC_ALPHA, GL_ONE);
        //ls->drawTubes(lightShader);

        //Controls::drawBorders(borderShader);
  	//lightTexture store in GL_TEXTURE2
        GLuint lightTexture = lScattering->ScatterLight(ls, lightingShader, gaussianShader, finalPass);
     	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Final combination of scene and Light
	finalShader.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lightTexture);
	glUniform1i(glGetUniformLocation(finalShader.Program, "lightTexture"), 0);
	
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, sceneTexture);
	//glUniform1i(glGetUniformLocation(finalShader.Program, "sceneTexture"), 0);
	
	RenderQuad();

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

