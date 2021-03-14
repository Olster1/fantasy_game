

typedef enum {
	ENTITY_SHOW_HEALTH_BAR = 1 << 0,
	ENTITY_SHOULD_SAVE = 1 << 1,
} EntityFlags;

typedef enum {
	ENTITY_SUB_TYPE_NONE = 0,
	ENTITY_SUB_TYPE_TORCH = 1 << 0,
	ENTITY_SUB_TYPE_ONE_WAY_UP_PLATFORM = 1 << 1,
	ENTITY_SUB_TYPE_SWORD = 2 << 1,
} SubEntityType;

typedef struct {
	EntityType type;

	u64 flags;


	//NOTE: This is for the redo undo system, and to save entities that shouldn't be deleted like the sword and shield
	bool isDeleted;

	//NOTE: If the entity is flipped on the x-Axis
	bool isFlipped; 

	bool isDead;

	int subEntityType;

	//NOTE: If no animation, use this sprite
	Texture *sprite;

	V4 colorTint;

	EasyTransform T;
	EasyAnimation_Controller animationController;

	EasyRigidBody *rb;
	EasyCollider *collider;
	EasyCollider *collider1;

	bool shieldInUse;
	bool staminaMaxedOut;

	float layer; //NOTE: zero for infront, +ve for more behind

	////////////////  Different entity sub types ////////////////
	float lifeSpanLeft;
	float maxLifeSpan;

	bool isDying;
	int health;
	int maxHealth;

	int itemCount;
	EntityType itemSpots[MAX_PLAYER_ITEM_COUNT];
	/////

	//Player stamina
	float stamina;
	float maxStamina;
	float staminaTimer;

	//For signs
	char *message;

	//For key prompts
	float tBob;

	EasyModel *model;

	float rotation;

	float healthBarTimer; 

	//For the audio checklist
	WavFile *audioFile;


} Entity;

typedef struct {
	Array_Dynamic entities;

	Array_Dynamic entitiesToAddForFrame;
	Array_Dynamic entitiesToDeleteForFrame;

	int lastEntityIndex;

	Entity *player;
} EntityManager;	

typedef struct {
	EntityType type;
	V3 position;
	V3 dP;
	EasyTransform *parentT;
	SubEntityType subType;
} EntityToAdd;

static void initEntityManager(EntityManager *manager) {
	initArray(&manager->entities, Entity);

	initArray(&manager->entitiesToAddForFrame, EntityToAdd);
	initArray(&manager->entitiesToDeleteForFrame, int);

	manager->lastEntityIndex = 0;
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


static inline void player_useAttackItem(EntityManager *manager, float damage, Entity *entity) {
	assert(entity->collider1->isTrigger);
    if(entity->collider1->collisionCount > 0) {
    	
    	easyConsole_addToStream(DEBUG_globalEasyConsole, "had collision");

        //Check if it got hurt 
        MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WEREWOLF, EASY_COLLISION_STAY);	
        if(info.found) {
        	easyConsole_addToStream(DEBUG_globalEasyConsole, "In bounds");

        	Entity *enemy = info.e;

        	easyConsole_addToStream(DEBUG_globalEasyConsole, "WEREWOLF GOT HURT");


        	V3 worldP = easyTransform_getWorldPos(&enemy->T);
        	V3 projectileP = easyTransform_getWorldPos(&entity->T);

        	//Knock the enemy back
        	V2 dir = normalizeV2(v2_minus(worldP.xy, projectileP.xy));
        	// enemy->rb->accumForceOnce.xy = v2_plus(enemy->rb->accumForceOnce.xy, v2_scale(100000, dir));
        	///

        	//Damage the enemy
        	if(enemy->health > 0.0f) {
        		enemy->health -= 1.0f;	
        	}

        	//See if the enemey is dead
        	if(enemy->health <= 0.0f) {
        		enemy->isDead = true;

        		//////////////////Release items /////////////////
        		//Add items skelton leaves behind
        		for(int i = 0; i < 10; ++i) {
        			// easyConsole_addToStream(DEBUG_globalEasyConsole, "creating potion");
	        		ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
	        		EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
	        		entityToAdd->type = ENTITY_HEALTH_POTION_1;
	        		entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0.0f, 0));

	        		entityToAdd->dP.y = 0;
	        		entityToAdd->dP.x = 0;//randomBetween(-5, 5);
	        	}
        		/////////////////////////////////////////
        	}
        }
	}
}



