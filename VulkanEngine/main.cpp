/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"
#include "CollisionHandling.h"
#include "PlayerCube.h"
#include "WallCube.h"
#include "BulletCube.h"
#include "EnemyCube.h"
#include "CollisionCubePhysics.h"

namespace ve {

	uint32_t g_score = 0;				//derzeitiger Punktestand
	bool g_gameLost = false;			//true... das Spiel wurde verloren

	class EventListenerGUI : public VEEventListener {
	protected:

		virtual void onDrawOverlay(veEvent event) {
			VESubrenderFW_Nuklear* pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
			if (pSubrender == nullptr) return;

			struct nk_context* ctx = pSubrender->getContext();

			if (!g_gameLost) {
				if (nk_begin(ctx, "", nk_rect(0, 0, 200, 170), NK_WINDOW_BORDER)) {
					char outbuffer[100];
					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Score: %03d", g_score);
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);
				}
			}
			else {
				if (nk_begin(ctx, "", nk_rect(500, 500, 200, 170), NK_WINDOW_BORDER)) {
					nk_layout_row_dynamic(ctx, 45, 1);
					nk_label(ctx, "Game Over", NK_TEXT_LEFT);
				}

			};

			nk_end(ctx);
		}

	public:
		///Constructor of class EventListenerGUI
		EventListenerGUI(std::string name) : VEEventListener(name) { };

