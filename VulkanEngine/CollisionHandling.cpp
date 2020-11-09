#include"CollisionHandling.h"

bool checkCollisionDone(Cube *c1, glm::vec3 lowestPoint, veEvent event) {
	int pointbeneathzerocount = 0;
	for (std::map<char, glm::vec3>::iterator it = c1->cubeedges.begin(); it != c1->cubeedges.end(); ++it) {
		glm::vec3 np = c1->rotationMatrix * it->second + c1->cube->getPosition();
		auto lowestnp = lowestPoint + event.dt * c1->linearMomentum / c1->weight;
		if (np.y <= (lowestnp.y + 0.1)) {
			pointbeneathzerocount++;
		}
	}
	if (pointbeneathzerocount > 3) {
		return true;
	}
	return false;
}

glm::vec3 findLowestPointRotated(Cube *c1) {
	glm::vec3 result = c1->cubeedges.at('a');

	for (std::map<char, glm::vec3>::iterator it = c1->cubeedges.begin(); it != c1->cubeedges.end(); ++it) {
		glm::vec3 np = c1->rotationMatrix * it->second + c1->cube->getPosition();
		if (np.y < result.y) {
			result = np;
		}
	}
	return result;
}
glm::vec3 findLowestPoint(Cube *c1) {
	glm::vec3 result = c1->cubeedges.at('a');

	for (std::map<char, glm::vec3>::iterator it = c1->cubeedges.begin(); it != c1->cubeedges.end(); ++it) {
		glm::vec3 np = c1->rotationMatrix * it->second + c1->cube->getPosition();
		if (np.y < result.y) {
			result = it->second;
		}
	}
	return result;
}

void calculateCollisionRespones(Cube *c1, glm::vec3 contactPointVector) {
	glm::vec3 contactPointVelocity = c1->linearMomentum + glm::cross(contactPointVector, c1->angularVelocity);

	glm::vec3 plainNormal = glm::vec3(0, 1, 0);

	float closingVelocity = glm::dot(contactPointVelocity, plainNormal);

	glm::vec3 t_ = glm::vec3(0.0);
	t_ = glm::normalize(contactPointVelocity - closingVelocity * plainNormal);


	float collisionResponse = (-(1 + 0.15) * closingVelocity) /
		(1 + glm::dot(glm::cross(c1->inertiaMatrix * glm::cross(contactPointVector, plainNormal), contactPointVector), plainNormal));

	float friction = -0.25;

	glm::vec3 collisionResponseVector = collisionResponse * plainNormal;

	if (!isnan(t_.x) && !isnan(t_.z)) {
		collisionResponseVector += friction * collisionResponse * t_;
	}

	c1->angularMomentum += glm::cross(collisionResponseVector, contactPointVector);
	c1->angularVelocity = c1->inertiaMatrix * c1->angularMomentum;

	c1->linearMomentum += collisionResponseVector;
	c1->rotationMatrix = c1->nextRotation;
	c1->cube->setTransform(c1->rotationMatrix);
	c1->cube->multiplyTransform(glm::translate(glm::mat4(1.0), c1->nextPosition));

}

