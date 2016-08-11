// CS148 Summer 2016 Homework 3 - Shaders
// Do not modify this file; you should only
// need to fill out phong.vs and phong.frag .

#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#define GLM_FORCE_RADIANS // force everything in radian
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "../Shader.h"
#include "../Camera.h"
//#include <SOIL.h>
#include "scattering.h"
#include "../common/controls.h"
#include "../lightstate.h"

// Import symbols from GLM
using namespace glm;

int Scattering::numLattice = 0;
int Scattering::row = 0;
GLuint Scattering::quadVAO = 0, Scattering::quadVBO = 0, Scattering::planesVAO = 0, Scattering::planesVBO = 0;
GLuint Scattering::windowHeight = 0, Scattering::windowWidth = 0;
GLuint Scattering::pingpongFBO[2] = {0,0}, Scattering::pingpongColorbuffers[2]={0,0}, Scattering::lightRenderFBO[2]={0,0};

glm::vec3 scale3(glm::vec3 vector, float scalar){
    vector[0] = vector[0] * scalar;
    vector[1] = vector[1] * scalar;
    vector[2] = vector[2] * scalar;
    return vector;
}   

glm::vec3 oneMinusVect3(glm::vec3 vector){
    vector[0] = 1 - vector[0];
    vector[1] = 1 - vector[1];
    vector[2] = 1 - vector[2];
    return vector;
	
}

//int numLattice = 100;
//int row = 0;

//http://learnopengl.com/code_viewer.php?code=advanced-lighting/bloom
//GLuint quadVAO = 0;
//GLuint quadVBO;
void Scattering::RenderQuad(){
    if(quadVAO == 0){
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
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0); 
}

