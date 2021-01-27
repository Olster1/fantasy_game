

typedef enum {
	ENTITY_SHOW_HEALTH_BAR = 1 << 0,
} EntityFlags;

typedef struct {
	Array_Dynamic entities;

	Array_Dynamic entitiesToAddForFrame;
	Array_Dynamic entitiesToDeleteForFrame;

} EntityManager;	



typedef struct {
	EntityType type;

	u64 flags;

	//NOTE: If the entity is flipped on the x-Axis
	bool isFlipped; 

	bool isDead;

	//NOTE: If no animation, use this sprite
	Texture *sprite;

	V4 colorTint;

	EasyTransform T;
	EasyAnimation_Controller animationController;

	EasyRigidBody *rb;
	EasyCollider *collider;
	EasyCollider *collider1;

	float layer; //NOTE: zero for infront, +ve for more behind

	////////////////  Different entity sub types ////////////////
	float lifeSpanLeft;

	bool isDying;
	int health;
	int maxHealth;

	int itemCount;
	EntityType itemSpots[MAX_PLAYER_ITEM_COUNT];
	/////

	float healthBarTimer; 


} Entity;


typedef struct {
	EntityType type;
	V3 position;
	V3 dP;
} EntityToAdd;

static void initEntityManager(EntityManager *manager) {
	initArray(&manager->entities, Entity);

	initArray(&manager->entitiesToAddForFrame, EntityToAdd);
	initArray(&manager->entitiesToDeleteForFrame, int);
}

Entity *initEntity(EntityManager *manager, Animation *animation, V3 pos, V2 dim, V2 physicsDim, GameState *gameState, EntityType type, float inverse_weight, Texture *sprite, V4 colorTint, float layer, bool canCollide) {
	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entities);

	Entity *entity = (Entity *)arrayInfo.elm;

	easyMemory_zeroStruct(entity, Entity);

	float width = dim.x;
	float height = dim.y;

	entity->colorTint = colorTint;

	easyTransform_initTransform_withScale(&entity->T, pos, v3(width, height, 1), EASY_TRANSFORM_STATIC_ID);
	easyAnimation_initController(&entity->animationController);

	if(animation) {
		easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, animation, EASY_ANIMATION_PERIOD);	
	}

	entity->flags = 0;
	entity->isDead = false;

	entity->healthBarTimer = -1;

	entity->isDying = false;
	entity->itemCount = 0;
	
	entity->layer = layer;
	entity->sprite = sprite;
	entity->type = type;
	
	entity->maxHealth = 3;
	entity->health = entity->maxHealth;

	float gravityFactor = 120;
	if(type == ENTITY_SCENERY) 
	{ 
		gravityFactor = 0; 
	}

	if(type == ENTITY_SKELETON) {
		entity->flags |= (u64)ENTITY_SHOW_HEALTH_BAR;
	}

	bool isTrigger = false;

	float dragFactor = 0.12f;

	entity->collider1 = 0;

	if(type == ENTITY_PLAYER_PROJECTILE) { isTrigger = true; dragFactor = 0; }

	if(type == ENTITY_HEALTH_POTION_1) { isTrigger = false;  }

	if(canCollide) {
		// char string[256];
		// sprintf(string, "%f", gravityFactor);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, string);

		entity->rb = EasyPhysics_AddRigidBody(&gameState->physicsWorld, inverse_weight, 0, dragFactor, gravityFactor);
		entity->collider = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), isTrigger, v3(physicsDim.x, physicsDim.y, 0));
		
		if(type == ENTITY_HEALTH_POTION_1) { 
			//Add a trigger aswell
			entity->collider1 = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), true, v3(physicsDim.x, physicsDim.y, 0));
			entity->collider1->layer = EASY_COLLISION_LAYER_ITEMS;
		}


		switch(entity->type) {
			case ENTITY_SCENERY: {
				entity->collider->layer = EASY_COLLISION_LAYER_WORLD;
			} break;
			case ENTITY_WIZARD: {
				entity->collider->layer = EASY_COLLISION_LAYER_PLAYER;
			} break;
			case ENTITY_PLAYER_PROJECTILE: {
				entity->collider->layer = EASY_COLLISION_LAYER_PLAYER_BULLET;
			} break;
			case ENTITY_SKELETON: {
				entity->collider->layer = EASY_COLLISION_LAYER_ENEMIES;
			} break;
			case ENTITY_HEALTH_POTION_1: {
				entity->collider->layer = EASY_COLLISION_LAYER_ITEM_RIGID;
			} break;
		}

	}
	
	return entity;
}


typedef struct {
    Entity *e;
    EasyCollisionType collisionType;
    bool found;
} MyEntity_CollisionInfo;

