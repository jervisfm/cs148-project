#ifndef SCATTERING_H
#define SCATTERING_H

#include "../common/controls.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../lightstate.h"
// GLM Mathematics
#define GLM_FORCE_RADIANS // force everything in radian
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Scattering {
public:
    Scattering(){
	numLattice = 100;
	row = 0;
	quadVAO = 0;
	windowWidth = Controls::getScreenWidth();
    	windowHeight = Controls::getScreenHeight();
        lightingShader = Shader("lightMap.vs", "lightMap.frag");
	gaussianShader = Shader("gaussian.vs", "gaussian.frag");
	finalShader = Shader("finalPass.vs", "finalPass.frag");
    	pingpongFBO[0] = pingpongFBO[1] = 0;
	lightRenderFBO[0] = lightRenderFBO[1] = 0;
    
    }
    static void ScatterLight(LightState *ls);
    

private:
    static int numLattice;
    static int row;
    static GLuint quadVAO, quadVBO, planesVAO, planesVBO;
    static GLuint windowHeight, windowWidth;
    static Shader lightingShader, gaussianShader, finalShader;
    static GLuint pingpongFBO[2], pingpongColorbuffers[2], lightRenderFBO[2];
    static void createVirtPlanes(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos);
    static void RenderQuad();
    static GLfloat *createVirtualPlanes(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, float* arr);
};
#endif //SCATTERING_H