static inline void entity_useItem(EntityManager *manager, GameState *gameState, Entity *entity, EntityType type, int itemIndex) {

	/////////Play sounds to signify use of item
	int attackSoundIndex = randomBetween(0, 3);
	if(attackSoundIndex == 3) {
		attackSoundIndex = 2;
	}

	// playGameSound(&globalLongTermArena, gameState->playerAttackSounds[attackSoundIndex], 0, AUDIO_FOREGROUND);

	////////////////////

	switch(type) {
		case ENTITY_HEALTH_POTION_1: {
			entity->health++;

			if(entity->health > entity->maxHealth) {
				entity->health = entity->maxHealth;
			}
			//EMPTY OUT THE SPOT
			gameState->playerHolding[itemIndex] = ENTITY_NULL;
			
		} break;
		case ENTITY_STAMINA_POTION_1: {
			entity->stamina++;

			if(entity->stamina > entity->maxStamina) {
				entity->stamina = entity->maxStamina;
			}
			//EMPTY OUT THE SPOT
			gameState->playerHolding[itemIndex] = ENTITY_NULL;
			
		} break;
		case ENTITY_SWORD: {
			easyConsole_pushInt(DEBUG_globalEasyConsole, (int)entity->shieldInUse);
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Used Sword");
			if(entity->stamina >= 3.0f && !entity->shieldInUse) {

				player_useAttackItem(manager, 1, entity);

				 gameState->swordSwingTimer = 0;

				// ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
				// EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
				// entityToAdd->type = ENTITY_PLAYER_PROJECTILE;
				// entityToAdd->position = v3(0.0f, 1.0f, 0);
				// entityToAdd->parentT = &entity->T;
				// easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);
				// entityToAdd->dP.x = 0;
				// entityToAdd->dP.y = 0;
				// entityToAdd->subType = ENTITY_SUB_TYPE_SWORD;

				entity->stamina -= 3.0f;
				entity->staminaTimer = 0.0f;

				if(entity->stamina <= 0.0f) {
					entity->stamina = 0;
					entity->staminaMaxedOut = true;
				}
			}
		} break;
		case ENTITY_SHEILD: {
			//can't use shield when using the sword
			if(gameState->swordSwingTimer < 0.0f) {
				entity->shieldInUse = true; 
			}
		} break;
		default: {

		}
	}

	
}



