//#include "scene.h"

#define M_PI = 3.141592635897932384626433832795

using namespace std;

/* Algorithm from: 
"Interactive Rendering Method for Displaying Shafts of Light"
Yoshinori Dobashi, Tsuyoshi Yamamoto, Tomoyuki Nishita
*/


uniform mat4 view;
uniform mat4 projection;
uniform vec3 uniform_light_position;
uniform vec3 uniform_light_direction;
uniform float uniform_light_radius;
uniform vec3 uniform_light_end_position;
vec3 LightColor(1.0,1.0,1.0);

out VirtualPlane vp[numLattice];

//given as an approximation of the mie scattering afffect
double phaseFunction(double radAngle){
    double constantK = 0.75; // update to change appearance
    return constantK*(1.0+9.0*pow(cos(radAngle/2.0), 16));
}

//attenuation ratio due to atmospheric particles
double BetaFunction(double distViewPlaneInter){
    double densityOfParticles = 1.2922;  //density of air(kg m^-3) Wikipedia
    double extinctionCoefficient = 0.25;
    return densityOfParticles * exp(-extinctionCoefficient*densityOfParticles*distViewPlaneInter);
}

//modifier for the intensity texture
double IntensityModifier(double phaseAnglePlaneInter, double distViewPlaneInter, 
	double distLightPlaneInter, double distBetweenPlanes){

    return phaseFunction(phaseAnglePlaneInter)*exp(-distLightPlaneInter)*BetaFunction(distViewPlaneInter)*distBetweenPlanes/ pow(distLightPlaneInter, 2);
}