//http://joshbeam.com/articles/dynamic_lightmaps_in_opengl/
GLfloat* Scattering::createVirtualPlanes(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, float* arr, glm::vec3 startPos, glm::vec3 endPos){

	glm::vec4 cube[8];
	cube[0] = glm::vec4(-1, 1,-1,1); //Front Top Left
	cube[1] = glm::vec4( 1, 1,-1,1);  //front top right
	cube[2] = glm::vec4(-1,-1,-1,1); //front bottom left
	cube[3] = glm::vec4( 1,-1,-1,1); //front bottom right
	cube[4] = glm::vec4(-1, 1, 1,1);
	cube[5] = glm::vec4( 1, 1, 1,1);
	cube[6] = glm::vec4(-1,-1, 1,1);
	cube[7] = glm::vec4( 1,-1, 1,1);

	//compute frustrum's edge vectors
	glm::mat4 inversePV = inverse(projection*view);

	glm::vec4 transCube[8];
	glm::vec3 finalCubeVert[8];
	//convert into world coordinates
	for(int x = 0; x < 8; x++){
		
		transCube[x] = inversePV*cube[x];
		//divide each point by w;
		for(int y = 0; y < 3; y++){
			finalCubeVert[x][y] = transCube[x][y]/transCube[x][3];
		}
		//std::cout<<finalCubeVert[x].x<<" "<<finalCubeVert[x].y<<" "<<finalCubeVert[x].z<<std::endl;
	}
	//calculate direction of edges of frustrum
	glm::vec3 edgeFrustrum[4];
	edgeFrustrum[0] = finalCubeVert[4] - finalCubeVert[0]; //topLeft edge
	edgeFrustrum[1]  = finalCubeVert[5] - finalCubeVert[1]; //topRight Edge
	edgeFrustrum[2]  = finalCubeVert[6] - finalCubeVert[2]; //bottomLeft edge
	edgeFrustrum[3]  = finalCubeVert[7] - finalCubeVert[3]; //bottomRight edge
	
	//Variables for virtual plane and lattice's calculation
	//double deltaT = length(edgeFrustrum[0])/(numLattice*length(edgeFrustrum[0]));
	glm::vec3 planeVert[4];
	glm::vec3 planeEdges[2];
	//double deltaNx;
	//double deltaNy;
	
	//Place planes inside the light's frustrum (z coordinate of beginning and end)
	float startT = (startPos.z - finalCubeVert[0].z) /edgeFrustrum[0].z; 
	float endT = (endPos.z - finalCubeVert[0].z) /edgeFrustrum[0].z; 
	float deltaT = (startT-endT)/((startT-endT)*numLattice);
	//std::cout<<"startT is: "<<startT<<std::endl;	
	//std::cout<<"endT is: "<<endT<<std::endl;
	//std::cout<<"deltaT is: "<<deltaT<<std::endl;

	for(int i = 0; i < numLattice; i++){
	    //determine start/end points for new plane
	    for(int j = 0; j < 4; j++){
		planeVert[j] = finalCubeVert[j]+edgeFrustrum[j]*(float)(startT+(endT-startT)*pow((deltaT*i),3));
		//std::cout<<"Plane Vertices: "<<planeVert[j].z<<std::endl;

	    }
	    //add vertices for bottom left, top left, top right (2,1,0)
	    //int l = 0;
	    for(int k = 2; k >= 0; k--){
		arr[8*row] = planeVert[k].x;
		arr[8*row+1] = planeVert[k].y;
		arr[8*row+2] = planeVert[k].z;
		//attribute normals			
		arr[8*row+3] = 0.0;
		arr[8*row+4] = 0.0;
		arr[8*row+5] = 1.0;
		if(k==2){
		    arr[8*row+6] = 0.0;
		    arr[8*row+7] = 0.0;
		}else if(k==1){
		    arr[8*row+6] = 1.0;
		    arr[8*row+7] = 1.0;
		}
		else{
		    arr[8*row+6] = 0.0;
		    arr[8*row+7] = 1.0;
		}	    	
		row++;
	    }

	    //add vertices for top right bottom left bottom right (2, 3, 1)
	    for (int k = 2; k < 4; k++){
		arr[8*row] = planeVert[k].x;
		arr[8*row+1] = planeVert[k].y;
		arr[8*row+2] = planeVert[k].z;
		//attribute normals		
		arr[8*row+3] = 0.0;
		arr[8*row+4] = 0.0;
		arr[8*row+5] = 1.0;
		if(k==2){
		    arr[8*row+6] = 0.0;
		    arr[8*row+7] = 0.0;
		}else{
		    arr[8*row+6] = 1.0;
		    arr[8*row+7] = 0.0;
		}
	    	row++;
	    }
                arr[8*row] = planeVert[1].x;
		arr[8*row+1] = planeVert[1].y;
		arr[8*row+2] = planeVert[1].z;
		//attribute normals		
		arr[8*row+3] = 0.0;
		arr[8*row+4] = 0.0;
		arr[8*row+5] = 1.0;
		arr[8*row+6] = 1.0;
		arr[8*row+7] = 1.0;
		row++;		
	}
	row = 0;
        return arr;
}

