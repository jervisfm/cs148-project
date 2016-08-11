#include "scene.h"
#include "common/controls.h"
#include <stdio.h>

using namespace std;

GLuint *createFrameBuffer(GLuint* frame_texture, int width, int height){
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

void Scene::drawScene() {
    // Draw the scene, update the stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    //0 index is Frame Buffer, 1 is the texture buffer
    //GLuint sceneBuffers[2];
    //GLuint *tempBuffer = createFrameBuffer(sceneBuffers, Controls::getScreenWidth(), Controls::getScreenHeight());
    //glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffers[0]);
    for (int i = 0; i < models.size(); i++) {
        SceneElement se = models[i];
        se.m->Draw(*(se.shader));
    }
    //return the texture Buffer.
    //return sceneBuffers[1];
}

bool Scene::nextTriangle(Vertex *A, Vertex *B, Vertex *C) {
    SceneElement se = models[modelIndex];
    Mesh mesh =  se.m->meshes[meshIndex];
    Vertex v;

    v = mesh.vertices[mesh.indices[indexIndex]];
    *A = se.m->transformPoint(v);

    v = mesh.vertices[mesh.indices[indexIndex+1]];
    *B = se.m->transformPoint(v);

    v = mesh.vertices[mesh.indices[indexIndex+2]];
    *C = se.m->transformPoint(v);
    //And now update the indices
    if (indexIndex + 3 < mesh.indices.size()) {
        indexIndex += 3;
    }
    else {
        indexIndex = 0;
        //Maybe next mesh
        if (meshIndex + 1 < se.m->meshes.size())
        {
            meshIndex += 1;
        }
        else
        {
            //This was the last mesh in the model. Get next model
            meshIndex = 0;
            if (modelIndex + 1 < models.size())
            {
                modelIndex += 1;
            }
            else
            {
                resetTriangleIterator(); // reset the iterator
                return false; //We have reached the end
            }
        }
    }
    return true;
}

void Scene::resetTriangleIterator() {
    indexIndex = meshIndex = modelIndex = 0; // reset the iterator
}

void Scene::addMirror(Model *m) {
    this->mirrors.push_back(m);
    m->hasMirror = true;
}

vector<Model*> Scene::getMirrors() {
    return this->mirrors;
}

void Scene::loadMap(const char* filename, Shader *s) {
    std::ifstream input_file_stream(filename);
    std::string file_line;
    while (std::getline(input_file_stream, file_line)) {
        // Skip over any lines in the map file that start with # -- these are comments.
        if (file_line[0] == '#') {
            continue;
        }

        char objPath[255];
        glm::vec3 pos, scale, rot;
        float degrees;
        if (sscanf(file_line.c_str(), "%s pos: (%f, %f, %f) scale: (%f, %f, %f) rot: (%f, %f, %f, %f)",
                      objPath, &pos.x, &pos.y, &pos.z, &scale.x, &scale.y, &scale.z,
                      &rot.x, &rot.y, &rot.z, &degrees) > 0)
        {
            loadOptions lOptions;
            lOptions.position = pos;
            lOptions.scale = scale;
            lOptions.rotAxis = rot;
            lOptions.rotDegrees = degrees;
            Model* model = new Model(objPath);
            model->lOptions = lOptions;
            model->setModelMatrix(model->getLoadedMatrix());
            this->addModel(model, s);
        }


    }
}

void Scene::loadMirrors(const char *filename, Shader* s)
{
    std::ifstream input_file_stream(filename);
    std::string file_line;
    while (std::getline(input_file_stream, file_line)) {
        // Skip over any lines in the map file that start with # -- these are comments.
        if (file_line[0] == '#') {
            continue;
        }

        char objPath[255];
        glm::vec3 pos, scale, rot;

        float degrees;
        if (sscanf(file_line.c_str(), "%s pos: (%f, %f, %f) scale: (%f, %f, %f) rot: (%f, %f, %f, %f)",
                      objPath, &pos.x, &pos.y, &pos.z, &scale.x, &scale.y, &scale.z,
                      &rot.x, &rot.y, &rot.z, &degrees) > 0)
        {
            loadOptions lOptions;
            lOptions.position = pos;
            lOptions.scale = scale;
            lOptions.rotAxis = rot;
            lOptions.rotDegrees = degrees;
            Model* model = new Model(objPath);
            model->meshes[0].isMirror = true;
            model->lOptions = lOptions;
            model->setModelMatrix(model->getLoadedMatrix());

            this->addModel(model, s);
            this->addMirror(model);
        }
    }
}
