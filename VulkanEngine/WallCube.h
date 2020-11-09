#include "Cube.h"

#ifndef WALLCUBE_H
#define WALLCUBE_H 

class WallCube : public Cube{
	private:
	public:
		glm::vec3 maxRotated;
		glm::vec3 wallNorm; 
		bool supportWall; 
	WallCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, glm::vec3 maxRotated, bool supportWall);
};

#endif

