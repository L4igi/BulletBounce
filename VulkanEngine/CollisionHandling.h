#include "WallCube.h"#include "PlayerCube.h"using namespace ve;

#ifndef COLLISIONHANDLING_H
#define COLLISIONHANDLING_H

bool checkCollisionDone(Cube *c1, glm::vec3 lowestPoint, veEvent event);

glm::vec3 findLowestPointRotated(Cube *c1);

glm::vec3 findLowestPoint(Cube *c1);

void calculateCollisionRespones(Cube *c1, glm::vec3 contactPointVector);

void calculateCubePhysics(Cube *c1, ve::veEvent event);

glm::mat3 getSkewMatrix(Cube *c1);

glm::vec3 support(Cube *c1, glm::vec3 dir);

bool gjk(Cube *c1, Cube *c2);

void update_simplex3(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir);

bool update_simplex4(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir);

#endif