void calculateCubePhysics(Cube *c1, ve::veEvent event) {
	c1->nextRotation = c1->rotationMatrix;

	c1->nextPosition = c1->cube->getPosition() + event.dt * c1->linearMomentum / c1->weight;
	c1->nextRotation += event.dt * getSkewMatrix(c1) * c1->rotationMatrix;
	c1->nextRotation = orthonormalize(c1->nextRotation);
	c1->inertiaMatrix = c1->rotationMatrix * glm::inverse(c1->inertiaMatrix) * glm::transpose(c1->rotationMatrix);
	c1->angularVelocity = c1->inertiaMatrix * c1->angularMomentum;
	c1->linearMomentum += event.dt * c1->accleration;

	c1->angularVelocity = c1->inertiaMatrix * c1->angularMomentum;



	glm::vec3 lowestPoint = findLowestPointRotated(c1);

	if (c1->collisionWithOtherObject) {
		auto temp = c1->nextRotation * c1->mtv + c1->nextPosition;
		calculateCollisionRespones(c1, temp - c1->cube->getPosition());
	}

	if (lowestPoint.y < 0.5 && !c1->collisionDone) {

		float distance = 0.5 - lowestPoint.y;
		c1->nextPosition.y = c1->nextPosition.y + distance;

		lowestPoint = c1->nextRotation * findLowestPoint(c1) + c1->nextPosition;

		glm::vec3 contactPointVector = lowestPoint - c1->cube->getPosition();

		calculateCollisionRespones(c1, contactPointVector);

		if (checkCollisionDone(c1, lowestPoint, event)) {
			c1->angularMomentum = glm::vec3(0);
			c1->angularVelocity = glm::vec3(0);
			c1->linearMomentum = glm::vec3(0);
			c1->collisionDone = true;
			c1->cube->multiplyTransform(glm::translate(glm::mat4(1), glm::vec3(0, 1 - c1->cube->getPosition().y, 0)));
		}

	}
	else if (!c1->collisionDone) {
		c1->rotationMatrix = c1->nextRotation;
		c1->cube->setTransform(c1->rotationMatrix);
		c1->cube->multiplyTransform(glm::translate(glm::mat4(1.0), c1->nextPosition));
	}
}

glm::mat3 getSkewMatrix(Cube *c1) {
	glm::mat3 result;
	result[0][0] = 0.0;
	result[0][1] = -c1->angularVelocity.z;
	result[0][2] = +c1->angularVelocity.y;
	result[1][0] = +c1->angularVelocity.z;
	result[1][1] = 0.0;
	result[1][2] = -c1->angularVelocity.x;
	result[2][0] = -c1->angularVelocity.y;
	result[2][1] = +c1->angularVelocity.x;
	result[2][2] = 0.0;
	return result;
}

//Before we start we need a support mapping function.Depending on the direction we can calculate any point inside the cube. (min = -0.5, max = 0.5)


glm::vec3 support(Cube *c1, glm::vec3 dir) {

	dir = glm::inverse(c1->rotationMatrix) * dir;

	//https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
	auto maxx = (glm::length(glm::vec3(c1->cube->getTransform()[0][0], c1->cube->getTransform()[1][0], c1->cube->getTransform()[2][0]))) / 2 ;
	auto maxy = (glm::length(glm::vec3(c1->cube->getTransform()[1][0], c1->cube->getTransform()[1][1], c1->cube->getTransform()[2][1]))) / 2 ;
	auto maxz = (glm::length(glm::vec3(c1->cube->getTransform()[2][0], c1->cube->getTransform()[2][1], c1->cube->getTransform()[2][2]))) / 2 ;

	if (c1->cubeType == "Wall") {
		maxx = glm::abs(static_cast<WallCube*>(c1)->maxRotated.x/2); 
		maxy = glm::abs(static_cast<WallCube*>(c1)->maxRotated.y/2);
		maxz = glm::abs(static_cast<WallCube*>(c1)->maxRotated.z/2);
	}

	auto minx = -1 * maxx;
	auto miny = -1 * maxy;
	auto minz = -1 * maxz;


	glm::vec3 support;

	support.x = (dir.x > 0) ? maxx : minx;

	support.y = (dir.y > 0) ? maxy : miny;

	support.z = (dir.z > 0) ? maxz : minz;

	//std::cout << glm::to_string(support) << std::endl;


	return c1->rotationMatrix * support + c1->cube->getPosition();
}

