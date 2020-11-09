#include "Cube.h"

#ifndef PLAYERCUBE_H
#define PLAYERCUBE_H

class PlayerCube : public Cube{
	private:
	public: 
		float movementSpeed; 
		glm::vec3 movementDirection; 
		std::vector<Cube*> availableBullets; 
		bool liveLost = false;
	PlayerCube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type, float movementSpeed);
};

#endif