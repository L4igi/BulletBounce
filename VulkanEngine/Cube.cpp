#include "Cube.h"


Cube::Cube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type) {
	this->cube = cube;
	this->cubeType = type; 

	if (cubeType == "FallingObject") {
		int randx = rand() % 5 + (-2);

		int randz = rand() % 5 + (2);

		cube->setTransform(rotationMatrix);

		//cube->setPosition(glm::vec3(randx, 5.0f, randz));
		cube->setPosition(glm::vec3(0, 5.0f, 0));

	}
	if (cubeType == "Player") {
		cube->setPosition(glm::vec3(0, 1.0f, 0));
	}
	this->name = name;
	this->rotationMatrix = rotationMatrix;

	cubeedges['a'] = glm::vec3(-0.5, 0.5, -0.5);
	cubeedges['b'] = glm::vec3(-0.5, 0.5, 0.5);
	cubeedges['c'] = glm::vec3(0.5, 0.5, 0.5);
	cubeedges['d'] = glm::vec3(0.5, 0.5, -0.5);
	cubeedges['e'] = glm::vec3(-0.5, -0.5, -0.5);
	cubeedges['f'] = glm::vec3(-0.5, -0.5, 0.5);
	cubeedges['g'] = glm::vec3(0.5, -0.5, 0.5);
	cubeedges['h'] = glm::vec3(0.5, -0.5, -0.5);

}
