#include "Cube.h"

#ifndef BULLETCUBE_H
#define BULLETCUBE_H 
class BulletCube :public Cube
{
private:
public:
	float shootSpeed;
	glm::vec3 shootDirection; 
	bool bulletFired = false;
	glm::vec3 spawnPosition;
	std::vector<Cube*> supportCubes; 
	bool supportBullet; 
	int supportBulletsActiveCount = 0;
	BulletCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, float shootSpeed, glm::vec3 spawnPosition, bool supportBullet);
};
#endif

