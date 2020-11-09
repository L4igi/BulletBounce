#include "Cube.h"

#ifndef ENEMYCUBE_H
#define ENEMYCUBE_H
class EnemyCube : public Cube
{
private:
public:
	float movementSpeed;
	glm::vec3 movementDirection;
	std::vector<Cube*> availableBullets;
	std::string enemyType; 
	float triggerDistance =0.0f; 
	bool collWall = false;
	EnemyCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, float movementSpeed, std::string enemyType);
};
#endif
