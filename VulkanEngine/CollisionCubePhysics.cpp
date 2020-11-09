#include "CollisionCubePhysics.h"

CollisionCubePhysics::CollisionCubePhysics(std::string name, VESceneNode* scene, glm::mat3 rotationMatrix) {
		VECHECKPOINTER(pMesh = getSceneManagerPointer()->getMesh("media/models/test/crate0/cube.obj/cube"));
		VECHECKPOINTER(pMat = getSceneManagerPointer()->getMaterial("media/models/test/crate0/cube.obj/cube"));

		VECHECKPOINTER(cube = getSceneManagerPointer()->createEntity(name, pMesh, pMat, scene));

		int randx = rand() % 5 + (-2);

		int randz = rand() % 5 + (2);

		cube->setTransform(rotationMatrix);

		cube->setPosition(glm::vec3(randx, 5.0f, randz));

		this->scene = scene;
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

	bool CollisionCubePhysics::checkCollisionDone(glm::vec3 lowestPoint, veEvent event) {
		int pointbeneathzerocount = 0;
		for (std::map<char, glm::vec3>::iterator it = cubeedges.begin(); it != cubeedges.end(); ++it) {
			glm::vec3 np = rotationMatrix * it->second + cube->getPosition();
			auto lowestnp = lowestPoint + event.dt * linearMomentum / weight;
			if (np.y <= (lowestnp.y + 0.1)) {
				pointbeneathzerocount++;
			}
		}
		if (pointbeneathzerocount > 3) {
			return true;
		}
		return false;
	}

	glm::vec3 CollisionCubePhysics::findLowestPointRotated() {
		glm::vec3 result = cubeedges.at('a');

		for (std::map<char, glm::vec3>::iterator it = cubeedges.begin(); it != cubeedges.end(); ++it) {
			glm::vec3 np = rotationMatrix * it->second + cube->getPosition();
			if (np.y < result.y) {
				result = np;
			}
		}
		return result;
	}
	glm::vec3 CollisionCubePhysics::findLowestPoint() {
		glm::vec3 result = cubeedges.at('a');

		for (std::map<char, glm::vec3>::iterator it = cubeedges.begin(); it != cubeedges.end(); ++it) {
			glm::vec3 np = rotationMatrix * it->second + cube->getPosition();
			if (np.y < result.y) {
				result = it->second;
			}
		}
		return result;
	}

	void CollisionCubePhysics::calculateCubePhysics(ve::veEvent event) {
		int collisioncount = 0;
		nextRotation = rotationMatrix;

		glm::vec3 nextPosition = cube->getPosition() + event.dt * linearMomentum / weight;
		nextRotation += event.dt * getSkewMatrix() * rotationMatrix;
		nextRotation = orthonormalize(nextRotation);
		inertiaMatrix = rotationMatrix * glm::inverse(inertiaMatrix) * glm::transpose(rotationMatrix);
		angularVelocity = inertiaMatrix * angularMomentum;
		linearMomentum += event.dt * accleration;

		angularVelocity = inertiaMatrix * angularMomentum;



		glm::vec3 lowestPoint = findLowestPointRotated();

		if (collisionWithOtherObject) {

		}

		if (lowestPoint.y < 0.5 && !collisionDone) {
			collisioncount++;

			float distance = 0.5 - lowestPoint.y;
			nextPosition.y = nextPosition.y + distance;

			lowestPoint = nextRotation * findLowestPoint() + nextPosition;

			glm::vec3 contactPointVector = lowestPoint - cube->getPosition();


			glm::vec3 contactPointVelocity = linearMomentum + glm::cross(contactPointVector, angularVelocity);

			glm::vec3 plainNormal = glm::vec3(0, 1, 0);

			float closingVelocity = glm::dot(contactPointVelocity, plainNormal);

			glm::vec3 t_ = glm::vec3(0.0);
			t_ = glm::normalize(contactPointVelocity - closingVelocity * plainNormal);


			float collisionResponse = (-(1 + 0.15) * closingVelocity) /
				(1 + glm::dot(glm::cross(inertiaMatrix * glm::cross(contactPointVector, plainNormal), contactPointVector), plainNormal));

			float friction = -0.25;

			glm::vec3 collisionResponseVector = collisionResponse * plainNormal;

			if (!isnan(t_.x) && !isnan(t_.z)) {
				collisionResponseVector += friction * collisionResponse * t_;
			}

			angularMomentum += glm::cross(collisionResponseVector, contactPointVector);
			angularVelocity = inertiaMatrix * angularMomentum;

			linearMomentum += collisionResponseVector;
			rotationMatrix = nextRotation;
			cube->setTransform(rotationMatrix);
			cube->multiplyTransform(glm::translate(glm::mat4(1.0), nextPosition));

			if (checkCollisionDone(lowestPoint, event)) {
				angularMomentum = glm::vec3(0);
				angularVelocity = glm::vec3(0);
				linearMomentum = glm::vec3(0);
				collisionDone = true;
				cube->multiplyTransform(glm::translate(glm::mat4(1), glm::vec3(0, 1 - cube->getPosition().y, 0)));
			}

		}
		else if (!collisionDone) {
			rotationMatrix = nextRotation;
			cube->setTransform(rotationMatrix);
			cube->multiplyTransform(glm::translate(glm::mat4(1.0), nextPosition));
		}
	}

	glm::mat3 CollisionCubePhysics::getSkewMatrix() {
		glm::mat3 result;
		result[0][0] = 0.0;
		result[0][1] = -angularVelocity.z;
		result[0][2] = +angularVelocity.y;
		result[1][0] = +angularVelocity.z;
		result[1][1] = 0.0;
		result[1][2] = -angularVelocity.x;
		result[2][0] = -angularVelocity.y;
		result[2][1] = +angularVelocity.x;
		result[2][2] = 0.0;
		return result;
	}