Entity *initEntity(EntityManager *manager, Animation *animation, V3 pos, V2 dim, V2 physicsDim, GameState *gameState, EntityType type, float inverse_weight, Texture *sprite, V4 colorTint, float layer, bool canCollide) {
	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entities);

	manager->lastEntityIndex = arrayInfo.absIndex;

	Entity *entity = (Entity *)arrayInfo.elm;

	easyMemory_zeroStruct(entity, Entity);

	float width = dim.x;
	float height = dim.y;

	entity->colorTint = colorTint;

	easyTransform_initTransform_withScale(&entity->T, pos, v3(width, height, 1), EASY_TRANSFORM_STATIC_ID);
	easyAnimation_initController(&entity->animationController);

	if(DEBUG_ANGLE_ENTITY_ON_CREATION) {
		//NOTE: Have entities spun around the x axis 45 degrees for the top down effect
		entity->T.Q = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);
	}
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
	entity->rotation = 0;
	entity->shieldInUse = false;
	entity->isDeleted = false;
	entity->tBob = 0;

	entity->maxStamina = 10;
	entity->stamina = entity->maxStamina;

	entity->staminaTimer = -1.0f;

	entity->maxHealth = 10;
	entity->health = entity->maxHealth;

	float gravityFactor = gameState->gravityScale; //150
	if(type == ENTITY_SCENERY) 
	{ 
		gravityFactor = 0; 
	}

	if(type == ENTITY_SKELETON || type == ENTITY_WEREWOLF) {
		entity->flags |= (u64)ENTITY_SHOW_HEALTH_BAR;
	}

	if(type == ENTITY_PLAYER_PROJECTILE || type == ENTITY_HEALTH_POTION_1) {

	} else {
		entity->flags |= (u64)ENTITY_SHOULD_SAVE;
	}

	bool isTrigger = false;

	float dragFactor = 0.12f;

	entity->collider1 = 0;

	entity->subEntityType = (int)ENTITY_SUB_TYPE_NONE;

	if(type == ENTITY_PLAYER_PROJECTILE) { isTrigger = true; dragFactor = 0; gravityFactor = 0; }

	if(type == ENTITY_SWORD || type == ENTITY_SHEILD) { isTrigger = true; dragFactor = 0; gravityFactor = 0; }

	if(type == ENTITY_HEALTH_POTION_1) { isTrigger = false;  }

	if(type == ENITY_CHECKPOINT || type == ENITY_AUDIO_CHECKPOINT) { isTrigger = true; }

	if(type == ENTITY_TERRAIN) { canCollide = false; isTrigger = false; gravityFactor = 0; inverse_weight = 0; dragFactor = 0; }
	

	if(canCollide) {
		// char string[256];
		// sprintf(string, "%f", gravityFactor);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, string);

		entity->rb = EasyPhysics_AddRigidBody(&gameState->physicsWorld, inverse_weight, 0, dragFactor, gravityFactor);
		entity->collider = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), isTrigger, v3(physicsDim.x, physicsDim.y, 0));
		
		if(type == ENTITY_HEALTH_POTION_1 || type == ENTITY_SIGN || type == ENTITY_WEREWOLF || type == ENTITY_WIZARD) { 
			//Add a TRIGGER aswell
			entity->collider1 = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), true, v3(physicsDim.x, physicsDim.y, 0));
			entity->collider1->layer = EASY_COLLISION_LAYER_ITEMS;

			//FOR WIZARD CREATE THE HIT BOX WHEN ENTITIES ARE INSIDE IT 
			if(type == ENTITY_WIZARD) {
				float newHeight = entity->collider1->dim2f.y * 2.0f;
				float diff = newHeight - entity->collider1->dim2f.y;

				easyConsole_pushFloat(DEBUG_globalEasyConsole, entity->collider1->offset.y);
				//Offset the trigger in front of the player
				entity->collider1->offset.y += 2.5f;
				entity->collider1->layer = EASY_COLLISION_LAYER_PLAYER_BULLET;
			}

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
			case ENTITY_SHEILD:
			case ENTITY_SWORD: {
				entity->collider->layer = EASY_COLLISION_LAYER_ITEMS;
			} break;
			case ENTITY_SIGN:  {
				entity->collider->layer = EASY_COLLISION_LAYER_WORLD;
			} break;
			case ENTITY_HEALTH_POTION_1: {
				entity->collider->layer = EASY_COLLISION_LAYER_ITEM_RIGID;
			} break;
			case ENITY_AUDIO_CHECKPOINT: {
				entity->collider->layer = EASY_COLLISION_LAYER_PLAYER;
			} break;
			case ENITY_CHECKPOINT: {
				entity->collider->layer = EASY_COLLISION_LAYER_PLAYER;
			} break;
			case ENTITY_WEREWOLF: {
				entity->collider->layer = EASY_COLLISION_LAYER_ENEMIES;
			} break;
		}

	}
	
	return entity;
}



////////////////////////////////////////////////////////////////////