static GLuint *createFrameBuffer(GLuint* frame_texture, int width, int height){
	//frame_texture is a pointer to the frame buffer and texture buffer numbers. 
    
    //http://learnopengl.com/code_viewer.php?code=advanced/framebuffers_screen_texture
    glGenFramebuffers(1, &frame_texture[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_texture[0]);  
    // Create a color attachment texture
    //GLuint textureColorBuffer; 
    glGenTextures(1, &frame_texture[1]);
    glBindTexture(GL_TEXTURE_2D, frame_texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture[1], 0);
    // Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // Use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // Now actually attach it
    // Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return frame_texture;
}

//GLuint containerVAO = 0;
//GLuint planeObject;
void Scattering::createVirtPlanes(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, glm::vec3 startPos, glm::vec3 endPos){
    GLfloat vert[48*numLattice];
    GLfloat *vertices = createVirtualPlanes(view, projection, viewPos, vert, startPos, endPos);
    if(planesVAO == 0){
	// First, set the container's VAO 
    	glGenVertexArrays(1, &planesVAO);
	glGenBuffers(1, &planesVBO);
	   
    }
    glBindVertexArray(planesVAO); 
    glBindBuffer(GL_ARRAY_BUFFER, planesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TEXTURE COORDINATE ATTRIBUE
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(planesVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6*numLattice);	
    glBindVertexArray(0);
    
}

// The MAIN function, from here we start the application and run the game loop
void Scattering::ScatterLight(LightState* ls, Shader lightingShader, Shader gaussianShader, Shader finalShader)
{
    
   
    glEnable(GL_DEPTH_TEST);
    Camera camera = Controls::getCamera();
    std::vector<DirectionalLight> lights = ls->getDirectionalLights();
    
    //FRAME BUFFER for the SCREEN
    //http://learnopengl.com/code_viewer.php?code=advanced/framebuffers_screen_texture
    //GLuint LightRender[2];
    //GLuint *lFrameBuffer = createFrameBuffer(LightRender, w,h);
    if(lightRenderFBO[0] == 0){
	GLuint *lFBO_ptr = createFrameBuffer(lightRenderFBO, windowWidth, windowHeight);
    }	    

    //FRAME BUFFER FOR THE GAUSSIAN
    if(pingpongFBO[0] == 0){
	    //GLuint pingpongFBO[2];
	    //GLuint pingpongColorbuffers[2];
	    glGenFramebuffers(2, pingpongFBO);
	    glGenTextures(2, pingpongColorbuffers);
	    for (GLuint i = 0; i < 2; i++)
	    {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// Also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		    std::cout << "Framebuffer not complete!" << std::endl;
	    }
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //while (!glfwWindowShouldClose(window))
    //{
    glBindFramebuffer(GL_FRAMEBUFFER, lightRenderFBO[0]);

        // Clear the colorbuffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
        // Use cooresponding shader when setting uniforms/drawing objects
    lightingShader.Use();
    GLint lightColorLoc  = glGetUniformLocation(lightingShader.Program, "lightColor");
    GLint lightPosLoc    = glGetUniformLocation(lightingShader.Program, "lightPos");
    GLint viewPosLoc     = glGetUniformLocation(lightingShader.Program, "viewPos");
    glUniform3f(lightColorLoc,  1.0f, 1.0f, 1.0f);
    glUniform3f(lightPosLoc,    lights[0].startPos.x, lights[0].startPos.y, lights[0].startPos.z);
    glUniform3f(viewPosLoc,     camera.Position.x, camera.Position.y, camera.Position.z);
    glUniform3f(glGetUniformLocation(lightingShader.Program, "endLightPos"), lights[0].endPos.x, lights[0].endPos.y, lights[0].endPos.z);
    glUniform1f(glGetUniformLocation(lightingShader.Program, "lightRadius"), (float)lights[0].radius);
    //std::cout<<"Radius is "<<lights[0].radius<<std::endl;
        // Create camera transformations
    //glm::mat4 view = camera.GetViewMatrix();
    //    glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
    	Controls::bindState(lightingShader);   
    // Get the uniform locations
        //GLint viewLoc  = glGetUniformLocation(lightingShader.Program,  "view");
        //GLint projLoc  = glGetUniformLocation(lightingShader.Program,  "projection");
	        
	// Pass the matrices to the shader
        //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        //glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	//MAKE DYNAMIC VIRTUAL PLANES:
	createVirtPlanes(Controls::getViewMatrix(), Controls::getProjectionMatrix(), camera.Position, lights[0].startPos, lights[0].endPos);
	
	//glBindVertexArray(containerVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6*numLattice);	
        //glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//GAUSSIAN SHADER
	gaussianShader.Use();
	bool horizontal = true;
	bool first_iteration = true;
	GLint amount = 10;
 	for (GLuint i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); 
            glUniform1i(glGetUniformLocation(gaussianShader.Program, "horizontal"), horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? lightRenderFBO[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            RenderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindTexture(GL_TEXTURE_2D, lFrameBuffer[1]);   

        //BIND TO DEFAULT FRAME BUFFER
	finalShader.Use();
	//glBindFramebuffer(GL_FRAMEBUFFER, lightRenderFBO[0]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);	
	RenderQuad();
   	//return lightRenderFBO[1];
	
        // Swap the screen buffers
    //    glfwSwapBuffers(window);
    //}
    //glDeleteVertexArrays(1, &containerVAO);
    //glDeleteBuffers(1, &planeObject);

    //glfwTerminate();
    //return 0;
}

