#include "EnemyCube.h"

EnemyCube::EnemyCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, float movementSpeed, std::string enemyType) : Cube(name, rotationMatrix, cube, type) {
	this->movementSpeed = movementSpeed;
	this->enemyType = enemyType;
};