void updateEntity(EntityManager *manager, Entity *entity, GameState *gameState, float dt, AppKeyStates *keyStates, EasyConsole *console, EasyCamera *cam, Entity *player, bool isPaused) {

	if(entity->type == ENTITY_WIZARD) {

		if(!isPaused) {

			if(entity->staminaTimer >= 0.0f) {
				entity->staminaTimer += dt;

				if(entity->staminaTimer >= 1.0f) {
					entity->staminaTimer = -1.0f;
				}

			} else {
				entity->stamina += dt;	
			}
			

			if(entity->stamina > entity->maxStamina) {
				entity->stamina = entity->maxStamina;
				entity->staminaMaxedOut = false;
			}

		}

		if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardIdle) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRun) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardJump)){
			
			float walkModifier = 1;

			if(entity->shieldInUse) {
				walkModifier = 0.5f;
			}

			if(isDown(keyStates->gameButtons, BUTTON_LEFT) && !isPaused) {
				entity->rb->accumForce.x += -gameState->walkPower*walkModifier;
			}

			if(isDown(keyStates->gameButtons, BUTTON_RIGHT) && !isPaused) {
				entity->rb->accumForce.x += gameState->walkPower*walkModifier;
			}
			if(isDown(keyStates->gameButtons, BUTTON_UP) && !isPaused) {
				entity->rb->accumForce.y += gameState->walkPower*walkModifier;
			}

			if(isDown(keyStates->gameButtons, BUTTON_DOWN) && !isPaused) {
				entity->rb->accumForce.y += -gameState->walkPower*walkModifier;
			}


			// float angle = ATan2_0toTau(entity->rb->dP.y, entity->rb->dP.x);

			// angle -= 0.5f*PI32;
			// //NOTE(ollie): Wrap the angle so it moves from 0 -> Tau to -Pi -> PI
			// if(angle > PI32) {
			// 	angle = angle - TAU32;
			// }

			// entity->rotation = angle;

		}

		// char string[512];
		// sprintf(string, "is grounded");
		// easyConsole_addToStream(console, string);


		Animation *animToAdd = 0;

		if(entity->rb->dP.x < -1 || entity->rb->dP.x > 1 || entity->rb->dP.y < -1 || entity->rb->dP.y > 1) {
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

			// playGameSound(&globalLongTermArena, gameState->playerAttackSounds[attackSoundIndex], 0, AUDIO_FOREGROUND);

			if(gameState->playerHolding[1] != ENTITY_NULL) {
				EntityType type = gameState->playerHolding[1];
				entity_useItem(manager, gameState, entity, type, 1);
				
			} 



		} else if(wasPressed(keyStates->gameButtons, BUTTON_Z) && !isPaused) {
			// animToAdd = &gameState->wizardAttack2;

			if(gameState->playerHolding[0] != ENTITY_NULL) {
				EntityType type = gameState->playerHolding[0];
				entity_useItem(manager, gameState, entity, type, 0);
			} 
		} 

		if((wasReleased(keyStates->gameButtons, BUTTON_X) && gameState->playerHolding[1] == ENTITY_SHEILD) ||
		(wasReleased(keyStates->gameButtons, BUTTON_Z) && gameState->playerHolding[0] == ENTITY_SHEILD)) {
			entity->shieldInUse = false;
		}


		if(wasPressed(keyStates->gameButtons, BUTTON_2)) {
			// animToAdd = &gameState->wizardHit;
		}

		if(wasPressed(keyStates->gameButtons, BUTTON_3)) {
			// animToAdd = &gameState->wizardDeath;
		}

		

		if(wasPressed(keyStates->gameButtons, BUTTON_5)) {
			// animToAdd = &gameState->wizardFall;
		}

		if(entity->rb->isGrounded && wasPressed(keyStates->gameButtons, BUTTON_SPACE) && !isPaused) {
			entity->rb->accumForceOnce.y += gameState->jumpPower;
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

	if(entity->type == ENTITY_SWORD || entity->type == ENTITY_SHEILD) {
		entity->rotation += dt;

		if(entity->collider->collisionCount > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	player->itemSpots[player->itemCount++] = entity->type;
            	
            	playGameSound(&globalLongTermArena, gameState->successSound, 0, AUDIO_FOREGROUND);

            	entity->isDead = true; //remove from entity list
            	
            }
		}
	}

		if(entity->type == ENTITY_SIGN) {

			if(entity->collider1->collisionCount > 0) {

	            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
	            if(info.found) {

	            	if(gameState->gameModeType != GAME_MODE_READING_TEXT) {
	            		entity->tBob += dt;

	            		V3 position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 1.5f + 0.2f*sin(entity->tBob), 0));

	            		

	            		gameState->tempTransform.pos = position;
	            		float width = 2;
	            		float height = gameState->spacePrompt->aspectRatio_h_over_w*width;

	            		gameState->tempTransform.scale.xy = v2(width, height);

	            		setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
	            		renderDrawSprite(globalRenderGroup, gameState->spacePrompt, COLOR_WHITE);
	            		
	            		
	            		if(wasPressed(keyStates->gameButtons, BUTTON_SPACE)) 
	            		{
	            			gameState->gameModeType = GAME_MODE_READING_TEXT;
	            			gameState->currentTalkText = entity->message;
	            			easyConsole_addToStream(DEBUG_globalEasyConsole, entity->message);
	            		}	
	            	} 
	            }
			}
		}

	if(entity->type == ENTITY_WEREWOLF) {
		V3 worldP = easyTransform_getWorldPos(&player->T);
		V3 entP = easyTransform_getWorldPos(&entity->T);

		V3 diff = v3_minus(worldP, entP);

		V2 dir = normalizeV2(diff.xy);

		if(getLength(diff.xy) < 10) {
			//Move towards the player
			// entity->rb->dP.xy = v2_scale(10.0f, dir);  
		}


		//WEREWOLF HURTING THE PLAYER//
		if(entity->collider1->collisionCount > 0) {
			assert(entity->collider1->isTrigger);

            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	float maxReboundForce = 100000;
            	float reboundForce = maxReboundForce;

            	float damage = 1.0f;

            	if(info.e->shieldInUse) {
            		reboundForce = maxReboundForce*0.1f;
            		damage = 0.0f;//0.5f;

            		if(info.e->stamina > 0.0f && !info.e->staminaMaxedOut) {
            			info.e->stamina -= 1.0f;
            			info.e->staminaTimer = 0; 

            			if(info.e->stamina < 0.0f) {
            				info.e->stamina = 0.0f;
            				info.e->staminaMaxedOut = true;
            			}	
            		} else {
            			damage = 0.5f;
            			reboundForce = 0.5f*maxReboundForce;
            		}
            		
            	}

            	info.e->rb->accumForceOnce.xy = v2_plus(info.e->rb->accumForceOnce.xy, v2_scale(reboundForce, dir));

            	if(info.e->health > 0.0f) {
            		info.e->health -= damage;	
            		easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);
            	}

            	if(info.e->health <= 0.0f) {
            		gameState->gameModeType = GAME_MODE_GAME_OVER;
            	}
            }
        }
        //////////////////////////////////////////////

        
		
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
			
		// char str[256];
		// sprintf(str, "%f", entity->rb->inverseWeight);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, str);

 		if(entity->collider->collisionCount > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_SKELETON, EASY_COLLISION_ENTER);	
            if(info.found) {
            	easyConsole_addToStream(console, "skeleton hit 2");
            	
            }

            info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WEREWOLF, EASY_COLLISION_ENTER);	
            if(info.found) {
            	easyConsole_addToStream(console, "werewolf hit");
            }
		}



		entity->lifeSpanLeft -= dt;

		

		if(entity->lifeSpanLeft <= 0.0f) {
			entity->isDead = true;
		}
	}

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
	if(!sprite && !entity->model && entity->type != ENTITY_TERRAIN && entity->type != ENTITY_SWORD && entity->type != ENTITY_SHEILD) {
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
	} else if(!DEBUG_DRAW_SCENERY_TEXTURES || entity->type == ENTITY_TERRAIN) {
		sprite = 0;
	}
	

	Quaternion Q = entity->T.Q;
	Quaternion Q1 = eulerAnglesToQuaternion(0, 0, entity->rotation);

	entity->T.Q = quaternion_mult(Q, Q1);

	Matrix4 T = easyTransform_getTransform(&entity->T);

	entity->T.Q = Q;

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
		// T.E_[0] *= -1;
		// T.E_[1] *= -1;
		// T.E_[2] *= -1;
	}

	/////////////////////

	setModelTransform(globalRenderGroup, T);
	
	if(sprite && DEBUG_DRAW_SCENERY_TEXTURES) { renderSetShader(globalRenderGroup, &glossProgram); renderDrawSprite(globalRenderGroup, sprite, entity->colorTint); }

	// if(entity->model) {
	// 	renderSetShader(globalRenderGroup, &phongProgram);
	// 	// easyConsole_addToStream(DEBUG_globalEasyConsole, "HAS MODEL");
	// 	renderModel(globalRenderGroup, entity->model, entity->colorTint);
	// }
	

	// renderDrawQuad(globalRenderGroup, COLOR_RED);

	//Reset for collision
}