bool gjk(Cube *c1, Cube *c2) {
	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 d;
	// get the start direction of simplex search by calculating c1 center - c2 center vector
	glm::vec3 search_dir = c1->cube->getPosition() - c2->cube->getPosition();
	//support function calculates first point of the simplex 
	c = support(c2, search_dir) - support(c1, -search_dir);

	// search in the origins direction 
	search_dir = -c;

	// to get second simplex point once again use the support function 
	b = support(c2, search_dir) - support(c1, -search_dir);

	//first check if collision is possible, if origin is not reached the gjk won't eclose it
	if (dot(b, search_dir) < 0) { return false; }

	auto bc = c - b;

	search_dir = cross(cross(bc, -b), bc);

	if (search_dir == glm::vec3(0, 0, 0)) {
		//origin is on this line segment
		search_dir = glm::cross(bc, glm::vec3(1, 0, 0));
		//normal with x-axis therefore (1,0,0)
		if (search_dir == glm::vec3(0, 0, 0)) {
			search_dir = glm::cross(bc, glm::vec3(0, 0, 1));
		}//normal with z-axis (therefore (0,0,-1)
	}

	// how many steps of the simplex were done 
	int simp_dim = 2;

	//define depth of iterations of the simplex 
	for (int iterations = 0; iterations < 32; iterations++)
	{
		auto a = support(c2, search_dir) - support(c1, -search_dir);
		//first check if collision is possible, if origin is not reached the gjk won't eclose it
		if (dot(a, search_dir) < 0) { return false; }

		simp_dim++;

		if (simp_dim == 3) {
			update_simplex3(a, b, c, d, simp_dim, search_dir);
		}
		else if (update_simplex4(a, b, c, d, simp_dim, search_dir)) {
			//if (mtv) *mtv = EPA(a, b, c, d, c1, c2);
			return true;
		}
	}//endfor
	return false;
}

//Triangle case
void update_simplex3(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir) {
	/* Required winding order:
	//  b
	//  | \
	//  |   \
	//  |    a
	//  |   /
	//  | /
	//  c
	*/
	glm::vec3 n = cross(b - a, c - a); //triangle's normal

	//Search closest to the origin make it new simplex 

	simp_dim = 2;

	auto ab = b - a;
	auto ac = c - a;

	if (glm::dot(glm::cross(ab, n), -a) > 0) { //Closest to edge AB
		c = a;
		search_dir = glm::cross(glm::cross(ab, -a), ab);
		return;
	}
	else if (glm::dot(glm::cross(n, ac), -a) > 0) { //Closest to edge AC
		b = a;
		search_dir = glm::cross(glm::cross(ac, -a), ac);
		return;
	}

	simp_dim = 3;
	//Search above triangle, making search direction the triangles normal 
	if (glm::dot(n, -a) > 0) {
		d = c;
		c = b;
		b = a;
		search_dir = n;
		return;
	}
	else {
		//Search below triangle, making search direction the negated triangles normal 
		d = b;
		b = a;
		search_dir = -n;
		return;
	}
	return;
}

//Tetrahedral case
bool update_simplex4(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& d, int& simp_dim, glm::vec3& search_dir) {
	//BCD is the base of the Tetrahedron and a is the top corner 
	//Origin must be between BCD and a

	//Calculate normals of all 3 faces
	glm::vec3 ABC = cross(b - a, c - a);
	glm::vec3 ACD = cross(c - a, d - a);
	glm::vec3 ADB = cross(d - a, b - a);

	simp_dim = 3; //hoisting this just cause

	//In front of ABC
	if (glm::dot(ABC, -a) > 0) {
		d = c;
		c = b;
		b = a;
		search_dir = ABC;
		return false;
	}
	//In front of ACD
	if (glm::dot(ACD, -a) > 0) {
		b = a;
		search_dir = ACD;
		return false;
	}
	//In front of ADB
	if (glm::dot(ADB, -a) > 0) {
		c = d;
		d = b;
		b = a;
		search_dir = ADB;
		return false;
	}

	//else it's enclosed inside the Tetrahedron
	return true;
}