static MyEntity_CollisionInfo MyEntity_hadCollisionWithType(EntityManager *manager, EasyCollider *collider, EntityType type, EasyCollisionType colType) {
    
    MyEntity_CollisionInfo result;
    result.found = false;
    
    for(int i = 0; i < collider->collisionCount && !result.found; ++i) {
        EasyCollisionInfo info = collider->collisions[i];

        if(info.type == colType) {
            int id = info.objectId; 
            for(int j = 0; j < manager->entities.count && !result.found; ++j) {
                Entity *e = (Entity *)getElement(&manager->entities, j);
                if(e) { //can be null
                    if(e->T.id == id && e->type == type) {
                        result.e = e;
                        result.collisionType = info.type;
                        result.found = true;
                        break;
                    }
                }
            }
        }
    } 
    
    return result;
}


////////////////////////////////////////////////////////////////////


void updateEntity(EntityManager *manager, Entity *entity, GameState *gameState, float dt, AppKeyStates *keyStates, EasyConsole *console, EasyCamera *cam, Entity *player, bool isPaused) {

	if(entity->type == ENTITY_WIZARD) {
		if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardIdle) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRun)){
			if(isDown(keyStates->gameButtons, BUTTON_LEFT) && !isPaused) {
				entity->rb->accumForce.x += -400;
			}

			if(isDown(keyStates->gameButtons, BUTTON_RIGHT) && !isPaused) {
				entity->rb->accumForce.x += 400;
			}


		}

		// char string[512];
		// sprintf(string, "is grounded");
		// easyConsole_addToStream(console, string);


		Animation *animToAdd = 0;

		if(entity->rb->dP.x < -1 || entity->rb->dP.x > 1) {
			if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardIdle)) {
				animToAdd = &gameState->wizardRun;
				
			}
		} else {
			if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRun)) {
				easyAnimation_emptyAnimationContoller(&entity->animationController, &gameState->animationFreeList);
				easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, &gameState->wizardIdle, EASY_ANIMATION_PERIOD);	
			}
		}

		if(wasPressed(keyStates->gameButtons, BUTTON_X) && !isPaused) {
			animToAdd = &gameState->wizardAttack;
		} else if(wasPressed(keyStates->gameButtons, BUTTON_Z)) {
			animToAdd = &gameState->wizardAttack2;
			if(gameState->playerHolding[0] != ENTITY_NULL) {
				switch(gameState->playerHolding[0]) {
					case ENTITY_HEALTH_POTION_1: {
						entity->health++;

						if(entity->health > entity->maxHealth) {
							entity->health = entity->maxHealth;
						}
						//EMPTY OUT THE SPOT
						gameState->playerHolding[0] = ENTITY_NULL;
					} break;
				}
			} 
		} 


		if(wasPressed(keyStates->gameButtons, BUTTON_2)) {
			animToAdd = &gameState->wizardHit;
		}

		if(wasPressed(keyStates->gameButtons, BUTTON_3)) {
			animToAdd = &gameState->wizardDeath;
		}

		

		if(wasPressed(keyStates->gameButtons, BUTTON_5)) {
			animToAdd = &gameState->wizardFall;
		}

		if(entity->rb->isGrounded && wasPressed(keyStates->gameButtons, BUTTON_SPACE) && !isPaused) {
			entity->rb->accumForceOnce.y += 70000;
			animToAdd = &gameState->wizardJump;
		}


		if(animToAdd) {

			if(animToAdd == &gameState->wizardAttack) {
				ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
				EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
				entityToAdd->type = ENTITY_PLAYER_PROJECTILE;
				entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0.0f, 0));

				easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);

				entityToAdd->dP.y = 0;
				if(entity->isFlipped) {
					entityToAdd->dP.x = -5;
				} else {
					entityToAdd->dP.x = 5;
				}
				
			}

			easyAnimation_emptyAnimationContoller(&entity->animationController, &gameState->animationFreeList);
			easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, animToAdd, EASY_ANIMATION_PERIOD);
			easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, &gameState->wizardIdle, EASY_ANIMATION_PERIOD);	
		}

		
	}

	if(entity->type == ENTITY_HEALTH_POTION_1) {
		if(entity->collider1->collisionCount > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	player->itemSpots[player->itemCount++] = ENTITY_HEALTH_POTION_1;

            	entity->isDead = true; //remove from entity list
            	
            }
		}
		
	}

	if(entity->type == ENTITY_SKELETON) {
		Animation *animToAdd = 0;

		// if(wasPressed(keyStates->gameButtons, BUTTON_SPACE)) {
		// 	animToAdd = &gameState->skeletonAttack;
		// }

		// if(wasPressed(keyStates->gameButtons, BUTTON_2)) {
		// 	animToAdd = &gameState->skeletonDeath;
		// }

		// if(wasPressed(keyStates->gameButtons, BUTTON_3)) {
		// 	animToAdd = &gameState->skeltonShield;
		// }

		// if(wasPressed(keyStates->gameButtons, BUTTON_4)) {
		// 	animToAdd = &gameState->skeltonHit;
		// }

		// if(wasPressed(keyStates->gameButtons, BUTTON_5)) {
		// 	animToAdd = &gameState->skeltonWalk;
		// }

		//NOTE: Check if a player bullet hit the skeleton 

		if(entity->collider->collisionCount > 0 && entity->health > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_PLAYER_PROJECTILE, EASY_COLLISION_ENTER);	
            if(info.found) {
            	easyConsole_addToStream(console, "skeleton hit");
            	animToAdd = &gameState->skeltonHit;	

            	entity->health--;

            	entity->healthBarTimer = 0;
            	
            }
		}

		if(!entity->isDying && entity->health == 0 && !animToAdd && !easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->skeltonHit)) {
			animToAdd = &gameState->skeletonDeath;
			entity->isDying = true;
		}


		if(animToAdd) {
			easyAnimation_emptyAnimationContoller(&entity->animationController, &gameState->animationFreeList);
			easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, animToAdd, EASY_ANIMATION_PERIOD);
			easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, &gameState->skeltonIdle, EASY_ANIMATION_PERIOD);	
		}

	}	

	if(entity->type == ENTITY_PLAYER_PROJECTILE) {
		
		if(entity->collider->collisionCount > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_SKELETON, EASY_COLLISION_ENTER);	
            if(info.found) {
            	easyConsole_addToStream(console, "skeleton hit 2");
            	
            }
		}

		entity->lifeSpanLeft -= dt;

		if(entity->lifeSpanLeft <= 0.0f) {
			entity->isDead = true;
		}
	}


	
	entity->T.pos.z = 0.1f*entity->layer;

	//NOTE: DRAWING HEALTH BAR OVER ENEMIES
	if(entity->flags & ENTITY_SHOW_HEALTH_BAR) {

		V3 worldP = easyTransform_getWorldPos(&player->T);

		V3 entP = easyTransform_getWorldPos(&entity->T);
		float distance = getLengthSqrV3(v3_minus(worldP, entP));

		if(distance < 4.0f || entity->healthBarTimer >= 0.0f) { //Show health bar
			float percent = (float)entity->health / (float)entity->maxHealth;

			V4 color = COLOR_GREEN;

			if(percent < 0.5f) {
				color = COLOR_RED;
			}

			float w = 0.8f; //game world meters
			float h = 0.1f; //game world meters

			entity->T.pos.z -= 0.05f;

			Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(w, h, 0)), v3_plus(entP, v3(0, 0.5f, 0)));

			setModelTransform(globalRenderGroup, T);
			renderDrawQuad(globalRenderGroup, COLOR_LIGHT_GREY);

			T.a.x = percent*w;
			T.d.z -= 0.05f;

			float overhang = w - percent*w;
			T.d.x -= 0.5f*overhang;			

			setModelTransform(globalRenderGroup, T);
			renderDrawQuad(globalRenderGroup, color);

			if(entity->healthBarTimer >= 0.0f) {
				entity->healthBarTimer += dt;
				
				if(entity->healthBarTimer > 2.0f) {
					entity->healthBarTimer = -1.0f;
				}	
			}
			
		}
	}
	////////////////////////////////

	Texture *sprite = entity->sprite;

	if(entity->type == ENTITY_HEALTH_POTION_1) {
		assert(entity->collider1);
		assert(entity->collider1->layer == EASY_COLLISION_LAYER_ITEMS);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, "Potion");
		assert(sprite);

		

		
	}

	if(!sprite) {
		char *animationFileName = easyAnimation_updateAnimation(&entity->animationController, &gameState->animationFreeList, dt);
		sprite = findTextureAsset(animationFileName);	
	
		if(entity->isDying && entity->type == ENTITY_SKELETON && !easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->skeletonDeath)) {
			//NOTE: Check if finished the dying animation, if so delete entity
			entity->isDead = true;

			//Add items skelton leaves behind
			ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
			EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
			entityToAdd->type = ENTITY_HEALTH_POTION_1;
			entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0.0f, 0));

			entityToAdd->dP.y = 10;
			entityToAdd->dP.x = randomBetween(-5, 5);
		}
	}
	

	

	Matrix4 T = easyTransform_getTransform(&entity->T);

	// char string[512];
	// sprintf(string, "size: %f %f", T.a.x, T.b.y);
	// easyConsole_addToStream(DEBUG_globalEasyConsole, string);


	//NOTE: We keep track of whether the entity is flipped since we only flip once we move fast a velocity threshold,
	//		otherwise entities default back to unflipped  
	if(entity->rb) {
		if(entity->rb->dP.x < -0.1) {
			entity->isFlipped = true;
		} else if(entity->rb->dP.x > 0.1) {
			entity->isFlipped = false;
		}
	}


	////////////////////
	//NOTE: Flip sprite
	if(entity->isFlipped) {
		//NOTE: flipSprite
		T.E_[0] *= -1;
		T.E_[1] *= -1;
		T.E_[2] *= -1;
	}

	/////////////////////

	setModelTransform(globalRenderGroup, T);
	renderDrawSprite(globalRenderGroup, sprite, entity->colorTint);
	// renderDrawQuad(globalRenderGroup, COLOR_RED);

	entity->T.pos.z = 0;
}