#include "BulletCube.h"

BulletCube::BulletCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type,  float shootspeed, glm::vec3 spawnPosition, bool supportBullet) : Cube(name, rotationMatrix, cube, type) {
	this->shootSpeed = shootSpeed;
	this->spawnPosition = spawnPosition; 
	this->supportBullet = supportBullet; 
};