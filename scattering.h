#ifndef SCATTERING_H
#define SCATTERING_H

#define GLM_FORCE_RADIANS
// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct VirtualPlane {
    glm::vec3 vertices[4];
    int textureNum;
};

class Scattering
{
public:
    VirtualPlane(){
	numLattice = 10;
	n = 8;

    }
private:
    int numLattice;
    int n;   // n x n lattice
};

#endif // Scattering_H