//Init entity types

static Entity *initScenery_noRigidBody(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
    return initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, false);
}

static Entity *initScenery_withRigidBody(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	return initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, true);
}

static Entity *initWizard(GameState *gameState, EntityManager *manager, V3 worldP) {
	 return initEntity(manager, &gameState->wizardIdle, worldP, v2(2.4f, 2.0f), v2(0.2f, 0.25f), gameState, ENTITY_WIZARD, gameState->inverse_weight, 0, COLOR_WHITE, 0, true);
}
static Entity *initSkeleton(GameState *gameState, EntityManager *manager, V3 worldP) {
	return initEntity(manager, &gameState->skeltonIdle, worldP, v2(2.5f, 2.5f), v2(0.25f, 0.15f), gameState, ENTITY_SKELETON, gameState->inverse_weight, 0, COLOR_WHITE, 1, true);
}

static Entity *initWerewolf(GameState *gameState, EntityManager *manager, V3 worldP) {
	return initEntity(manager, &gameState->werewolfIdle, worldP, v2(2.5f, 2.5f), v2(0.5f, 0.25f), gameState, ENTITY_WEREWOLF, gameState->inverse_weight, 0, COLOR_WHITE, 1, true);
}



static Entity *initTorch(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e  = initEntity(manager, &gameState->torchAnimation, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, -1, false);
	e->subEntityType |= (int)ENTITY_SUB_TYPE_TORCH;

	return e;
}
				