		///Destructor of class EventListenerGUI
		virtual ~EventListenerGUI() {};
	};

	void ScaleRotateTranslateObject(VESceneNode* object, glm::vec3 scale = glm::vec3(1, 1, 1), float rotationvalue = 0.0f, glm::vec3 rotationaxis = glm::vec3(0, 0, 0), glm::vec3 translation = glm::vec3(0, 0, 0)) {
		object->multiplyTransform(glm::scale(glm::mat4(1.0f), scale));
		object->multiplyTransform(glm::rotate(glm::mat4(1.0f), rotationvalue, rotationaxis));
		object->multiplyTransform(glm::translate(glm::mat4(1.0f), translation));
	}

	void RotateObject(VESceneNode* object, float rotationvalue = 0.0f, glm::vec3 rotationaxis = glm::vec3(0, 0, 0)) {
		glm::vec3 temptrans = object->getPosition();
		object->multiplyTransform(glm::translate(glm::mat4(1.0f), (-temptrans)));
		object->multiplyTransform(glm::rotate(glm::mat4(1.0f), rotationvalue, rotationaxis));
		object->multiplyTransform(glm::translate(glm::mat4(1.0f), temptrans));
	}


	void ScaleObject(VESceneNode* object, glm::vec3 scale = glm::vec3(1, 1, 1)) {
		glm::vec3 temptrans = object->getPosition();
		object->multiplyTransform(glm::translate(glm::mat4(1.0f), (-temptrans)));
		object->multiplyTransform(glm::scale(glm::mat4(1.0f), scale));
		object->multiplyTransform(glm::translate(glm::mat4(1.0f), temptrans));
	}

	void TranslateObject(VESceneNode* object, glm::vec3 translation = glm::vec3(0, 0, 0)) {
		object->multiplyTransform(glm::translate(glm::mat4(1.0f), translation));
	}

	int distance2points2Dxz(glm::vec2 Point1, glm::vec2 Point2) {

		return glm::distance(Point1, Point2);
	}

	class PlayerListener : public VEEventListener {
		VESceneNode* player;
		VESceneNode* pScene;
		VECamera* camera;
		PlayerCube* playerCube;
		glm::vec3 shootingdirection;
		bool bulletFired = false;
		int bulletcount = 0; 
		glm::vec3 lastlastPos;
		glm::vec3 lastPos;
		bool lastPosInit = false; 
	public:
		PlayerListener(std::string name, VESceneNode* player, VESceneNode* pScene, VECamera* camera, PlayerCube* playerCube) : VEEventListener(name), player(player), pScene(pScene), camera(camera), playerCube(playerCube) {
		};


		virtual bool onKeyboard(veEvent event) {
			float angle = 0;
			auto rot4 = glm::vec4();
			glm::vec3 rot3;
			glm::mat4 rotate;
			float movementSpeed = playerCube->movementSpeed;
			if (event.idata3 == GLFW_RELEASE) return false;

			if (lastPosInit == true) {
				lastlastPos = lastPos;
			}
			lastPos = player->getPosition(); 
			lastPosInit = true;

			switch (event.idata1) {
			case (GLFW_KEY_W):
				player->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(0, 0, movementSpeed)));
				//camera->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(0, 0, movementSpeed)));
				break;

			case(GLFW_KEY_A):
				player->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(-movementSpeed, 0, 0.0)));
				//camera->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(-movementSpeed, 0, 0.0)));
				break;

			case(GLFW_KEY_S):
				player->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -movementSpeed)));
				//camera->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -movementSpeed)));
				break;

			case(GLFW_KEY_D):
				player->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(movementSpeed, 0, 0.0)));
				//camera->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(movementSpeed, 0, 0.0)));
				break;

			case (GLFW_KEY_M):							//yaw rotation is already in parent space
				angle = (float)event.dt * 4.0f;
				rot3 = glm::vec3(0, 1, 0);
				rotate = glm::rotate(glm::mat4(1.0), angle, rot3);
				glm::vec3 backuptransform = player->getPosition();
				player->multiplyTransform(glm::translate(glm::mat4(1.0f), (backuptransform * glm::vec3(-1, -1, -1))));
				player->multiplyTransform(rotate);
				player->multiplyTransform(glm::translate(glm::mat4(1.0f), backuptransform));
				break;

			case (GLFW_KEY_N):						//yaw rotation is already in parent space
				angle = -(float)event.dt * 4.0f;
				rot3 = glm::vec3(0, 1, 0);
				rotate = glm::rotate(glm::mat4(1.0), angle, rot3);
				backuptransform = player->getPosition();
				player->multiplyTransform(glm::translate(glm::mat4(1.0f), (backuptransform * glm::vec3(-1, -1, -1))));
				player->multiplyTransform(rotate);
				player->multiplyTransform(glm::translate(glm::mat4(1.0f), backuptransform));
				break;
			default:
				break;
			}

			if (event.idata1 == GLFW_KEY_Q && event.idata3 == GLFW_PRESS) {
				if (bulletFired == false) {
					//bulletFired = true;
					shootingdirection = player->getXAxis();
					//std::cout << playerCube->availableBullets.size() << " " << bulletcount << std::endl;
					BulletCube* currentBullet = static_cast<BulletCube*>(playerCube->availableBullets.at(bulletcount));
					currentBullet->shootDirection = glm::normalize(shootingdirection);
					currentBullet->bulletFired = true;
					bulletcount++;
					if (bulletcount == playerCube->availableBullets.size()) {
						bulletcount = 0;
					}
					currentBullet->cube->setTransform(playerCube->cube->getWorldTransform());
					currentBullet->collisionCount = 0; 
					bulletFired = false;
					//std::cout << glm::to_string(shootingdirection) << std::endl;
				}
			}

			if (lastPosInit == false) {
				playerCube->movementDirection = player->getPosition() - lastPos;
			}
			else{
				playerCube->movementDirection = glm::normalize(player->getPosition() - lastPos + player->getPosition() - lastlastPos);
			}

			//std::cout << glm::to_string(playerCube->movementDirection) << std::endl;

			return false;
		}

		void onFrameStarted(veEvent event) {
			camera->setPosition(glm::vec3(player->getPosition().x, 20, player->getPosition().z-22));
		}

	};

	class BulletListener : public VEEventListener {
		VESceneNode* Bullet;
		BulletCube* bulletCube = nullptr;
		glm::vec3 shootingDirection;
		float shootSpeed = 0.2; 
		glm::mat3 rotationMatrix;
		PlayerCube* player;

	public:
		BulletListener(std::string name, VESceneNode* Bullet, glm::mat3 rotationMatrix, PlayerCube* Player, BulletCube* bulletCube, bool supportbullet) : VEEventListener(name), rotationMatrix(rotationMatrix), Bullet(Bullet), player(Player), bulletCube(bulletCube){
			if (!supportbullet) {
				player->availableBullets.push_back(bulletCube);
			}
		}

		void onFrameStarted(veEvent event) {
			if (bulletCube->bulletFired == true) {
				bulletCube->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), bulletCube->shootDirection * shootSpeed));
			//	std::cout << "Bullet Fired " << std::endl;
			}
		}
	};


	class LivesListener : public VEEventListener {
		PlayerCube* playerCube;
		float currentspeed = 0.15;
		std::vector<Cube*> activeLives; 
		VESceneNode* pScene;
	public:
		LivesListener(std::string name, PlayerCube* playerCube, VESceneNode* pScene) : VEEventListener(name), playerCube(playerCube), pScene(pScene) {
			for (int i = 0; i < 3; i++) {
				VESceneNode* live;
				VECHECKPOINTER(live = getSceneManagerPointer()->loadModel("live" + std::to_string(i), "media/models/test/crate0", "cube.obj"));
				live->multiplyTransform(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)));
				pScene->addChild(live);
				ScaleObject(live, glm::vec3(0.8-(i*0.1), 1, 0.5-(i*0.1)));
				Cube* liveCube = new Cube("live" + std::to_string(i), glm::mat3(1.0f), live, "Live");
				activeLives.push_back(liveCube);
			}
		};

		void onFrameStarted(veEvent event) {
			if(!activeLives.empty()){
				if (playerCube->liveLost) {
					getSceneManagerPointer()->deleteSceneNodeAndChildren("live" + std::to_string(activeLives.size()-1));
					activeLives.resize(activeLives.size() - 1);
					if (activeLives.empty()) {
						g_gameLost = true;
					}
					playerCube->liveLost = false;
				}
			for (int i = 0; i < activeLives.size(); i++) {
				if (i == 0) {
					if (glm::distance(playerCube->cube->getPosition(), activeLives.at(i)->cube->getPosition()) > 1) {
						glm::vec3 followVector = playerCube->cube->getPosition() - activeLives.at(i)->cube->getPosition();
						//std::cout << glm::to_string(followVector) << std::endl;
						activeLives.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), followVector * currentspeed));
					}
				}
				else {
					if (glm::distance(activeLives.at(i - 1)->cube->getPosition(), activeLives.at(i)->cube->getPosition()) > 0.75) {
						glm::vec3 followVector = activeLives.at(i - 1)->cube->getPosition() - activeLives.at(i)->cube->getPosition();
						//std::cout << glm::to_string(followVector) << std::endl;
						activeLives.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), followVector * currentspeed));
					}
				}
				}
			}

		}

	};

	class CollisionListener : public VEEventListener {
		std::vector<Cube*> *collidWithVec;
		int spawnSupportCubes = false; 
		BulletCube* explodingCube = nullptr;
		bool canActivateSupport = true; 
		float timepassed = 0; 
	public: 
		CollisionListener(std::string name, std::vector<Cube*> *collidWithVec) : VEEventListener(name), collidWithVec(collidWithVec) {

		}

		void onFrameStarted(veEvent event) {
			for (int i = 0; i < collidWithVec->size(); i++) {
				for (int j = i + 1; j < collidWithVec->size(); j++) {
					if (gjk(collidWithVec->at(j), collidWithVec->at(i))) {
						if (collidWithVec->at(j)->cubeType == "Bullet" && collidWithVec->at(i)->cubeType == "Bullet"
							&& !static_cast<BulletCube*>(collidWithVec->at(i))->supportBullet && !static_cast<BulletCube*>(collidWithVec->at(j))->supportBullet) {
							//std::cout << "Bullets Colliding " << std::endl;							
							//Spawn supporting cubes to blast in 4 directions
							if (canActivateSupport) {
								spawnSupportCubes = true;
								explodingCube = static_cast<BulletCube*>(collidWithVec->at(i));
								explodingCube->supportBulletsActiveCount = 4;
								if (spawnSupportCubes == true && explodingCube->supportBulletsActiveCount && canActivateSupport == true) {
									canActivateSupport = false;
									for (int i = 0; i < explodingCube->supportCubes.size(); i++) {
										//std::cout << "support activated " << std::endl;
										explodingCube->supportCubes.at(i)->cube->setTransform(explodingCube->cube->getWorldTransform());
										if (i == 0) {
											explodingCube->supportCubes.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(1, 0, 0)));
											ScaleObject(static_cast<BulletCube*>(explodingCube)->supportCubes.at(i)->cube, glm::vec3(0.5, 0.5, 0.5));
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootDirection = glm::vec3(1, 0, 0);
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootSpeed = 0.2;
											collidWithVec->push_back(static_cast<BulletCube*>(explodingCube->supportCubes.at(i)));
										}
										if (i == 1) {
											explodingCube->supportCubes.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1, 0, 0)));
											ScaleObject(explodingCube->supportCubes.at(i)->cube, glm::vec3(0.5, 0.5, 0.5));
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootDirection = glm::vec3(-1, 0, 0);
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootSpeed = 0.2;
											collidWithVec->push_back(static_cast<BulletCube*>(explodingCube->supportCubes.at(i)));
										}
										if (i == 2) {
											explodingCube->supportCubes.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1)));
											ScaleObject(static_cast<BulletCube*>(explodingCube)->supportCubes.at(i)->cube, glm::vec3(0.5, 0.5, 0.5));
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootDirection = glm::vec3(0, 0, 1);
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootSpeed = 0.2;
											collidWithVec->push_back(static_cast<BulletCube*>(explodingCube->supportCubes.at(i)));
										}
										if (i == 3) {
											explodingCube->supportCubes.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1)));
											ScaleObject(static_cast<BulletCube*>(explodingCube)->supportCubes.at(i)->cube, glm::vec3(0.5, 0.5, 0.5));
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootDirection = glm::vec3(0, 0, -1);
											static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootSpeed = 0.2;
											collidWithVec->push_back(static_cast<BulletCube*>(explodingCube->supportCubes.at(i)));
										}
										//std::cout << glm::to_string(static_cast<BulletCube*>(explodingCube)->supportCubes.at(0)->cube->getPosition()) << std::endl;
									}
									spawnSupportCubes = false;
								}
							}
							collidWithVec->at(j)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(j))->spawnPosition);
							collidWithVec->at(j)->collisionCount = 0;
							collidWithVec->at(i)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(i))->spawnPosition);
							collidWithVec->at(i)->collisionCount = 0;
						}
						if (collidWithVec->at(i)->cubeType == "Enemy" && collidWithVec->at(j)->cubeType == "Bullet") {
							//std::cout << "Bullet Enemy Collision " << std::endl;
							if (!static_cast<BulletCube*>(collidWithVec->at(j))->supportBullet) {
								if (collidWithVec->at(j)->collisionCount >= 1) {
									collidWithVec->at(i)->collisionCount++;
									g_score++;
								}
							}
							else {
								collidWithVec->at(i)->collisionCount++;
							}
						}
						if (collidWithVec->at(j)->cubeType == "Enemy" && collidWithVec->at(i)->cubeType == "Bullet") {
							//std::cout << "Bullet Enemy Collision " << std::endl;
							if (!static_cast<BulletCube*>(collidWithVec->at(i))->supportBullet) {
								if (collidWithVec->at(i)->collisionCount >= 1) {
									collidWithVec->at(j)->collisionCount++;
									g_score++;
								}
							}
							else {
								collidWithVec->at(j)->collisionCount++;
							}
						}

						if (collidWithVec->at(j)->cubeType == "Enemy" && collidWithVec->at(i)->cubeType == "Player") {
							//std::cout << "Enemy Player Collision " << std::endl;
							collidWithVec->at(i)->collisionCount++;
							collidWithVec->at(j)->collisionCount++;
							//std::cout << "Player Collision Count " << collidWithVec->at(i)->collisionCount << std::endl;
							static_cast<PlayerCube*>(collidWithVec->at(i))->liveLost = true;
						}
						if (collidWithVec->at(i)->cubeType == "Enemy" && collidWithVec->at(j)->cubeType == "Player") {
							//std::cout << "Enemy Player Collision " << std::endl;
							collidWithVec->at(j)->collisionCount++;
							collidWithVec->at(i)->collisionCount++;
							//std::cout << "Player Collision Count " << collidWithVec->at(i)->collisionCount << std::endl;
							static_cast<PlayerCube*>(collidWithVec->at(j))->liveLost = true;
						}
						/*
						if (collidWithVec->at(i)->cubeType == "EnemyBullet" && collidWithVec->at(j)->cubeType == "Player") {
							//std::cout << "Enemy Player Collision " << std::endl;
							collidWithVec->at(j)->collisionCount++;
							collidWithVec->at(i)->collisionCount++;
							if (collidWithVec->at(i)->collisionCount >= 1) {
								collidWithVec->at(i)->collisionCount = 0;
								float randxdir = ((rand() % 20 + 1) - 10) / 10;
								float randzdir = ((rand() % 20 + 1) - 10) / 10;
								collidWithVec->at(j)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(j))->spawnPosition);
								//static_cast<BulletCube*>(collidWithVec->at(j))->shootDirection = glm::vec3(randxdir, 0, randzdir);
								//static_cast<BulletCube*>(collidWithVec->at(i))->shootDirection = glm::normalize(glm::vec3(randxdir, 0, randzdir));
							
								//std::cout << " Colliding End " << collidWithVec->at(i)->name << std::endl;
							}
							//std::cout << "Player Collision Count " << collidWithVec->at(i)->collisionCount << std::endl;
							static_cast<PlayerCube*>(collidWithVec->at(j))->liveLost = true;
						}
						if (collidWithVec->at(j)->cubeType == "EnemyBullet" && collidWithVec->at(i)->cubeType == "Player") {
							//std::cout << "Enemy Player Collision " << std::endl;
							collidWithVec->at(j)->collisionCount++;
							collidWithVec->at(i)->collisionCount++;
							if (collidWithVec->at(j)->collisionCount >= 1) {
								collidWithVec->at(j)->collisionCount = 0;
								float randxdir = ((rand() % 20 + 1) - 10) / 10.0;
								float randzdir = ((rand() % 20 + 1) - 10) / 10.0;
								collidWithVec->at(j)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(j))->spawnPosition);
								std::cout << glm::to_string(static_cast<BulletCube*>(collidWithVec->at(j))->spawnPosition) << std::endl;
								//static_cast<BulletCube*>(collidWithVec->at(j))->shootDirection = glm::vec3(randxdir, 0, randzdir);								
								//static_cast<BulletCube*>(collidWithVec->at(j))->shootDirection = glm::normalize(glm::vec3(randxdir, 0, randzdir));
								//std::cout << " Colliding End " << collidWithVec->at(i)->name << std::endl;
							}
							//std::cout << "Player Collision Count " << collidWithVec->at(i)->collisionCount << std::endl;
							static_cast<PlayerCube*>(collidWithVec->at(i))->liveLost = true;
						}
						*/
					}
				}
			}
			if (explodingCube != nullptr && explodingCube->supportBulletsActiveCount) {
				for (int i = 0; i < explodingCube->supportCubes.size(); i++) {
					static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootDirection* static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->shootSpeed));
					timepassed += event.dt; 
					//std::cout << timepassed << std::endl;
					if (static_cast<BulletCube*>(explodingCube->supportCubes.at(i))->collisionCount == 2 || timepassed > 35.0f) {
						explodingCube->supportCubes.at(i)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(i))->spawnPosition);
						timepassed = 0; 
						for (int count = 0; count < collidWithVec->size(); count++) {
							if (collidWithVec->at(count) == explodingCube->supportCubes.at(i)) {
								explodingCube->supportCubes.at(i)->collisionCount = 0; 
								collidWithVec->erase(collidWithVec->begin() + count);
								explodingCube->supportBulletsActiveCount--;
							}
						}
					}
				}
				if (explodingCube->supportBulletsActiveCount == 0) {
					canActivateSupport = true;
				}
			}
			
		}
	};

	class WallListener : public VEEventListener {
		VESceneNode* Wall;
		std::vector<Cube*> *collidWithVec;
		WallCube* wallCube = nullptr;
		glm::mat3 rotationMatrix;
		glm::vec3 maxRotated;
		std::vector<Cube*>* wallVec;
	public:
		WallListener(std::string name, VESceneNode* Wall, glm::mat3 rotationMatrix, std::vector<Cube*> *collidWithVec, std::vector<Cube*>* wallVec, glm::vec3 maxRotated, bool supportWall) : VEEventListener(name), Wall(Wall), rotationMatrix(rotationMatrix), collidWithVec(collidWithVec), wallVec(wallVec), maxRotated(maxRotated) {
			wallCube = new WallCube(name, rotationMatrix, Wall, "Wall", maxRotated, supportWall);

			auto maxvec = Wall->getPosition() + glm::abs(static_cast<WallCube*>(wallCube)->maxRotated / 2)* rotationMatrix ;

			auto minvec = Wall->getPosition() - glm::abs(static_cast<WallCube*>(wallCube)->maxRotated / 2)* rotationMatrix;

			float edge1X = 0; 
			float edge2X = 0; 
			float edge1Z = 0;
			float edge2Z = 0;

			//left down 
if (Wall->getPosition().x <= 0 && Wall->getPosition().z <= 0) {
	if (maxvec.x <= Wall->getPosition().x) {
		edge1X = maxvec.x;
		edge2X = minvec.x;
	}
	else {
		edge1X = minvec.x;
		edge2X = maxvec.x;
	}
	if (maxvec.z >= Wall->getPosition().z) {
		edge1Z = maxvec.z;
		edge2Z = minvec.z;
	}
	else {
		edge1Z = minvec.z;
		edge2Z = maxvec.z;
	}

	glm::vec2 vecE1E2 = glm::vec2(edge1X, edge1Z) - glm::vec2(edge2X, edge2Z);

	wallCube->wallNorm = glm::normalize(glm::vec3(glm::abs(vecE1E2.y), 0, glm::abs(vecE1E2.x)));
}

//left up 
if (Wall->getPosition().x <= 0 && Wall->getPosition().z >= 0) {
	if (maxvec.x <= Wall->getPosition().x) {
		edge1X = maxvec.x;
		edge2X = minvec.x;
	}
	else {
		edge1X = minvec.x;
		edge2X = maxvec.x;
	}
	if (maxvec.z <= Wall->getPosition().z) {
		edge1Z = maxvec.z;
		edge2Z = minvec.z;
	}
	else {
		edge1Z = minvec.z;
		edge2Z = maxvec.z;
	}
	glm::vec2 vecE1E2 = glm::vec2(edge1X, edge1Z) - glm::vec2(edge2X, edge2Z);

	wallCube->wallNorm = glm::normalize(glm::vec3(glm::abs(vecE1E2.y), 0, -1*glm::abs(vecE1E2.x)));
}

// right down
if (Wall->getPosition().x > 0 && Wall->getPosition().z <= 0) {
	if (maxvec.x >= Wall->getPosition().x) {
		edge1X = maxvec.x;
		edge2X = minvec.x;
	}
	else {
		edge1X = minvec.x;
		edge2X = maxvec.x;
	}
	if (maxvec.z >= Wall->getPosition().z) {
		edge1Z = maxvec.z;
		edge2Z = minvec.z;
	}
	else {
		edge1Z = minvec.z;
		edge2Z = maxvec.z;
	}
	glm::vec2 vecE1E2 = glm::vec2(edge1X, edge1Z) - glm::vec2(edge2X, edge2Z);

	wallCube->wallNorm = glm::normalize(glm::vec3(-1*glm::abs(vecE1E2.y), 0, glm::abs(vecE1E2.x)));
}

//right up
if (Wall->getPosition().x > 0 && Wall->getPosition().z >= 0) {
	if (maxvec.x >= Wall->getPosition().x) {
		edge1X = maxvec.x-1;
		edge2X = minvec.x;
	}
	else {
		edge1X = minvec.x;
		edge2X = maxvec.x-1;
	}
	if (maxvec.z <= Wall->getPosition().z) {
		edge1Z = maxvec.z-1;
		edge2Z = minvec.z;
	}
	else {
		edge1Z = minvec.z;
		edge2Z = maxvec.z-1;
	}
	glm::vec2 vecE1E2 = glm::vec2(edge1X, edge1Z) - glm::vec2(edge2X, edge2Z);

	wallCube->wallNorm = glm::normalize(glm::vec3(-1*glm::abs(vecE1E2.y), 0, -1*glm::abs(vecE1E2.x)));

}

wallVec->push_back(wallCube);


//std::cout << name << "\n Edge1Coord x " << edge1X << " Edge1Coord z " << edge1Z << " Edge2Coord x " << edge2X << " Edge2Coord z " << edge2Z << std::endl << std::endl;

glm::vec2 vecE1E2 = glm::vec2(edge1X, edge1Z) - glm::vec2(edge2X, edge2Z);

wallCube->wallNorm = glm::normalize(glm::vec3(vecE1E2.y, 0, -vecE1E2.x));

//std::cout << name << " \n" << glm::to_string(wallCube->wallNorm) << std::endl << std::endl;

		}

		void onFrameStarted(veEvent event) {
			for (int i = 0; i < collidWithVec->size(); i++) {
				//check collision Object with Wall
				if (gjk(wallCube, collidWithVec->at(i))) {
					//std::cout << "COLLLLL" << std::endl;
					//glm::vec3 normalVecPrep = wallCube->cube->getPosition() - collidWithVec.at(i)->cube->getPosition();
				//determins from which direction the cube comes from; 
					
					if (wallCube->cube->getPosition().x <= 0 && wallCube->cube->getPosition().z <= 0) {
						wallCube->wallNorm = glm::vec3(glm::abs(wallCube->wallNorm.x), 0, glm::abs(wallCube->wallNorm.z));
						//std::cout << "first if " << glm::to_string(wallCube->wallNorm) << std::endl;
					}
					else if (wallCube->cube->getPosition().x <= 0 && wallCube->cube->getPosition().z >= 0) {
						wallCube->wallNorm = glm::vec3(glm::abs(wallCube->wallNorm.x), 0, -1 * glm::abs(wallCube->wallNorm.z));
						//std::cout << "second if " << glm::to_string(wallCube->wallNorm) << std::endl;
					}
					else if (wallCube->cube->getPosition().x > 0 && wallCube->cube->getPosition().z <= 0) {
						wallCube->wallNorm = glm::vec3(-1 * glm::abs(wallCube->wallNorm.x), 0, glm::abs(wallCube->wallNorm.z));
						//std::cout << "third if " << glm::to_string(wallCube->wallNorm) << std::endl;
					}
					else if (wallCube->cube->getPosition().x > 0 && wallCube->cube->getPosition().z >= 0) {
						wallCube->wallNorm = glm::vec3(-1 * glm::abs(wallCube->wallNorm.x), 0, -1 * glm::abs(wallCube->wallNorm.z));
						//std::cout << "fourth if " << glm::to_string(wallCube->wallNorm) << std::endl;
					}
					

					//collidWithVec.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0), wallCube->wallNorm));
					if (collidWithVec->at(i)->cubeType == "Bullet") {
						if (collidWithVec->at(i)->collisionCount >= 3) {
							collidWithVec->at(i)->collisionCount = 0;
							collidWithVec->at(i)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(i))->spawnPosition);
							//std::cout << " Colliding End " << collidWithVec->at(i)->name << std::endl;
						}
						else {
							//std::cout << "Bullet Wall Collisision" << std::endl;
							auto collidingbulletCube = static_cast<BulletCube*>(collidWithVec->at(i));
							//calculate bouncing angle for bullet with wall normal
							auto angleWallBullet = glm::angle(wallCube->wallNorm, -1 * collidingbulletCube->shootDirection);
							//std::cout << "angle between wall norm and bullet: " << glm::degrees(angleWallBullet) << std::endl;
							//std::cout << "Shootdirection : " << glm::to_string(collidingbulletCube->shootDirection) << std::endl;
							auto newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), angleWallBullet);
							//std::cout << "New Direction : " << glm::to_string(newDirection*-1) << std::endl;

							//std::cout << "New Direction *-1 : " << roundf(newDirection.x*-1000)/1000 << "," << roundf(newDirection.y * -1000) / 1000 << " bullet direction " << roundf(collidingbulletCube->shootDirection.x * 1000) / 1000 << "," << roundf(collidingbulletCube->shootDirection.z * 1000) / 1000 << std::endl;

							glm::vec2 roundedNewDir = glm::vec2(roundf(newDirection.x * -1000) / 1000, roundf(newDirection.y * -1000) / 1000);
							glm::vec2 roundedShootDirection = glm::vec2(roundf(collidingbulletCube->shootDirection.x * 1000) / 1000, roundf(collidingbulletCube->shootDirection.z * 1000) / 1000);

							if (roundedNewDir.x == roundedShootDirection.x && roundedNewDir.y == roundedShootDirection.y) {
								//std::cout << "Reversing shoot direction " << std::endl;
								newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), -1 * angleWallBullet);
							}
							collidingbulletCube->shootDirection = glm::vec3(newDirection.x, 0, newDirection.y);

							//collidingbulletCube->shootDirection = wallCube->wallNorm;
							collidWithVec->at(i)->collisionCount++;
						}
					}
					/*
					if (collidWithVec->at(i)->cubeType == "EnemyBullet") {
						if (collidWithVec->at(i)->collisionCount >= 1) {
							collidWithVec->at(i)->collisionCount = 0;
							collidWithVec->at(i)->cube->setPosition(static_cast<BulletCube*>(collidWithVec->at(i))->spawnPosition);
							//std::cout << " Colliding End " << collidWithVec->at(i)->name << std::endl;
						}
						else {
							//std::cout << collidWithVec->at(i)->collisionCount << " EnemyBullet " << collidWithVec->at(i)->name << std::endl;

							//std::cout << "Bullet Wall Collisision" << std::endl;
							auto collidingbulletCube = static_cast<BulletCube*>(collidWithVec->at(i));
							//calculate bouncing angle for bullet with wall normal
							auto angleWallBullet = glm::angle(wallCube->wallNorm, -1 * collidingbulletCube->shootDirection);
							//std::cout << "angle between wall norm and bullet: " << glm::degrees(angleWallBullet) << std::endl;
							//std::cout << "Shootdirection : " << glm::to_string(collidingbulletCube->shootDirection) << std::endl;
							auto newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), angleWallBullet);
							//std::cout << "New Direction : " << glm::to_string(newDirection*-1) << std::endl;

							//std::cout << "New Direction *-1 : " << roundf(newDirection.x*-1000)/1000 << "," << roundf(newDirection.y * -1000) / 1000 << " bullet direction " << roundf(collidingbulletCube->shootDirection.x * 1000) / 1000 << "," << roundf(collidingbulletCube->shootDirection.z * 1000) / 1000 << std::endl;

							glm::vec2 roundedNewDir = glm::vec2(roundf(newDirection.x * -1000) / 1000, roundf(newDirection.y * -1000) / 1000);
							glm::vec2 roundedShootDirection = glm::vec2(roundf(collidingbulletCube->shootDirection.x * 1000) / 1000, roundf(collidingbulletCube->shootDirection.z * 1000) / 1000);

							if (roundedNewDir.x == roundedShootDirection.x && roundedNewDir.y == roundedShootDirection.y) {
								//std::cout << "Reversing shoot direction " << std::endl;
								newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), -1 * angleWallBullet);
							}
							collidingbulletCube->shootDirection = glm::vec3(newDirection.x, 0, newDirection.y);

							//collidingbulletCube->shootDirection = wallCube->wallNorm;
							collidWithVec->at(i)->collisionCount++;
						}
					}
					*/
					if (collidWithVec->at(i)->cubeType == "Player") {
						//auto collidingbulletCube = static_cast<PlayerCube*>(collidWithVec->at(i));
						//collidingbulletCube->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), wallCube->wallNorm));

						//std::cout << "Bullet Wall Collisision" << std::endl;
						auto collidingPlayer = static_cast<PlayerCube*>(collidWithVec->at(i));
						//calculate bouncing angle for bullet with wall normal
						auto angleWallBullet = glm::angle(wallCube->wallNorm, -1 * collidingPlayer->movementDirection);
						//std::cout << "angle between wall norm and bullet: " << glm::degrees(angleWallBullet) << std::endl;
						//std::cout << "Shootdirection : " << glm::to_string(collidingbulletCube->shootDirection) << std::endl;
						auto newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), angleWallBullet);
						//std::cout << "New Direction : " << glm::to_string(newDirection*-1) << std::endl;

						//std::cout << "New Direction *-1 : " << roundf(newDirection.x*-1000)/1000 << "," << roundf(newDirection.y * -1000) / 1000 << " bullet direction " << roundf(collidingbulletCube->shootDirection.x * 1000) / 1000 << "," << roundf(collidingbulletCube->shootDirection.z * 1000) / 1000 << std::endl;

						glm::vec2 roundedNewDir = glm::vec2(roundf(newDirection.x * -1000) / 1000, roundf(newDirection.y * -1000) / 1000);
						glm::vec2 roundedShootDirection = glm::vec2(roundf(collidingPlayer->movementDirection.x * 1000) / 1000, roundf(collidingPlayer->movementDirection.z * 1000) / 1000);

						if (roundedNewDir.x == roundedShootDirection.x && roundedNewDir.y == roundedShootDirection.y) {
							//std::cout << "Reversing shoot direction " << std::endl;
							newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), -1 * angleWallBullet);
						}
						//collidingPlayer->movementDirection = glm::vec3(newDirection.x, 0, newDirection.y);

						collidingPlayer->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(newDirection.x, 0, newDirection.y)));
					}
					/*
					auto angleWallPlayer = glm::angle(wallCube->wallNorm, static_cast<PlayerCube*>(collidWithVec.at(i))->movementDirection);
					std::cout << glm::degrees(angleWallPlayer) << std::endl;
					auto collisionDirection = glm::rotate(glm::vec2(static_cast<PlayerCube*>(collidWithVec.at(i))->movementDirection.x, static_cast<PlayerCube*>(collidWithVec.at(i))->movementDirection.z), glm::radians(glm::degrees(angleWallPlayer)));
					static_cast<PlayerCube*>(collidWithVec.at(i))->cube->multiplyTransform(glm::translate(glm::mat4(1.0), glm::vec3(collisionDirection.x,0, collisionDirection.y)*10));
					std::cout << glm::to_string(collidWithVec.at(i)->cube->getPosition()) << std::endl;
					*/

					if (collidWithVec->at(i)->cubeType == "Enemy") {
						//auto collidingbulletCube = static_cast<PlayerCube*>(collidWithVec->at(i));
						//collidingbulletCube->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), wallCube->wallNorm));

						//std::cout << "Bullet Wall Collisision" << std::endl;
						auto collidingEnemy = static_cast<EnemyCube*>(collidWithVec->at(i));
						//calculate bouncing angle for bullet with wall normal
						auto angleWallEnemy = glm::angle(wallCube->wallNorm, -1 * collidingEnemy->movementDirection);
						//std::cout << "angle between wall norm and bullet: " << glm::degrees(angleWallEnemy) << std::endl;
						//std::cout << "Shootdirection : " << glm::to_string(collidingbulletCube->shootDirection) << std::endl;
						auto newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), angleWallEnemy);
						//std::cout << "New Direction : " << glm::to_string(newDirection*-1) << std::endl;

						//std::cout << "New Direction *-1 : " << roundf(newDirection.x*-1000)/1000 << "," << roundf(newDirection.y * -1000) / 1000 << " bullet direction " << roundf(collidingbulletCube->shootDirection.x * 1000) / 1000 << "," << roundf(collidingbulletCube->shootDirection.z * 1000) / 1000 << std::endl;

						glm::vec2 roundedNewDir = glm::vec2(roundf(newDirection.x * -1000) / 1000, roundf(newDirection.y * -1000) / 1000);
						glm::vec2 roundedShootDirection = glm::vec2(roundf(collidingEnemy->movementDirection.x * 1000) / 1000, roundf(collidingEnemy->movementDirection.z * 1000) / 1000);

						if (roundedNewDir.x == roundedShootDirection.x && roundedNewDir.y == roundedShootDirection.y) {
							//std::cout << "Reversing shoot direction " << std::endl;
							newDirection = glm::rotate(glm::vec2(wallCube->wallNorm.x, wallCube->wallNorm.z), -1 * angleWallEnemy);
						}
						//collidingPlayer->movementDirection = glm::vec3(newDirection.x, 0, newDirection.y);

						//std::cout << collidingEnemy->enemyType << std::endl;

						if (collidingEnemy->enemyType == "ShyGuy") {
							//std::cout << "ShuyGuy Move " << std::endl;
							collidingEnemy->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(newDirection.x, 0, newDirection.y)*0.5));
							collidingEnemy->movementDirection = glm::vec3(newDirection.x, 0, newDirection.y);
							collidingEnemy->collWall = true;
						}
						if (collidingEnemy->enemyType == "Mud") {
							//std::cout << "ShuyGuy Move " << std::endl;
							collidingEnemy->collWall = true;
						}
					}
				}
			}
		}
	};

	class EnemyListener : public VEEventListener {
		PlayerCube* playerCube;
		VESceneNode* pScene;
		std::vector <EnemyCube*> activeEnemies;
		bool startWave = false;
		int enemyCount = 2;
		std::vector<Cube*>* collidWithVec;
		float timeTillShyGuyFlee = 0;
		std::vector<Cube*>* wallVec;
		bool* enemySwitchPressed;
		bool enemySpawned = false;
		float timeOfSlowDown=0;
	public:
		EnemyListener(std::string name, PlayerCube* playerCube, VESceneNode* pScene, int startingEnemyAmount, std::vector<Cube*>* collidWithVec, std::vector<Cube*>* wallVec, bool* enemySwitchPressed) : VEEventListener(name), playerCube(playerCube), pScene(pScene), collidWithVec(collidWithVec), wallVec(wallVec), enemySwitchPressed(enemySwitchPressed) {
		};

/**/

		void onFrameStarted(veEvent event) {
			if (!g_gameLost) {
				if (*enemySwitchPressed && !enemySpawned) {
					if (!activeEnemies.empty()) {
						for (int i = 0; i < activeEnemies.size(); i++) {
							for (int count = 0; count < collidWithVec->size(); count++) {
								if (collidWithVec->at(count) == activeEnemies.at(i)) {
									getSceneManagerPointer()->deleteSceneNodeAndChildren("enemy" + std::to_string(i));
									collidWithVec->erase(collidWithVec->begin() + count);
								}
							}
						}
						activeEnemies.clear();
					}

					for (int i = 0; i < enemyCount; i++) {

						int randEnemyType = rand() % 3 + 1;

						VESceneNode* enemy;
						if (randEnemyType == 1) {
							VECHECKPOINTER(enemy = getSceneManagerPointer()->loadModel("enemy" + std::to_string(i), "media/models/gamemodels", "d20.obj"));
						}
						else if (randEnemyType == 2) {
							VECHECKPOINTER(enemy = getSceneManagerPointer()->loadModel("enemy" + std::to_string(i), "media/models/gamemodels", "donut.obj"));
						}
						else if (randEnemyType == 3) {
							VECHECKPOINTER(enemy = getSceneManagerPointer()->loadModel("enemy" + std::to_string(i), "media/models/gamemodels", "turret.obj"));
						}
						pScene->addChild(enemy);

						//calculate random position to spawn enemy in play area
						int randx = (rand() % 52 + 1) - 26;
						int randz = (rand() % 52 + 1) - 26;
						if (randx >= 20 || randx <= -20) {
							int tocalc = 38 + (abs(randx) * -1);
							randz = (rand() % tocalc + 1) - (tocalc / 2);
						}
						else if (randz >= 20 || randz <= -20) {
							int tocalc = 38 + (abs(randz) * -1);
							randx = (rand() % tocalc + 1) - (tocalc / 2);
						}

						enemy->setPosition(glm::vec3(randx, 1, randz));

						//std::cout << glm::to_string(enemy->getPosition()) << std::endl;

						//int randEnemyType = 2;

						std::string chosenEnemyType;

						switch (randEnemyType) {
						case 1:
							chosenEnemyType = "ShyGuy";
							break;
						case 2:
							chosenEnemyType = "Mud";
							break;
						case 3:
							chosenEnemyType = "Turret";
							break;
							//case4:
								//chosenEnemyType = "Mimic";
								//break;
						default:
							break;
						}

						EnemyCube* enemyCube = new EnemyCube("enemy" + std::to_string(i), glm::mat3(1.0f), enemy, "Enemy", 0.2, chosenEnemyType);

						enemyCube->collisionCount = 0;

						activeEnemies.push_back(enemyCube);

						collidWithVec->push_back(enemyCube);
						//std::cout << chosenEnemyType << std::endl;
						if (chosenEnemyType == "Turret") {
							int bulletcount = 2;
							for (int j = 0; j < bulletcount; j++) {
								VESceneNode* enemyBullet;
								VECHECKPOINTER(enemyBullet = getSceneManagerPointer()->loadModel("enemyBullet" + std::to_string(i) + std::to_string(j), "media/models/gamemodels", "enemySphere.obj"));
								pScene->addChild(enemyBullet);
								//enemyBullet->multiplyTransform(glm::rotate(glm::radians(45.0f), glm::vec3(0, 1, 0)));
								enemyBullet->setPosition(activeEnemies.at(i)->cube->getPosition());
								BulletCube* enemyBulletCube = new BulletCube("enemyBullet" + std::to_string(i) + std::to_string(j), glm::mat3(1.0f), enemyBullet, "EnemyBullet", 0.2, activeEnemies.at(i)->cube->getPosition(), false);
								//collidWithVec->push_back(enemyBulletCube);
								activeEnemies.at(i)->availableBullets.push_back(enemyBulletCube);

								float randxdir = ((rand() % 20 + 1) - 10) / 10;
								float randzdir = ((rand() % 20 + 1) - 10) / 10;

								static_cast<BulletCube*>(activeEnemies.at(i)->availableBullets.at(j))->shootDirection = glm::normalize(glm::vec3(randx, 0, randz));

								if (j == 1) {
									static_cast<BulletCube*>(activeEnemies.at(i)->availableBullets.at(j))->shootDirection = glm::normalize(glm::vec3(-randx, 0, -randz));
								}

							}
						}
						//std::cout << "CollisionVector Size " << collidWithVec->size() << std::endl;
						//std::cout << "ActiveEnemies Size " << activeEnemies.size() << std::endl;
						enemySpawned = true;
					}
					enemyCount++;
					/*
					enemyCount--;
					getSceneManagerPointer()->deleteSceneNodeAndChildren("enemy" + std::to_string(enemyCount));
					activeEnemies.erase(activeEnemies.begin() + enemyCount);
					*/
					startWave = true;
				}
				if (startWave == true) {
					//std::cout << "Enemy Wave started" << std::endl;
					for (int i = 0; i < activeEnemies.size(); i++) {
						if (activeEnemies.at(i)->enemyType == "ShyGuy") {
							if (activeEnemies.at(i)->triggerDistance <= 0.0f) {
								float randAngle = glm::radians(float(rand() % rand() % 360 + 1));
								activeEnemies.at(i)->movementDirection = glm::vec3(sin(randAngle), 0.0f, cos(randAngle));
								activeEnemies.at(i)->movementSpeed = 0.05f;
								activeEnemies.at(i)->triggerDistance = 10.0f;
							}

							if (activeEnemies.at(i)->collWall == false && glm::distance(playerCube->cube->getPosition(), activeEnemies.at(i)->cube->getPosition()) < 5.0) {
								//activeEnemies.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), (activeEnemies.at(i)->cube->getPosition() - player->getPosition()) * 0.03));
								activeEnemies.at(i)->movementDirection = glm::normalize(activeEnemies.at(i)->cube->getPosition() - playerCube->cube->getPosition());
								activeEnemies.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), activeEnemies.at(i)->movementDirection * 0.35));

							}
							else {
								activeEnemies.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), activeEnemies.at(i)->movementDirection * activeEnemies.at(i)->movementSpeed));

								//std::cout << glm::to_string(movementDirection) << std::endl; 
								//std::cout << distance << std::endl;

								activeEnemies.at(i)->triggerDistance -= activeEnemies.at(i)->movementSpeed;
								//activeEnemies.at(i)->collWall = false;

							}
							if (activeEnemies.at(i)->collWall == true) {
								timeTillShyGuyFlee += event.dt;
								if (timeTillShyGuyFlee >= 1.5) {
									timeTillShyGuyFlee = 0;
									activeEnemies.at(i)->collWall = false;
								}
							}
						}

						if (activeEnemies.at(i)->enemyType == "Mud") {
							//std::cout << glm::to_string(followVector) << std::endl;
							activeEnemies.at(i)->movementSpeed = (rand() % 1000 + 100) / 10000.0f;
							//std::cout << "MudSpeed " << mudSpeed << std::endl;
							if (activeEnemies.at(i)->collWall == true) {
								timeOfSlowDown += event.dt;
								activeEnemies.at(i)->movementSpeed = 0.005;
								//std::cout << timeOfSlowDown << std::endl;
								if (timeOfSlowDown >= 1.5) {
									timeOfSlowDown = 0;
									activeEnemies.at(i)->collWall = false;
									activeEnemies.at(i)->movementSpeed = (rand() % 1000 + 100) / 10000.0f;
								}
							}
							activeEnemies.at(i)->movementDirection = glm::normalize(playerCube->cube->getPosition() - activeEnemies.at(i)->cube->getPosition());
							activeEnemies.at(i)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), activeEnemies.at(i)->movementDirection * activeEnemies.at(i)->movementSpeed));
						}

						if (activeEnemies.at(i)->enemyType == "Turret") {
							if (activeEnemies.at(i)->availableBullets.size()) {
								for (int j = 0; j < activeEnemies.at(i)->availableBullets.size(); j++) {
									activeEnemies.at(i)->availableBullets.at(j)->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), static_cast<BulletCube*>(activeEnemies.at(i)->availableBullets.at(j))->shootDirection * 0.1));
									//std::cout << glm::to_string(activeEnemies.at(i)->movementDirection) << std::endl;
								}
							}
							for (int j = 0; j < collidWithVec->size(); j++) {
								for (int k = 0; k < activeEnemies.at(i)->availableBullets.size(); k++) {
									if (gjk(collidWithVec->at(j), activeEnemies.at(i)->availableBullets.at(k))) {
										if (collidWithVec->at(j)->cubeType == "Player") {
											//std::cout << "Enemy Player Collision " << std::endl;
											collidWithVec->at(j)->collisionCount++;
											activeEnemies.at(i)->availableBullets.at(k)->collisionCount++;
											if (activeEnemies.at(i)->availableBullets.at(k)->collisionCount >= 1) {
												activeEnemies.at(i)->availableBullets.at(k)->collisionCount = 0;
												float randxdir = ((rand() % 20 + 1) - 10) / 10;
												float randzdir = ((rand() % 20 + 1) - 10) / 10;
												activeEnemies.at(i)->availableBullets.at(k)->cube->setPosition(static_cast<BulletCube*>(activeEnemies.at(i)->availableBullets.at(k))->spawnPosition);
												//static_cast<BulletCube*>(collidWithVec->at(j))->shootDirection = glm::vec3(randxdir, 0, randzdir);
												//static_cast<BulletCube*>(collidWithVec->at(i))->shootDirection = glm::normalize(glm::vec3(randxdir, 0, randzdir));

												//std::cout << " Colliding End " << collidWithVec->at(i)->name << std::endl;
											}
											//std::cout << "Player Collision Count " << collidWithVec->at(i)->collisionCount << std::endl;
											static_cast<PlayerCube*>(collidWithVec->at(j))->liveLost = true;
										}
									}
								}
							}
							for (int k = 0; k < activeEnemies.at(i)->availableBullets.size(); k++) {
								for (int j = 0; j < wallVec->size(); j++) {
									if (gjk(wallVec->at(j), activeEnemies.at(i)->availableBullets.at(k))) {
										if (activeEnemies.at(i)->availableBullets.at(k)->collisionCount >= 1) {
											activeEnemies.at(i)->availableBullets.at(k)->collisionCount = 0;
											activeEnemies.at(i)->availableBullets.at(k)->cube->setPosition(static_cast<BulletCube*>(activeEnemies.at(i)->availableBullets.at(k))->spawnPosition);
											//std::cout << " Colliding End " << collidWithVec->at(i)->name << std::endl;
										}
										else {
											//std::cout << collidWithVec->at(i)->collisionCount << " EnemyBullet " << collidWithVec->at(i)->name << std::endl;

											//std::cout << "Bullet Wall Collisision" << std::endl;
											auto collidingbulletCube = static_cast<BulletCube*>(activeEnemies.at(i)->availableBullets.at(k));
											//calculate bouncing angle for bullet with wall normal
											auto angleWallBullet = glm::angle(static_cast<WallCube*>(wallVec->at(j))->wallNorm, -1 * collidingbulletCube->shootDirection);
											//std::cout << "angle between wall norm and bullet: " << glm::degrees(angleWallBullet) << std::endl;
											//std::cout << "Shootdirection : " << glm::to_string(collidingbulletCube->shootDirection) << std::endl;
											auto newDirection = glm::rotate(glm::vec2(static_cast<WallCube*>(wallVec->at(j))->wallNorm.x, static_cast<WallCube*>(wallVec->at(j))->wallNorm.z), angleWallBullet);
											//std::cout << "New Direction : " << glm::to_string(newDirection*-1) << std::endl;

											//std::cout << "New Direction *-1 : " << roundf(newDirection.x*-1000)/1000 << "," << roundf(newDirection.y * -1000) / 1000 << " bullet direction " << roundf(collidingbulletCube->shootDirection.x * 1000) / 1000 << "," << roundf(collidingbulletCube->shootDirection.z * 1000) / 1000 << std::endl;

											glm::vec2 roundedNewDir = glm::vec2(roundf(newDirection.x * -1000) / 1000, roundf(newDirection.y * -1000) / 1000);
											glm::vec2 roundedShootDirection = glm::vec2(roundf(collidingbulletCube->shootDirection.x * 1000) / 1000, roundf(collidingbulletCube->shootDirection.z * 1000) / 1000);

											if (roundedNewDir.x == roundedShootDirection.x && roundedNewDir.y == roundedShootDirection.y) {
												//std::cout << "Reversing shoot direction " << std::endl;
												newDirection = glm::rotate(glm::vec2(static_cast<WallCube*>(wallVec->at(j))->wallNorm.x, static_cast<WallCube*>(wallVec->at(j))->wallNorm.z), -1 * angleWallBullet);
											}
											collidingbulletCube->shootDirection = glm::vec3(newDirection.x, 0, newDirection.y);

											//collidingbulletCube->shootDirection = wallCube->wallNorm;
											activeEnemies.at(i)->availableBullets.at(k)->collisionCount++;
										}
									}

								}
							}
						}

						bool enemyDelete = false;
						int deletePosActive = 0;
						int deletePosCollideWith = 0;
						for (int i = 0; i < activeEnemies.size(); i++) {
							if (activeEnemies.at(i)->collisionCount >= 1) {
								for (int count = 0; count < collidWithVec->size(); count++) {
									if (collidWithVec->at(count) == activeEnemies.at(i)) {
										enemyDelete = true;
										deletePosActive = i;
										deletePosCollideWith = count;
									}
								}
							}
						}
						if (enemyDelete && activeEnemies.at(deletePosActive)->enemyType != "Turret") {
							getSceneManagerPointer()->deleteSceneNodeAndChildren(activeEnemies.at(deletePosActive)->name);
							activeEnemies.erase(activeEnemies.begin() + deletePosActive);
							collidWithVec->erase(collidWithVec->begin() + deletePosCollideWith);
							enemyDelete = false;
						}
						else if (enemyDelete && activeEnemies.at(deletePosActive)->enemyType == "Turret") {
							int bulletsToDelete = 0;
							getSceneManagerPointer()->deleteSceneNodeAndChildren(activeEnemies.at(deletePosActive)->name);
							getSceneManagerPointer()->deleteSceneNodeAndChildren(activeEnemies.at(deletePosActive)->availableBullets.at(0)->name);
							getSceneManagerPointer()->deleteSceneNodeAndChildren(activeEnemies.at(deletePosActive)->availableBullets.at(1)->name);
							bulletsToDelete = activeEnemies.at(deletePosActive)->availableBullets.size();
							//delete 2 bullets before deleting the actual turret
							/*
							if (bulletsToDelete == 2) {
								int bulletDeletePos = 0;
								for (int count = 0; count < collidWithVec->size(); count++) {
									if (collidWithVec->at(count) == activeEnemies.at(deletePosActive)->availableBullets.at(0)) {
										bulletDeletePos = count;
									}
								}
								collidWithVec->erase(collidWithVec->begin() + bulletDeletePos);

								bulletDeletePos = 0;
								for (int count = 0; count < collidWithVec->size(); count++) {
									if (collidWithVec->at(count) == activeEnemies.at(deletePosActive)->availableBullets.at(1)) {
										bulletDeletePos = count;
									}
								}
								collidWithVec->erase(collidWithVec->begin() + bulletDeletePos);
							}
							else if (bulletsToDelete == 1) {
								int bulletDeletePos = 0;
								for (int count = 0; count < collidWithVec->size(); count++) {
									if (collidWithVec->at(count) == activeEnemies.at(deletePosActive)->availableBullets.at(0)) {
										bulletDeletePos = count;
									}
								}
								collidWithVec->erase(collidWithVec->begin() + bulletDeletePos);
							}
							*/
							activeEnemies.at(deletePosActive)->availableBullets.clear();
							activeEnemies.erase(activeEnemies.begin() + deletePosActive);
							collidWithVec->erase(collidWithVec->begin() + deletePosCollideWith);
							enemyDelete = false;
							//std::cout << "enemy" + std::to_string(deletePosActive) << std::endl;
						}
					}
					if (activeEnemies.empty()) {
						*enemySwitchPressed = false;
						enemySpawned = false;
					}
				}
			}
		};
	};

	class SwitchListener : public VEEventListener {
		Cube* playerCube;
		VESceneNode* pScene;
		Cube* switchCube = nullptr;
		bool* enemySwitchPressed; 

	public:
		SwitchListener(std::string name, Cube* playerCube, VESceneNode* pScene, bool * enemySwitchPressed): VEEventListener(name), playerCube(playerCube), pScene(pScene), enemySwitchPressed(enemySwitchPressed){
			VESceneNode* enemySwitch;
			VECHECKPOINTER(enemySwitch = getSceneManagerPointer()->loadModel("enemySwitch", "media/models/test/crate0", "cube.obj"));
			pScene->addChild(enemySwitch);
			enemySwitch->setPosition(playerCube->cube->getPosition() + glm::vec3(0, 0, 5));
			switchCube = new Cube("SwitchCube", glm::mat3(1.0f), enemySwitch, "Switch");
		}

		void onFrameStarted(veEvent event) {
			if (gjk(playerCube, switchCube)) {
				switchCube->cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, -40, 0)));
				switchCube->collisionCount++;
				*enemySwitchPressed = true;
				std::cout << "Pressing Switch " << switchCube->collisionCount << std::endl;
			}
			if (*enemySwitchPressed == false) {
				switchCube->cube->setPosition(glm::vec3(0, 1, 5));
			}
		}
	};

	class CollisionResponseListener : public VEEventListener {
		VESceneNode* scene = nullptr;
		VEEntity* currentcube = nullptr;
		int cubecounter = 0;
		std::vector<CollisionCubePhysics*> activeCubes;
		bool spacebarpressed = false;

	public: 
		CollisionResponseListener(std::string name, VESceneNode* pScene) : VEEventListener(name), scene(pScene) {

		}

		virtual bool onKeyboard(veEvent event) {

			if (event.idata1 == GLFW_KEY_SPACE && event.idata3 == GLFW_PRESS) {
				cubecounter++;

				srand(time(NULL));
				int randx = rand() % 2;
				int randy = rand() % 2;
				int randz = rand() % 2;
				if (randx && randy) {
					randz = 0;
				}
				if (randx && randz) {
					randy = 0;
				}
				if (randy && randz) {
					randx = 0;
				}
				glm::vec3 rotationvec = glm::vec3(randx, randy, randz);
				float randdegrees = (rand() % 360);
				std::cout << randdegrees << std::endl;
				float rotangle = glm::radians(randdegrees);
				glm::mat4 rotationMatrix = rotationMatrix = glm::rotate(rotangle, rotationvec);
				CollisionCubePhysics *fallingCube = new CollisionCubePhysics("Cube" + std::to_string(cubecounter), scene, rotationMatrix);
				activeCubes.push_back(fallingCube);
				spacebarpressed = true;
				return true;
			}

			return false;
		}

		void onFrameStarted(veEvent event) {
			if (spacebarpressed) {
				for (int i = 0; i < activeCubes.size(); i++) {
					activeCubes.at(i)->calculateCubePhysics(event);
				}
			}
		}
	};


	///simple event listener for rotating objects
	class RotatorListener : public VEEventListener {
		VESceneNode* m_pObject = nullptr;
		float m_speed;
		glm::vec3 m_axis;
	public:
		///Constructor
		RotatorListener(std::string name, VESceneNode* pObject, float speed, glm::vec3 axis) :
			VEEventListener(name), m_pObject(pObject), m_speed(speed), m_axis(glm::normalize(axis)) {};

		void onFrameStarted(veEvent event) {
			glm::mat4 rot = glm::rotate(glm::mat4(1.0f), m_speed * (float)event.dt, m_axis);
			m_pObject->multiplyTransform(rot);
		}

		bool onSceneNodeDeleted(veEvent event) {
			if (m_pObject == event.ptr) return true;
			return false;
		};
	};


	///simple event listener for rotating objects
	class BlinkListener : public VEEventListener {
		VEEntity* m_pEntity;
		double t_now = 0.0;
		double t_last = 0.0;
		double m_blinkDuration;

	public:
		///Constructor
		BlinkListener(std::string name, VEEntity* pEntity, double duration) :
			VEEventListener(name), m_pEntity(pEntity), m_blinkDuration(duration) {};

		///\brief let cubes blink
		void onFrameStarted(veEvent event) {
			t_now += event.dt;
			double duration = t_now - t_last;

			if (duration > m_blinkDuration) {
				m_pEntity->m_visible = m_pEntity->m_visible ? false : true;	//toggle visibility
				t_last = t_now;
			}
		}

		///\returns true if this event listener instance should be deleted also
		bool onSceneNodeDeleted(veEvent event) {
			if (m_pEntity == event.ptr) return true;
			return false;
		};

	};


	///simple event listener for loading levels
	class LevelListener : public VEEventListener {
	public:
		///Constructor
		LevelListener(std::string name) : VEEventListener(name) {};

		///load a new level when pressing numbers 1-3
		virtual bool onKeyboard(veEvent event) {
			if (event.idata3 == GLFW_RELEASE) return false;

			if (event.idata1 == GLFW_KEY_1 && event.idata3 == GLFW_PRESS) {
				getSceneManagerPointer()->deleteScene();
				getEnginePointer()->loadLevel(1);
				return true;
			}

			if (event.idata1 == GLFW_KEY_2 && event.idata3 == GLFW_PRESS) {
				getSceneManagerPointer()->deleteScene();
				getEnginePointer()->loadLevel(2);
				return true;
			}

			if (event.idata1 == GLFW_KEY_3 && event.idata3 == GLFW_PRESS) {
				getSceneManagerPointer()->deleteScene();
				getEnginePointer()->loadLevel(3);
				return true;
			}
			return false;
		}
	};


	///simple event listener for switching on/off light
	class LightListener : public VEEventListener {
	public:
		///Constructor
		LightListener(std::string name) : VEEventListener(name) {};

		///\brief switch on or off a given light
		void toggleLight(std::string name) {
			VELight* pLight = (VELight*)getSceneManagerPointer()->getSceneNode(name);
			if (pLight == nullptr) return;

			pLight->m_switchedOn = pLight->m_switchedOn ? false : true;
		}

		///load a new level when pressing numbers 1-3
		virtual bool onKeyboard(veEvent event) {
			if (event.idata3 == GLFW_RELEASE) return false;

			if (event.idata1 == GLFW_KEY_8 && event.idata3 == GLFW_PRESS) {
				toggleLight("StandardDirLight");
				return true;
			}

			if (event.idata1 == GLFW_KEY_9 && event.idata3 == GLFW_PRESS) {
				toggleLight("StandardPointLight");
				return true;
			}

			if (event.idata1 == GLFW_KEY_0 && event.idata3 == GLFW_PRESS) {
				toggleLight("StandardSpotLight");
				return true;
			}

			return false;
		}
	};



	///user defined manager class, derived from VEEngine
	class MyVulkanEngine : public VEEngine {
	protected:

	public:
		/**
		* \brief Constructor of my engine
		* \param[in] debug Switch debuggin on or off
		*/
		MyVulkanEngine(bool debug = false) : VEEngine(debug) {};
		~MyVulkanEngine() {};

		///Register an event listener to interact with the user
		virtual void registerEventListeners() {
			VEEngine::registerEventListeners();

			registerEventListener(new LevelListener("LevelListener"), { veEvent::VE_EVENT_KEYBOARD });
			registerEventListener(new LightListener("LightListener"), { veEvent::VE_EVENT_KEYBOARD });
			registerEventListener(new VEEventListenerNuklearDebug("NuklearDebugListener"), { veEvent::VE_EVENT_DRAW_OVERLAY });
		};

		///create many cubes
		void createCubes(uint32_t n, VESceneNode* parent) {

			float stride = 300.0f;
			static std::default_random_engine e{ 12345 };
			static std::uniform_real_distribution<> d{ 1.0f, stride };
			static std::uniform_real_distribution<> r{ -1.0f, 1.0f };
			static std::uniform_real_distribution<> v{ 0.1f, 5.0f };
			static std::uniform_real_distribution<> dur{ 900.3f, 1000.0f };

			VEMesh* pMesh;
			VECHECKPOINTER(pMesh = getSceneManagerPointer()->getMesh("media/models/test/crate0/cube.obj/cube"));

			VEMaterial* pMat;
			VECHECKPOINTER(pMat = getSceneManagerPointer()->getMaterial("media/models/test/crate0/cube.obj/cube"));

			for (uint32_t i = 0; i < n; i++) {
				VESceneNode* pNode;
				VECHECKPOINTER(pNode = getSceneManagerPointer()->createSceneNode("The Node" + std::to_string(i), parent));
				pNode->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(d(e) - stride / 2.0f, d(e) / 2.0f, d(e) - stride / 2.0f)));
				registerEventListener(new RotatorListener("RotatorListener" + std::to_string(i), pNode, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f)), { veEvent::VE_EVENT_FRAME_STARTED, veEvent::VE_EVENT_DELETE_NODE });

				VEEntity* e2;
				VECHECKPOINTER(e2 = getSceneManagerPointer()->createEntity("The Cube" + std::to_string(i), pMesh, pMat, pNode));
				registerEventListener(new RotatorListener("RotatorListener2" + std::to_string(i), e2, v(e), glm::vec3(r(e), r(e), r(e))), { veEvent::VE_EVENT_FRAME_STARTED, veEvent::VE_EVENT_DELETE_NODE });
				//registerEventListener( new BlinkListener("BlinkListener" + std::to_string(i), e2, dur(e) ), { veEvent::VE_EVENT_FRAME_STARTED, veEvent::VE_EVENT_DELETE_NODE });
			}

		}

		void setUpWall(std::string wallName, VESceneNode* player, VESceneNode* pScene, std::vector<Cube*> *collidWithVec, std::vector<Cube*>* wallVec,  float rotationAngle, glm::vec3 translatePosition, glm::vec3 scaleVec, bool supportWall) {

			VESceneNode* Wall;
			VECHECKPOINTER(Wall = getSceneManagerPointer()->loadModel(wallName, "media/models/test/crate0", "cube.obj"));
			Wall->multiplyTransform(glm::scale(glm::mat4(1.0f), scaleVec));
			Wall->multiplyTransform(glm::rotate(glm::radians(rotationAngle), glm::vec3(0, 1, 0)));
			Wall->multiplyTransform(glm::translate(glm::mat4(1.0f), player->getPosition() + translatePosition));
			pScene->addChild(Wall);
			glm::mat3 rotmat = glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0));

			float rotationangle_CollWall = rotationAngle - 90.0f;

			registerEventListener(new WallListener(wallName, Wall, glm::rotate(glm::radians(rotationangle_CollWall), glm::vec3(0, 1, 0)), collidWithVec, wallVec, (scaleVec * rotmat), supportWall), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });

		}

		BulletCube* createBullet(std::string name, PlayerCube* Player, VESceneNode* pScene, glm::vec3 spawnPosition, bool supportbullet) {
			glm::mat3 rotationMatrix = glm::mat3(1.0f);
			VESceneNode* bullet;
			VECHECKPOINTER(bullet = getSceneManagerPointer()->loadModel(name, "media/models/gamemodels", "sphere.obj"));
			bullet->multiplyTransform(glm::translate(glm::mat4(1.0f), spawnPosition));
			pScene->addChild(bullet);
			BulletCube* bulletCube = new BulletCube(name, glm::mat3(1.0f), bullet, "Bullet", 0.2f, spawnPosition, false);

			if (supportbullet) {
				bulletCube = new BulletCube(name, glm::mat3(1.0f), bullet, "Bullet", 0.2f, spawnPosition, true);
			}
			registerEventListener(new BulletListener(name, bullet, glm::mat3(1.0f), Player, bulletCube, supportbullet), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });
			//std::cout << name << std::endl;
			return bulletCube;
		}

		void ceateEnemies(std::string name, PlayerCube* playerCube, VESceneNode* pScene, int startingEnemyAmount, std::vector<Cube*>* collidWithVec, std::vector<Cube*>* wallVec, bool* enemySwitchPressed) {
			registerEventListener(new EnemyListener(name, playerCube, pScene, startingEnemyAmount, collidWithVec, wallVec, enemySwitchPressed), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });
		}

		///create many lights
		void createLights(uint32_t n, VESceneNode* parent) {
			float stride = 200.0f;
			static std::default_random_engine e{ 12345 };
			static std::uniform_real_distribution<> d{ 1.0f, stride };

			for (uint32_t i = 0; i < n; i++) {
				VELight* pLight;
				VECHECKPOINTER(pLight = getSceneManagerPointer()->createLight("Light" + std::to_string(i), VELight::VE_LIGHT_TYPE_SPOT, parent));
				pLight->m_col_diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
				pLight->m_col_specular = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
				pLight->m_param[0] = stride / 3.0f;
				//pLight->multiplyTransform(glm::translate(glm::vec3(d(e) - stride / 2.0f, d(e)/10.0f, d(e) - stride / 2.0f)));

				pLight->lookAt(glm::vec3(d(e) - stride / 2.0f, d(e) / 30.0f, d(e) - stride / 2.0f),
					glm::vec3(d(e) - stride / 2.0f, 0.0f, d(e) - stride / 2.0f),
					glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}

		///Load the first level into the game engine
		///The engine uses Y-UP, Left-handed
		virtual void loadLevel(uint32_t numLevel = 1) {

			VEEngine::loadLevel(numLevel);			//create standard cameras and lights

			VESceneNode* pScene;
			VECHECKPOINTER(pScene = getSceneManagerPointer()->createSceneNode("Level 1", getRoot()));

			//scene models

			VESceneNode* sp1;
			VECHECKPOINTER(sp1 = getSceneManagerPointer()->createSkybox("The Sky", "media/models/test/sky/cloudy",
				{ "bluecloud_ft.jpg", "bluecloud_bk.jpg", "bluecloud_up.jpg",
					"bluecloud_dn.jpg", "bluecloud_rt.jpg", "bluecloud_lf.jpg" }, pScene));

			RotatorListener* pRot;
			VECHECKPOINTER(pRot = new RotatorListener("CubemapRotator", sp1, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f)));
			getEnginePointer()->registerEventListener(pRot, { veEvent::VE_EVENT_DELETE_NODE, veEvent::VE_EVENT_FRAME_STARTED });

			VESceneNode* e4;
			VECHECKPOINTER(e4 = getSceneManagerPointer()->loadModel("The Plane", "media/models/test", "plane_t_n_s.obj", 0, pScene));
			e4->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1.0f, 1000.0f)));

			VEEntity* pE4;
			VECHECKPOINTER(pE4 = (VEEntity*)getSceneManagerPointer()->getSceneNode("The Plane/plane_t_n_s.obj/plane/Entity_0"));
			pE4->setParam(glm::vec4(1000.0f, 1000.0f, 0.0f, 0.0f));
			/*
						VESceneNode *pointLight = getSceneManagerPointer()->getSceneNode("StandardPointLight");
						if (pointLight != nullptr) {
							VESceneNode *eL;
							VECHECKPOINTER(eL = getSceneManagerPointer()->loadModel("The Light", "media/models/test/sphere", "sphere.obj", 0, pointLight));
							eL->multiplyTransform(glm::scale(glm::vec3(0.02f, 0.02f, 0.02f)));

							VEEntity *pE;
							VECHECKPOINTER( pE = (VEEntity*)getSceneManagerPointer()->getSceneNode("The Light/sphere.obj/default/Entity_0"));
							pE->m_castsShadow = false;
						}
			*/
			VESceneNode* player;
			VECHECKPOINTER(player = getSceneManagerPointer()->loadModel("player", "media/models/gamemodels", "cube.obj"));
			ScaleObject(player, glm::vec3(1, 1, 1));
			player->multiplyTransform(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)));
			pScene->addChild(player);

			PlayerCube* playerCube = new PlayerCube("player", glm::mat3(1.0f), player, "Player", 0.12);

			//create a Bullet for the player to shoot 

			VECamera* pCamera = getSceneManagerPointer()->getCamera();

			float rotangle = glm::radians(45.0f);
			glm::vec3 rotvec = glm::vec3(1, 0, 0);
			pCamera->multiplyTransform(glm::rotate(rotangle, rotvec));

			pCamera->setPosition(glm::vec3(0.0, 50, 0.0));

			std::vector<Cube*>* collidWithVec = new std::vector<Cube*> ();

			std::vector<Cube*>* wallVec = new std::vector<Cube*> ();

			registerEventListener(new PlayerListener("PlayerListerner", player, pScene, pCamera, playerCube), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });

			registerEventListener(new LivesListener("LiveListener", playerCube, pScene), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });

			//creating bullets 
			auto bullet1 = createBullet("Bullet1", playerCube, pScene, glm::vec3(0.0, -2.0, 0.0), false); 
			auto bullet2 = createBullet("Bullet2", playerCube, pScene, glm::vec3(0.0, -4.0, 0.0), false);
			auto bullet3 = createBullet("Bullet3", playerCube, pScene, glm::vec3(0.0, -6.0, 0.0), false);
			auto bullet4 = createBullet("Bullet4", playerCube, pScene, glm::vec3(0.0, -8.0, 0.0), false);

			//create 4 support bulltes for each bullet 

			collidWithVec->push_back(playerCube);
			collidWithVec->push_back(bullet1);
			collidWithVec->push_back(bullet2);
			collidWithVec->push_back(bullet3);
			collidWithVec->push_back(bullet4);

			
			for (int i = 0; i < collidWithVec->size(); i++) {
				if (collidWithVec->at(i)->cubeType == "Bullet") {
					for (int j = 0; j < 4; j++) {
						BulletCube* supportBullet = createBullet(" SupportBullet " + std::to_string(i) + std::to_string(j), playerCube, pScene, glm::vec3(0.0, -20.0, 0.0), true);
						static_cast<BulletCube*>(collidWithVec->at(i))->supportCubes.push_back(supportBullet);

					}
				}
			}
			//std::cout << static_cast<BulletCube*>(collidWithVec.at(1))->supportCubes.size() << std::endl;
			
			registerEventListener(new CollisionListener("BulletCollisionListener", collidWithVec), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });


			setUpWall("WallUpRight", player, pScene, collidWithVec, wallVec, 45.0f, glm::vec3(20, 0, 20), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallUpLeft", player, pScene, collidWithVec, wallVec, 135.0f, glm::vec3(-20, 0, 20), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallUp", player, pScene, collidWithVec, wallVec, 0.0f, glm::vec3(0,0, 28), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallRight", player, pScene, collidWithVec, wallVec, 90.0f, glm::vec3(28, 0, 0), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallLeft", player, pScene, collidWithVec, wallVec, -90.0f, glm::vec3(-28, 0, 0), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallDownRight", player, pScene, collidWithVec, wallVec, -45.0f, glm::vec3(20, 0, -20), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallDownLeft", player, pScene, collidWithVec, wallVec, -135.0f, glm::vec3(-20, 0, -20), glm::vec3(25.0, 1.0, 0.3), false);
			setUpWall("WallDown", player, pScene, collidWithVec, wallVec, 0.0f, glm::vec3(0, 0, -28), glm::vec3(25.0, 1.0, 0.3), false);

			/*
			setUpWall("SupportWall1", player, pScene, collidWithVec, wallVec, 0.0f, glm::vec3(0, 0, 10), glm::vec3(10.0, 1.0, 0.3), true);
			setUpWall("SupportWall2", player, pScene, collidWithVec, wallVec, 0.0f, glm::vec3(0, 0, -10), glm::vec3(10.0, 1.0, 0.3), true);
			setUpWall("SupportWall3", player, pScene, collidWithVec, wallVec, 0.0f, glm::vec3(10, 0, 0), glm::vec3(10.0, 1.0, 0.3), true);
			setUpWall("SupportWall4", player, pScene, collidWithVec, wallVec, 0.0f, glm::vec3(-10, 0, 10), glm::vec3(10.0, 1.0, 0.3), true);
			*/
			bool* enemySwitchPressed = new bool(false);

			int startingEnemyAmount = 4; 
			ceateEnemies("AllEnemies", playerCube, pScene, startingEnemyAmount, collidWithVec, wallVec, enemySwitchPressed);

			registerEventListener(new SwitchListener("EnemySwitchListener", playerCube, pScene, enemySwitchPressed));

			//registerEventListener(new CollisionResponseListener("CollisionResponseListener", pScene), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });

			registerEventListener(new EventListenerGUI("GUI"), { veEvent::VE_EVENT_DRAW_OVERLAY });

			
			//setUpWall("supportWall1", player, pScene, collidWithVec, 0.0f, glm::vec3(-1, 0, 8), glm::vec3(8.0, 1.0, 0.3), true);

			//setUpWall("supportWall2", player, pScene, collidWithVec, 0.0f, glm::vec3(1, 0, 2), glm::vec3(8.0, 1.0, 0.3), true);




			/*
						VESceneNode* WallUp;
						VECHECKPOINTER(WallUp = getSceneManagerPointer()->loadModel("WallUp", "media/models/test/crate0", "cube.obj"));
						WallUp->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(25.0, 1.0, 1.0)));
						WallUp->multiplyTransform(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
						WallUp->multiplyTransform(glm::translate(glm::mat4(1.0f), player->getPosition() + glm::vec3(10, 0, 20)));
						pScene->addChild(WallUp);

						registerEventListener(new WallListener("WallUp", WallUp, glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)), collidWithVec), { veEvent::VE_EVENT_KEYBOARD, veEvent::VE_EVENT_FRAME_STARTED });
			*/
			//createLights(10, pScene );
			//VESceneNode *pSponza = m_pSceneManager->loadModel("Sponza", "models/sponza", "sponza.dae", aiProcess_FlipWindingOrder);
			//pSponza->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));

		};
	};
}


using namespace ve;

int main() {
	srand(time(NULL));
	std::mutex mtx;

	bool debug = false;
#ifdef  _DEBUG
	debug = true;
#endif

	MyVulkanEngine mve(false);	//enable or disable debugging (=callback, validation layers)

	mve.initEngine();
	mve.loadLevel(1);
	mve.run();

	return 0;
}

