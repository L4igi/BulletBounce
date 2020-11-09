#include "WallCube.h"

WallCube::WallCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, glm::vec3 maxRotated, bool supportWall) : Cube(name, rotationMatrix, cube, type) {
	this->maxRotated = maxRotated;
	this->supportWall = supportWall; 
};