static Entity *initOneWayPlatform(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	Entity *e  = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, true);
	e->subEntityType |= (int)ENTITY_SUB_TYPE_ONE_WAY_UP_PLATFORM;

	assert(e->collider);
	e->collider->layer = EASY_COLLISION_LAYER_PLATFORM_ONE_WAY_UP;

	return e;
} 

static Entity *initCheckPoint(GameState *gameState, EntityManager *manager, V3 worldP) {
	return initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENITY_CHECKPOINT, 0, &globalWhiteTexture, COLOR_BLUE, -1, true);
}

static Entity *initTerrain(GameState *gameState, EntityManager *manager, V3 worldP) {
	return initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_TERRAIN, 0, &globalWhiteTexture, COLOR_BLUE, -1, true);
}

static Entity *initAudioCheckPoint(GameState *gameState, EntityManager *manager, V3 worldP) {
	return initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENITY_AUDIO_CHECKPOINT, 0, &globalWhiteTexture, COLOR_BLUE, -1, true);
}

static Entity *initSword(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SWORD, 0, 0, COLOR_WHITE, -1, true);
	e->model = findModelAsset_Safe("sword.obj"); 
	return e;
}

static Entity *initSheild(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SHEILD, 0, 0, COLOR_WHITE, -1, true);
	e->model = findModelAsset_Safe("shield.obj"); 
	e->T.Q = eulerAnglesToQuaternion(0, -0.5f*PI32, 0);
	return e;
}

static Entity *init3dModel(GameState *gameState, EntityManager *manager, V3 worldP, EasyModel *model) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, -1, true);
	e->model = model; 
	e->sprite = 0;
	return e;
}

static Entity *initSign(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SIGN, 0, findTextureAsset("sign.png"), COLOR_WHITE, -1, true);
	e->message = easyString_copyToHeap("Empty");
	return e;
}

static void entityManager_emptyEntityManager(EntityManager *manager, EasyPhysics_World *physicsWorld) {
	easyArray_clear(&manager->entitiesToDeleteForFrame);
	easyArray_clear(&manager->entities);
	easyArray_clear(&manager->entitiesToAddForFrame);

	EasyPhysics_emptyPhysicsWorld(physicsWorld);
	
	manager->lastEntityIndex = 0;
	manager->player = 0;
}