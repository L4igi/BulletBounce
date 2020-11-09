#include "PlayerCube.h"

PlayerCube::PlayerCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, float movementSpeed): Cube(name, rotationMatrix, cube, type) {
	this->movementSpeed = movementSpeed;
};