//returns radians of alpha angle
//180 - Angle between two vectors.
double computeAlphaAngle(vec3 pointToLight, vec3 pointToView){
    return M_PI - acos((dot(pointToLight, pointToView)/(length(pointToLight)*length(pointToView)));
}

bool withinLight(vec3 pos){
    vec3 vecStart2End = uniform_light_end_position - uniform_light_position;
    vec3 vecPoint2End = uniform_light_end_position - pos;
    vec3 vecStart2Point = pos - uniform_light_postion ; 
    if(abs(cross(vecStart2End, vecStart2Point)) >= uniform_light_radius){
	if(dot(vecStart2Point, vecStart2End) >= 0 && dot(vecPoint2End, vecStart2End) >= 0){
            return true;
	}
    }
    return false;
}

//Code came from the following tutorial. 
//http://joshbeam.com/articles/dynamic_lightmaps_in_opengl/
unsigned int generate_LightMap(vec3 vertices[4], float s_dist, float t_dist){
    int LightMap_Size = ;
    static unsigned char data[ LightMap_Size * LightMap_Size * 3];
    static unsigned int lightmap_tex_num = 0;
    unsigned int i, j;
    vec3 pos;
    float step, s, t;
    if (lightmap_tex_num == 0)
	glGenTextures(1, &lightmap_tex_num);
    step = 1.0f / LightMap_Size;
    s = t = 0.0f;
    for ( i = 0; i < LightMap_Size; i++){
	for(j = 0; j < LightMap_Size; j++){
	    float d;
            float tmp;
            pos[0] = s_dist*s;
	    pos[1] = t_dist*t;
            pos[2] = 0.0f;
            pos[0] += vertices[0][0];
	    pos[1] += vertices[0][1];
            pos[2] += vertices[0][2];
       
            //check to see if point is within cylinder
            if(withinLight(pos)){
		tmp = 1;
	    }else{
		tmp = 0;
	    }	
	    data[i*LightMap_Size * 3 + j * 3 + 0] = (unsigned char)(255.0f * tmp * LightColor[0]);
	    data[i*LightMap_Size * 3 + j * 3 + 1] = (unsigned char)(255.0f * tmp * LightColor[1]);
            data[i*LightMap_Size * 3 + j * 3 + 3] = (unsigned char)(255.0f * tmp * LightColor[2]);
	    s += step;
        }
        t += step;
        s = 0.0f;
    }
​    glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, LightMap_Size, LightMap_Size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    return lightmap_tex_num;	    
}

VirtualPlane createVirtualPlane(vec3 vertices[4], int textureNum){
	VirtualPlane v;
	v.vertices[0] = vertices[0];
	v.vertices[1] = vertices[1];
	v.vertices[2] = vertices[2];
	v.vertices[3] = vertices[3];
	v.textureNum = textureNum;
	return v;
}

void main(){

	//cube points (front/back)(top/bottom)(left/right)
	vec4 cube[8];
	cube[0] = vec4(-1,1,-1,1); //Front Top Left
	cube[1] = vec4(1,1,-1,1);  //front top right
	cube[2] = vec4(-1,-1,-1,1); //front bottom left
	cube[3] = vec4(1,-1,-1,1); //front bottom right
	cube[4] = vec4(-1,1,1,1);
	cube[5] = vec4(1,1,1,1);
	cube[6] = vec4(-1,-1,1,1);
	cube[7] = vec4(1,-1,1,1);

	//compute frustrum's edge vectors
	mat4 inversePV = inverse(projection*view);

	vec4 transCube[8];
	vec3 finalCubeVert[8];
	//convert into world coordinates
	for(int x = 0; x < 8; x++){
		transCube[x] = inversePV * cube[x];
		//divide each point by w;
		for(int y = 0; y < 3; y++){
			finalCubeVert[x][y] = transCube[x][y]/transCube[x][3];
	}
	//calculate direction of edges of frustrum
	vec3 edgeFrustrum[4];
	edgeFrustrum[0] = finalCubeVert[4] - finalCubeVert[0]; //topLeft edge
	edgeFrustrum[1]  = finalCubeVert[5] - finalCubeVert[1]; //topRight Edge
	edgeFrustrum[2]  = finalCubeVert[6] - finalCubeVert[2]; //bottomLeft edge
	edgeFrustrum[3]  = finalCubeVert[7] - finalCubeVert[3]; //bottomRight edge
	
	//Variables for virtual plane and lattice's calculation
	double deltaT = length(edgeTopLeft)/numLattice;
	vec3 planeVert[4];
	vec3 planeEdges[2];
	double deltaNx;
	double deltaNy;
        unsigned char intensityData[n*n];

	vec3 P;			//position (in the n x n lattice)
	vec3 PToLight;          //vector from P to Light Source
	vec3 PToView;           //vector from P to View Position
	vec4 worldViewTemp = vec4(0,0,0,1)*inverse(view);
	vec3 viewPosition; 
	unsigned int light_map_tex_num = 0;

	//camera in world coordinates = inverse of 0,0,0,1 divide by w
	for(int a = 0; a < 3; a++){
	    viewPosition[a] = worldViewTemp[a]/worldView[3];
	}

	for(int i = 1; i < numLattice; i++){
	    //determine start/end points for new plane
	    for(int j = 0; j < 4; j++){
		planeVert[j] = finalCubeVert[j]+(deltaT*i)*(edgeFrustrum[j]);
	    }
	    //edge parallel to screen on x - axis
	    planeEdges[0] = planeVert[1]-planeVert[0];
	    deltaNx = length(planeEdges[0])/n;
	    //edge perpendicular to screen on y-axis
	    planeEdges[1] = planeVert[3]-planeVert[2];
	    deltaNy = length(planeEdges[1])/n;
	      
	    //compute light Intensity modifier at each point in the lattice
	    //start at top left point in the plane and traverse left to right top to bottom;
	    for(int k = 0; k < n; k++){ //y - axis
	
		for(int l = 0; l < n; k++){//move along x-axis
		    P = planeVert[0] + (deltaNx*l)*planeEdges[0] + (deltaNy*k)*planeEdges[1]);
		    //compute distance from P to camera, lightsource
		    PToLight = uniform_light_position - P;
		    PToView = viewPosition - P;
		    data[k*n+l] = (unsigned char)  (255.0f * IntensityModifier(computeAlphaAngle(PToLight, PToView), length(PToView), length(PToLight), deltaT));       
		}    
	    }
	    //need to create a light map texture for each lattice
	    //http://joshbeam.com/articles/dynamic_lightmaps_in_opengl/
	    light_map_tex_num = generate_LightMap(planeVert, length(planeEdges[0]), length(planeEdges[1]));
	    vp[i] = createVirtualPlane(planeVert, light_map_tex_num); 
	}

	//create 'virtual planes' 
	// for each light source
	// for each virtual plane
	//	compute IntensityModifier for each lattice point
	// 	create a light map texture
	//	project light map unto virtual plane
	//	render virtual plane with additive blending function 	

}
/*
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
}*/
