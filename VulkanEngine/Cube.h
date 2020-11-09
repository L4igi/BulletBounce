#ifndef CUBE_H
#define CUBE_H

#include "VEInclude.h"
#include "CubeTypes.h"

using namespace ve;

class Cube
{
	private:
	public:
		//VEEntity *entity = nullptr;

		float weight = 1; // cube weight

		glm::vec3 linearMomentum = glm::vec3(0, 0, 0);
		glm::vec3 angularVelocity = glm::vec3(0, 0, 0);
		glm::vec3 angularMomentum = glm::vec3(0, 0, 0);

		std::map<char, glm::vec3> cubeedges;

		//glm::vec3 touchPoint = glm::vec3(0, 0, 0);
		glm::mat3 rotationMatrix;
		std::string name;

		VESceneNode* cube;

		glm::mat3 nextRotation;

		glm::mat3 inertiaMatrix = glm::mat3(1.0);
		glm::mat3 initInertiaMatrix = glm::mat3(1.0);

		glm::vec3 accleration = glm::vec3(0.0, -9.81, 0.0);
		bool collisionDone = false;

		bool collisionWithOtherObject = false;

		glm::vec3 collidingpointcubes = glm::vec3(0, 0, 0);

		glm::vec3 mtv = glm::vec3(0);
		glm::vec3 nextPosition = glm::vec3(0);

		std::string cubeType; 

		Cube(std::string name, glm::mat3 rotationMatrix, VESceneNode* cube, std::string type);

		int collisionCount = 0;

	};

#endif

