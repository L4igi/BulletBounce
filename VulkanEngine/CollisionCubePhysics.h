#ifndef COLLISIONCUBEPHYSICS_H
#define COLLISIONCUBEPHYSICS_H

#include "VEInclude.h"

using namespace ve;
class CollisionCubePhysics
{
	public:
		VESceneNode* scene = nullptr;
		//VEEntity *entity = nullptr;

		float weight = 1; // cube weight

		glm::vec3 linearMomentum = glm::vec3(0, 0, 0);
		glm::vec3 angularVelocity = glm::vec3(0, 0, 0);
		glm::vec3 angularMomentum = glm::vec3(0, 0, 0);

		std::map<char, glm::vec3> cubeedges;

		//glm::vec3 touchPoint = glm::vec3(0, 0, 0);
		glm::mat3 rotationMatrix;
		std::string name;

		VESceneNode* pModel;
		VEMesh* pMesh;
		VEMaterial* pMat;
		VESceneNode* cube;

		glm::mat3 nextRotation;

		glm::mat3 inertiaMatrix = glm::mat3(1.0);
		glm::mat3 initInertiaMatrix = glm::mat3(1.0);

		glm::vec3 accleration = glm::vec3(0.0, -9.81, 0.0);
		bool collisionDone = false;

		bool collisionWithOtherObject = false;

		glm::vec3 collidingpointcubes = glm::vec3(0, 0, 0);


		CollisionCubePhysics(std::string name, VESceneNode* scene, glm::mat3 rotationMatrix);

		bool checkCollisionDone(glm::vec3 lowestPoint, veEvent event);

		glm::vec3 findLowestPointRotated();

		glm::vec3 findLowestPoint();

		void calculateCubePhysics(ve::veEvent event);

		glm::mat3 getSkewMatrix();

};

#endif

