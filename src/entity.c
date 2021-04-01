

typedef enum {
	ENTITY_SHOW_HEALTH_BAR = 1 << 0,
	ENTITY_SHOULD_SAVE = 1 << 1,
	ENTITY_SHOULD_NOT_RENDER = 1 << 2,
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

	bool isSwimming;

	//FOr empty triggers that do stuff
	EntityTriggerType triggerType;
	char *levelToLoad; //for triggers that load levels

	//Location Sound type to retrieve string
	EntityLocationSoundType locationSoundType;

	///NOTE: CHest information
	EntityType chestContains;
	int itemCount;
	bool chestIsOpen;
	/////

	//For entity creator
	float rateOfCreation;
	float timeSinceLastCreation;
	EntityType typeToCreate;
	/////////////////////////////


	int subEntityType;

	//NOTE: If no animation, use this sprite
	Texture *sprite;

	V4 colorTint;

	EasyTransform T;
	EasyAnimation_Controller animationController;

	EasyRigidBody *rb;
	EasyCollider *collider;
	//Four colliders for the rock
	EasyCollider *collider1;
	EasyCollider *collider2;
	EasyCollider *collider3;
	EasyCollider *collider4;

	bool shieldInUse;
	bool staminaMaxedOut;

	float flashHurtTimer;

	float layer; //NOTE: zero for infront, +ve for more behind

	////////////////  Different entity sub types ////////////////
	float lifeSpanLeft;
	float maxLifeSpan;

	bool isDying;
	int health;
	int maxHealth;

	
	/////

	float lightIntensity;
	V3 lightColor;


	bool renderFirstPass; //Render before entities like terrain so the doesn't affect the depth buffer ordering 

	PlayingSound *currentSound;

	//Player stamina
	float stamina;
	float maxStamina;
	float staminaTimer;

	//For NPCs and signs
	DialogInfoType dialogType;

	//For key prompts
	float tBob;

	EasyModel *model;

	float rotation;

	float healthBarTimer; 

	//For the audio checklist
	char *audioFile;


} Entity;


typedef struct {
	EntityType type;
	V3 position;
	V3 dP;
	EasyTransform *parentT;
	SubEntityType subType;
} EntityToAdd;

typedef struct {
	float aliveTimer;
	char *str;
	V3 pos;
} Entity_DamageNumber;

static void initEntityManager(EntityManager *manager) {
	initArray(&manager->entities, Entity);

	initArray(&manager->entitiesToAddForFrame, EntityToAdd);
	initArray(&manager->entitiesToDeleteForFrame, int);

	initArray(&manager->damageNumbers, Entity_DamageNumber);

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

static void renderKeyPromptHover(GameState *gameState, Texture *keyTexture, Entity *entity, float dt, bool useScaleY) {
	entity->tBob += dt;

	V3 scale = easyTransform_getWorldScale(&entity->T);

	float sy = 0.11f;
	if(useScaleY) {
		sy = scale.y + 0.5f;
	}

	V3 position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0, -(sy + 0.1f*sin(entity->tBob))));

	gameState->tempTransform.pos = position;
	float width = 2;
	float height = keyTexture->aspectRatio_h_over_w*width;

	gameState->tempTransform.scale.xy = v2(width, height);

	setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
	renderDrawSprite(globalRenderGroup, keyTexture, COLOR_WHITE);

}

static void addItemToPlayer(GameState *state, EntityType t, int numToAdd, bool isDisposable) {
	ItemInfo *info = 0;

	for(int i = 0; i < state->itemCount && !info; ++i) {
		if(state->itemSpots[i].type == t) {
			info = &state->itemSpots[i];
			break;
		}
	}

	// if(!info) {
	// 	//look for empty spot
	// 	for(int i = 0; i < state->itemCount && !info; ++i) {
	// 		if(state->itemSpots[i].count == 0) {
	// 			info = &state->itemSpots[i];
	// 			break;
	// 		}
	// 	}
	// }

	if(!info) {
		//add to end
		assert(state->itemCount < arrayCount(state->itemSpots));
		info = state->itemSpots + state->itemCount++;
	}

	info->count += numToAdd;
	info->isDisposable = isDisposable;
	info->type = t; 
}


static inline void createDamageNumbers(EntityManager *manager, float value, V3 position) {

	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->damageNumbers);
	Entity_DamageNumber *number = (Entity_DamageNumber *)arrayInfo.elm;

    number->aliveTimer = 0;
    number->pos = position;
    	
    bool hasFractional = (((float)value - ((int)value)) > 0.0f);

    char *resultStr = 0;

    if(hasFractional) {
    	resultStr = easy_createString_printf(&globalPerFrameArena, "%.1f", (float)value);
    } else {
    	resultStr = easy_createString_printf(&globalPerFrameArena, "%d", (int)value);
    }
    
    number->str = easyString_copyToHeap(resultStr);


}


static inline float findMaxStamina(EntityType type) {

	if(type == ENTITY_WEREWOLF) { return 3; }

	if(type == ENTITY_WIZARD) { return 10; }

	return 10.0f; //default value
}

static inline float findMaxHealth(EntityType type) {

	if(type == ENTITY_WEREWOLF) { return 10; }

	if(type == ENTITY_WIZARD) { return 10; }

	return 10.0f; //default value
}


static inline float findDamage(EntityType type) {

	if(type == ENTITY_SWORD) { return 1; }

	if(type == ENTITY_WEREWOLF) { return 5; }

	return 1.0f; //default value
}

static inline void player_useAttackItem(GameState *gameState, EntityManager *manager, float damage, Entity *entity) {
	assert(entity->collider1->isTrigger);
    if(entity->collider1->collisionCount > 0) {
    	
    	easyConsole_addToStream(DEBUG_globalEasyConsole, "had collision");

        //Check if it got hurt 
        MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WEREWOLF, EASY_COLLISION_STAY);	
        if(info.found) {
        	easyConsole_addToStream(DEBUG_globalEasyConsole, "In bounds");

        	Entity *enemy = info.e;

        	easyConsole_addToStream(DEBUG_globalEasyConsole, "WEREWOLF GOT HURT");


        	enemy->flashHurtTimer = 0;
        	enemy->healthBarTimer = 0;

        	V3 worldP = easyTransform_getWorldPos(&enemy->T);
        	V3 projectileP = easyTransform_getWorldPos(&entity->T);

        	//Knock the enemy back
        	V2 dir = normalizeV2(v2_minus(worldP.xy, projectileP.xy));
        	enemy->rb->accumForceOnce.xy = v2_plus(enemy->rb->accumForceOnce.xy, v2_scale(gameState->werewolf_knockback_distance, dir));
        	///

        	//Damage the enemy
        	if(enemy->health > 0.0f) {
        		enemy->health -= damage;

        		createDamageNumbers(manager, damage, v3_plus(worldP, v3(0, 0, -1.0f)));	
        	}

        	//See if the enemey is dead
        	if(enemy->health <= 0.0f) {
        		enemy->isDead = true;

        		//////////////////Release items /////////////////
        		//Add items skelton leaves behind
        		// for(int i = 0; i < 10; ++i) {
        		// 	// easyConsole_addToStream(DEBUG_globalEasyConsole, "creating potion");
	        	// 	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
	        	// 	EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
	        	// 	entityToAdd->type = ENTITY_HEALTH_POTION_1;
	        	// 	entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0.0f, 0));

	        	// 	entityToAdd->dP.y = 0;
	        	// 	entityToAdd->dP.x = 0;//randomBetween(-5, 5);
	        	// }
        		/////////////////////////////////////////
        	}
        }
	}
}



static inline void entity_useItem(EntityManager *manager, GameState *gameState, Entity *entity, EntityType type, int itemIndex, float dt) {

	assert(gameState->playerHolding[itemIndex]);
	if(gameState->playerHolding[itemIndex]->isDisposable && gameState->playerHolding[itemIndex]->count == 0) {
		return;
	}

	/////////Play sounds to signify use of item
	int attackSoundIndex = randomBetween(0, 3);
	if(attackSoundIndex == 3) {
		attackSoundIndex = 2;
	}

	// playGameSound(&globalLongTermArena, gameState->playerAttackSounds[attackSoundIndex], 0, AUDIO_FOREGROUND);

	////////////////////

	bool drawParticleSystem = false;
	switch(type) {
		case ENTITY_HEALTH_POTION_1: {
			entity->health++;

			if(entity->health > entity->maxHealth) {
				entity->health = entity->maxHealth;
			}

			drawParticleSystem = true;
			
			
		} break;
		case ENTITY_STAMINA_POTION_1: {
			entity->stamina++;

			if(entity->stamina > entity->maxStamina) {
				entity->stamina = entity->maxStamina;
			}
			
			drawParticleSystem = true;

			
			
		} break;
		case ENTITY_SWORD: {
			easyConsole_pushInt(DEBUG_globalEasyConsole, (int)entity->shieldInUse);
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Used Sword");
			if(entity->stamina >= 3.0f && !entity->shieldInUse) {

				player_useAttackItem(gameState, manager, findDamage(type), entity);

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

	if(drawParticleSystem) {
		Reactivate(&gameState->playerUseItemParticleSystem);
		playGameSound(&globalLongTermArena, gameState->bubbleSound, 0, AUDIO_FOREGROUND);
	}

	if(gameState->playerHolding[itemIndex]->isDisposable) {
		gameState->playerHolding[itemIndex]->count--;
	}

	
}


static void entityManager_emptyEntityManager(EntityManager *manager, EasyPhysics_World *physicsWorld) {
	easyArray_clear(&manager->entitiesToDeleteForFrame);
	easyArray_clear(&manager->entities);
	easyArray_clear(&manager->entitiesToAddForFrame);
	easyArray_clear(&manager->damageNumbers);

	EasyPhysics_emptyPhysicsWorld(physicsWorld);
	
	manager->lastEntityIndex = 0;
	manager->player = 0;
}



typedef struct {
	GameState *gameState;
	EntityManager *manager;	
	char *sceneToLoad;
	EditorState *editorState;
} EntityLoadSceneData;

static void loadScene(void *data_) {
	EntityLoadSceneData *data = (EntityLoadSceneData *)data_;

	if(gameScene_doesSceneExist(data->sceneToLoad)) {
		letGoOfSelectedEntity(data->editorState);

		entityManager_emptyEntityManager(data->manager, &data->gameState->physicsWorld);

		gameScene_loadScene(data->gameState, data->manager, data->sceneToLoad);

		
	}
	data->gameState->gameIsPaused = false;	

	free(data_);
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

	entity->rateOfCreation = 3.f;
	entity->typeToCreate = ENTITY_NULL;
	entity->healthBarTimer = -1;

	entity->isDying = false;
	
	entity->layer = layer;
	entity->sprite = sprite;
	entity->type = type;
	entity->rotation = 0;
	entity->shieldInUse = false;
	entity->isDeleted = false;
	entity->tBob = 0;
	entity->chestIsOpen = false;

	entity->lightIntensity = 2.0f;
	entity->lightColor = v3(1, 0.5f, 0);

	entity->flashHurtTimer = -1.0f;

	entity->maxStamina = findMaxStamina(type);
	entity->stamina = entity->maxStamina;

	entity->staminaTimer = -1.0f;
	entity->renderFirstPass = false;

	entity->maxHealth = findMaxHealth(type);
	entity->health = entity->maxHealth;

	float gravityFactor = gameState->gravityScale; //150
	if(type == ENTITY_SCENERY) 
	{ 
		gravityFactor = 0; 
	}

	if(type == ENTITY_SKELETON || type == ENTITY_WEREWOLF) {
		entity->flags |= (u64)ENTITY_SHOW_HEALTH_BAR;
	}

	if(type == ENTITY_PLAYER_PROJECTILE || type == ENTITY_HEALTH_POTION_1 || type == ENTITY_SEAGULL) {

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
	
	if(type == ENTITY_TRIGGER) { isTrigger = true; gravityFactor = 0; inverse_weight = 0; dragFactor = 0;} 


	if(type == ENTITY_SEAGULL) { gravityFactor = 0; inverse_weight = 1.f / 20.0f; dragFactor = 0; }

	if(canCollide) {
		// char string[256];
		// sprintf(string, "%f", gravityFactor);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, string);

		entity->rb = EasyPhysics_AddRigidBody(&gameState->physicsWorld, inverse_weight, 0, dragFactor, gravityFactor);
		entity->collider = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), isTrigger, v3(physicsDim.x, physicsDim.y, 0));
		
		if(type == ENTITY_HEALTH_POTION_1 || type == ENTITY_SIGN || type == ENTITY_WEREWOLF || type == ENTITY_WIZARD || type == ENTITY_HORSE || type == ENTITY_CHEST || type == ENTITY_HOUSE) { 
			//Add a TRIGGER aswell
			entity->collider1 = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), true, v3(physicsDim.x, physicsDim.y, 0));
			entity->collider1->layer = EASY_COLLISION_LAYER_ITEMS;

			//FOR WIZARD CREATE THE HIT BOX WHEN ENTITIES ARE INSIDE IT 
			if(type == ENTITY_WIZARD) {
				float newHeight = entity->collider1->dim2f.y * 2.0f;
				float diff = newHeight - entity->collider1->dim2f.y;

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
			case ENTITY_TRIGGER:
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
			case ENTITY_SEAGULL: {
				entity->collider->layer = EASY_COLLISION_LAYER_NULL;
			} break;
		}

	}
	
	return entity;
}



////////////////////////////////////////////////////////////////////


void updateEntity(EntityManager *manager, Entity *entity, GameState *gameState, float dt, AppKeyStates *keyStates, EasyConsole *console, EasyCamera *cam, Entity *player, bool isPaused, V3 cameraZ_axis, EasyTransitionState *transitionState, EasySound_SoundState *soundState, EditorState *editorState) {


	if(!isPaused && !entity->isSwimming) {

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


	if(entity->type == ENTITY_WIZARD) {

		//Update listener position which is the wizard
		easySound_updateListenerPosition(soundState, easyTransform_getWorldPos(&entity->T));

		Animation *animToAdd = 0;

		

		Animation *idleAnimation = 0;
		// if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardIdle) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRun) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardBottom) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRight) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardLeft) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardJump))
		{
			
			float walkModifier = 1;

			if(isDown(keyStates->gameButtons, BUTTON_SHIFT)) {
				walkModifier = 2.5f;
			}

			if(entity->shieldInUse) {
				walkModifier = 0.5f;
			}

			float staminaFactor = 0.01f;

			if(isDown(keyStates->gameButtons, BUTTON_LEFT) && !isPaused) {
				entity->rb->accumForce.x += -gameState->walkPower*walkModifier;

				if(entity->isSwimming) {
					animToAdd = &gameState->wizardSwimLeft;
				} else {
					animToAdd = &gameState->wizardLeft;
					idleAnimation = &gameState->wizardIdleLeft;	
				}
				
				staminaFactor = 0.1f;
			}

			if(isDown(keyStates->gameButtons, BUTTON_RIGHT) && !isPaused) {
				entity->rb->accumForce.x += gameState->walkPower*walkModifier;
				if(entity->isSwimming) {
					animToAdd = &gameState->wizardSwimRight;
				} else {
					animToAdd = &gameState->wizardRight;
					idleAnimation = &gameState->wizardIdleRight;
				}
				staminaFactor = 0.1f;
			}
			if(isDown(keyStates->gameButtons, BUTTON_UP) && !isPaused) {
				entity->rb->accumForce.y += gameState->walkPower*walkModifier;
				if(entity->isSwimming) {
					animToAdd = &gameState->wizardSwimUp;
				} else {
					animToAdd = &gameState->wizardForward;
					idleAnimation = &gameState->wizardIdleForward;
				}
				staminaFactor = 0.1f;
			}

			if(isDown(keyStates->gameButtons, BUTTON_DOWN) && !isPaused) {
				entity->rb->accumForce.y += -gameState->walkPower*walkModifier;

				if(entity->isSwimming) {
					animToAdd = &gameState->wizardSwimDown;
				} else {
					animToAdd = &gameState->wizardBottom;
					idleAnimation = &gameState->wizardIdleBottom;
				}
				staminaFactor = 0.1f;
			}

			//decrement stamina while swimming
			if(entity->isSwimming) {
				entity->stamina -= staminaFactor*dt;

				if(entity->stamina < 0) {
					entity->stamina = 0;
				}
			}
			// entity->rotation = angle;

		}

		// char string[512];
		// sprintf(string, "is grounded");
		// easyConsole_addToStream(console, string);


		

		// if(entity->rb->dP.x < -1 || entity->rb->dP.x > 1 || entity->rb->dP.y < -1 || entity->rb->dP.y > 1) {
		// 	if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardIdle)) {
		// 		// animToAdd = &gameState->wizardRun;
				
		// 	}
		// } else {
		// 	if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRun)) {
		// 		// easyAnimation_emptyAnimationContoller(&entity->animationController, &gameState->animationFreeList);
		// 		// easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, &gameState->wizardIdle, EASY_ANIMATION_PERIOD);	
		// 	}
		// }

		{
			// float angle = ATan2_0toTau(entity->rb->dP.y, entity->rb->dP.x);

			// angle -= 0.5f*PI32;
			// // //NOTE(ollie): Wrap the angle so it moves from 0 -> Tau to -Pi -> PI

			// if(angle < -0.25f*PI32) {
			// 	;
			// }

			// float bestValue = INFINITY_VALUE;

			// float values[] = { 0, 0.5f*PI32, PI32, 1.5f*PI32 };

			//  easyConsole_pushFloat(DEBUG_globalEasyConsole, angle);

			// for(int i = 0; i < arrayCount(values); ++i) {
			// 	float value = absVal(angle - values[i]);

			// 	if(value < bestValue) {
			// 		bestValue = value;
			// 		if(i == 0) { animToAdd = &gameState->wizardRun;  }
			// 		if(i == 1) { animToAdd = &gameState->wizardLeft;  }
			// 		if(i == 2) { animToAdd = &gameState->wizardBottom; }
			// 		if(i == 3) { animToAdd = &gameState->wizardRight; }
			// 	}
			// }


		}

		if(wasPressed(keyStates->gameButtons, BUTTON_X) && !isPaused) {

			// playGameSound(&globalLongTermArena, gameState->playerAttackSounds[attackSoundIndex], 0, AUDIO_FOREGROUND);

			if(gameState->playerHolding[1]) {
				ItemInfo *info = gameState->playerHolding[1];
				entity_useItem(manager, gameState, entity, info->type, 1, dt);
				
			} 



		} else if(wasPressed(keyStates->gameButtons, BUTTON_Z) && !isPaused) {
			// animToAdd = &gameState->wizardAttack2;

			if(gameState->playerHolding[0]) {
				ItemInfo *info = gameState->playerHolding[0];
				entity_useItem(manager, gameState, entity, info->type, 0, dt);
			} 
		} 

		// if((wasReleased(keyStates->gameButtons, BUTTON_X) && gameState->playerHolding[1] && gameState->playerHolding[1]->type == ENTITY_SHEILD) ||
		// (wasReleased(keyStates->gameButtons, BUTTON_Z) && gameState->playerHolding[0] == ENTITY_SHEILD)) {
		// 	entity->shieldInUse = false;
		// }


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

		//Add collect item animation
		if(gameState->gameModeType == GAME_MODE_ITEM_COLLECT) {
			animToAdd = &gameState->wizardGetItem;
		}

		if(animToAdd) {

			// if(animToAdd == &gameState->wizardAttack) {
			// 	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
			// 	EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
			// 	entityToAdd->type = ENTITY_PLAYER_PROJECTILE;
			// 	entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0.0f, 0));

			// 	easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);

			// 	entityToAdd->dP.y = 0;
			// 	if(entity->isFlipped) {
			// 		entityToAdd->dP.x = -5;
			// 	} else {
			// 		entityToAdd->dP.x = 5;
			// 	}
				
			// }

			if(!easyAnimation_getCurrentAnimation(&entity->animationController, animToAdd)) {
				easyAnimation_emptyAnimationContoller(&entity->animationController, &gameState->animationFreeList);
				easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, animToAdd, EASY_ANIMATION_PERIOD);	
			}
			
			if(idleAnimation) {
				easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, idleAnimation, EASY_ANIMATION_PERIOD);		
			}

			
		}

		
	}

	if(entity->type == ENTITY_HEALTH_POTION_1) {
		if(entity->collider1->collisionCount > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	gameState->itemSpots[gameState->itemCount++].type = ENTITY_HEALTH_POTION_1;

            	entity->isDead = true; //remove from entity list
            	
            }
		}
		
	}

	if(entity->type == ENTITY_HORSE) {
		if(entity->collider1->collisionCount > 0) {

            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	if(entity->currentSound) {
            		easySound_endSound(entity->currentSound);
            	}	
            	WavFile *s = easyAudio_findSound("horse.wav");
            	entity->currentSound = playGameSound(&globalLongTermArena, s, 0, AUDIO_FOREGROUND);
            	
            }

            info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
            if(info.found) {
            	if(wasPressed(keyStates->gameButtons, BUTTON_SPACE)) {
            		// entity->T.parent = &manager->player->T;
            	}
            }
        }

	}

	if(entity->type == ENTITY_SWORD || entity->type == ENTITY_SHEILD) {
		entity->rotation += dt;

		if(entity->collider->collisionCount > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	gameState->itemSpots[gameState->itemCount++].type = entity->type;
            	
            	playGameSound(&globalLongTermArena, gameState->successSound, 0, AUDIO_FOREGROUND);

            	entity->isDead = true; //remove from entity list
            	
            }
		}
	}


	

	if(entity->type == ENTITY_HOUSE) {
		if(entity->collider1->collisionCount > 0) {

            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
            if(info.found) {

            	bool canInteractWith = gameState->gameModeType != GAME_MODE_READING_TEXT && !EasyTransition_InTransition(transitionState);

            	if(canInteractWith) {
            		// renderKeyPromptHover(gameState, gameState->spacePrompt, entity, dt, false);
            		
            		if(isDown(keyStates->gameButtons, BUTTON_UP)) {
            			//GO inside the house
            			playGameSound(&globalLongTermArena, gameState->doorSound, 0, AUDIO_FOREGROUND);

            			EntityLoadSceneData *loadSceneData = (EntityLoadSceneData *)easyPlatform_allocateMemory(sizeof(EntityLoadSceneData), EASY_PLATFORM_MEMORY_ZERO);
            			loadSceneData->gameState = gameState;
            			loadSceneData->manager = manager;
            			loadSceneData->sceneToLoad = entity->levelToLoad;
            			loadSceneData->editorState = editorState;


            			EasySceneTransition *transition = EasyTransition_PushTransition(transitionState, loadScene, loadSceneData, EASY_TRANSITION_CIRCLE_N64);
            			gameState->gameIsPaused = true;	
            		}
        			
            	}
            }
        }
	}


		if(entity->type == ENTITY_SIGN || entity->type == ENTITY_CHEST) {
			if(entity->collider1->collisionCount > 0) {

	            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
	            if(info.found) {

	            	bool canInteractWith = gameState->gameModeType != GAME_MODE_READING_TEXT;

	            	if(entity->type == ENTITY_CHEST && entity->chestIsOpen) {
	            		canInteractWith = false;
	            	}
	            	
	            	if(canInteractWith) {
	            		renderKeyPromptHover(gameState, gameState->spacePrompt, entity, dt, true);
	            		
	            		
	            		if(wasPressed(keyStates->gameButtons, BUTTON_SPACE)) 
	            		{	
	            			
	            			if(entity->type == ENTITY_CHEST) {
	            				assert(!entity->chestIsOpen);
	            				entity->chestIsOpen = true;



	            				int c = (int)randomBetween(1, 4);
	            				addItemToPlayer(gameState, ENTITY_STAMINA_POTION_1, c, true);

	            				//Assign the information for what the player collected
	            				gameState->itemCollectType = ENTITY_STAMINA_POTION_1;
	            				gameState->gameModeType = GAME_MODE_ITEM_COLLECT;
	            				gameState->entityChestToDisplay = entity;
	            				gameState->itemCollectCount = c;
	            				//////

	            				playGameSound(&globalLongTermArena, gameState->chestOpenSound, 0, AUDIO_FOREGROUND);
	            				playGameSound(&globalLongTermArena, gameState->successSound, 0, AUDIO_FOREGROUND);

	            			} else {
		            			if(entity->dialogType != ENTITY_DIALOG_NULL) {
									gameState->gameModeType = GAME_MODE_READING_TEXT;
									gameState->currentTalkText = findDialogInfo(entity->dialogType);
									gameState->messageIndex = 0; //start at the begining

									WavFile *sound = 0;
									if(gameState->currentTalkText.audioArray) {
										Asset *asset = findAsset(gameState->currentTalkText.audioArray[0]);
										if(asset) {
											sound = (WavFile *)asset->file;
										}
									}
									

									if(sound) {
										gameState->talkingNPC = playGameSound(&globalLongTermArena, sound, 0, AUDIO_FOREGROUND);
										gameState->talkingNPC->volume = 3.0f;
									}
		            			}	
	            			}
	            		}	
	            	} 
	            }
			}
		}


	if(entity->flashHurtTimer >= 0.0f) {
		entity->flashHurtTimer += dt;

		float canVal = entity->flashHurtTimer / 0.5f; 

		entity->colorTint = smoothStep01010V4(COLOR_WHITE, canVal, COLOR_RED);


		if(canVal >= 1.0f) {
			entity->flashHurtTimer = -1.0f;
		}
	}

	if(entity->type == ENTITY_WEREWOLF) {
		V3 worldP = easyTransform_getWorldPos(&player->T);
		V3 entP = easyTransform_getWorldPos(&entity->T);

		V3 diff = v3_minus(worldP, entP);

		V2 dir = normalizeV2(diff.xy);

		float werewolfMoveSpeed = gameState->werewolf_attackSpeed;

		//WEREWOLF ATTACKING//
		if(entity->stamina < entity->maxStamina) {
			werewolfMoveSpeed = gameState->werewolf_restSpeed;
		}

		if(getLength(diff.xy) < 10) {
			//Move towards the player
			entity->rb->dP.xy = v2_scale(werewolfMoveSpeed, dir);  
		}




		//WEREWOLF HURT PLAYER//
		if(entity->collider1->collisionCount > 0) {
			assert(entity->collider1->isTrigger);

            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {

            	//use 3 stamina
            	entity->stamina -= 3;
            	
            	float maxReboundForce = gameState->player_knockback_distance;
            	float reboundForce = maxReboundForce;

            	float damage = findDamage(entity->type);

            	if(info.e->shieldInUse) {
            		reboundForce = maxReboundForce*0.5f;
            		damage = 0.0f; //shield blocks all damage

            		if(info.e->stamina > 0.0f && !info.e->staminaMaxedOut) {
            			info.e->stamina -= 1.0f;
            			info.e->staminaTimer = 0; 

            			if(info.e->stamina < 0.0f) {
            				info.e->stamina = 0.0f;
            				info.e->staminaMaxedOut = true;
            			}	
            		} else {
            			//out of stamina - sheild not working very well
            			damage = 0.5f*findDamage(entity->type);
            			reboundForce = maxReboundForce;
            		}
            		
            	}

            	info.e->rb->accumForceOnce.xy = v2_plus(info.e->rb->accumForceOnce.xy, v2_scale(reboundForce, dir));

            	if(info.e->health > 0.0f) {
            		info.e->health -= damage;
            		createDamageNumbers(manager, damage, v3_plus(worldP, v3(0, 0, -1.0f)));	

            		easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);
            	}

            	if(info.e->health <= 0.0f) {
            		gameState->gameModeType = GAME_MODE_GAME_OVER;
            	}
            }
        }
        //////////////////////////////////////////////

        
		
	}

	if(entity->type == ENTITY_TRIGGER) {


		if(entity->triggerType == ENTITY_TRIGGER_LOAD_SCENE) {
			MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
			if(info.found) {
				playGameSound(&globalLongTermArena, gameState->doorSound, 0, AUDIO_FOREGROUND);

				EntityLoadSceneData *loadSceneData = (EntityLoadSceneData *)easyPlatform_allocateMemory(sizeof(EntityLoadSceneData), EASY_PLATFORM_MEMORY_ZERO);
				loadSceneData->gameState = gameState;
				loadSceneData->manager = manager;
				loadSceneData->sceneToLoad = entity->levelToLoad;
				loadSceneData->editorState = editorState;


				EasySceneTransition *transition = EasyTransition_PushTransition(transitionState, loadScene, loadSceneData, EASY_TRANSITION_CIRCLE_N64);
				gameState->gameIsPaused = true;	
			}
		}

		MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
		if(info.found) {
			if(entity->triggerType == ENTITY_TRIGGER_START_SWIM) {
				info.e->isSwimming = true;
				playGameSound(&globalLongTermArena, gameState->waterInSound, 0, AUDIO_FOREGROUND);
			} else if(entity->triggerType == ENTITY_TRIGGER_LOCATION_SOUND) {
				entity->currentSound = playLocationSound(&globalLongTermArena, gameState->seagullsSound, 0, AUDIO_FOREGROUND, easyTransform_getWorldPos(&entity->T));
				EasySound_LoopSound(entity->currentSound);

				entity->currentSound->innerRadius = 1.f;
				entity->currentSound->outerRadius = 50.0f;
			} 
		}

		info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_EXIT);	
		if(info.found) {
			if(entity->triggerType == ENTITY_TRIGGER_START_SWIM) {
				info.e->isSwimming = false;
				playGameSound(&globalLongTermArena, gameState->waterInSound, 0, AUDIO_FOREGROUND);
			} else if(entity->triggerType == ENTITY_TRIGGER_LOCATION_SOUND) {
				easySound_endSound(entity->currentSound);
				entity->currentSound = 0;
			} 
		}

		// if(entity->triggerType == ENTITY_TRIGGER_LOCATION_SOUND && entity->currentSound) {
			
		// }
	}

	

	if(entity->type == ENTITY_BLOCK_TO_PUSH) {

		float margin_dP = 0.0f;

		float speedFactor = 1.1f;

		//left collider
		MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found) {
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Left");
			if(info.e->rb->dP.x > margin_dP) {
				entity->rb->dP.x = speedFactor*info.e->rb->dP.x;
			}
		}

		//right collider
		info = MyEntity_hadCollisionWithType(manager, entity->collider2, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found) {
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Right");
			if(info.e->rb->dP.x < margin_dP) {
				entity->rb->dP.x = speedFactor*info.e->rb->dP.x;
			}
		}

		//Top collider
		info = MyEntity_hadCollisionWithType(manager, entity->collider3, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found) {
			if(info.e->rb->dP.y < -margin_dP) {
				entity->rb->dP.y = speedFactor*info.e->rb->dP.y;
			}
		}

		//bottom collider
		info = MyEntity_hadCollisionWithType(manager, entity->collider4, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found) {
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Bottom");
			if(info.e->rb->dP.y > margin_dP) {
				entity->rb->dP.y = speedFactor*info.e->rb->dP.y;
			}
		}

	}

	if(entity->type == ENTITY_SEAGULL) {
		entity->lifeSpanLeft -= dt;

		float fadeVal = 2.0f;

		if(entity->lifeSpanLeft < fadeVal) {
			//fade out
			float lerpVal = 1.0f - ((fadeVal - entity->lifeSpanLeft) / fadeVal); 

			entity->colorTint.w = lerpVal;

			if(entity->lifeSpanLeft <= 0.0f) {
				entity->isDead = true;
			}
		}
	}

	if(entity->type == ENTITY_ENTITY_CREATOR) {

		if(entity->typeToCreate != ENTITY_NULL) {
			entity->timeSinceLastCreation += dt;

			if(entity->timeSinceLastCreation > entity->rateOfCreation) {
				entity->timeSinceLastCreation = 0;

				ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
				EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
				entityToAdd->type = entity->typeToCreate;
				entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, randomBetween(-5, 5), 0));

				entityToAdd->dP.y = 0;
				entityToAdd->dP.x = -2;//randomBetween(-5, 5);
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

		if(distance < 100.0f || entity->healthBarTimer >= 0.0f) { //Show health bar
			float percent = (float)entity->health / (float)entity->maxHealth;

			V4 color = COLOR_GREEN;

			if(percent < 0.5f) {
				color = COLOR_RED;
			}


			float w = 1.2f; //game world meters
			float h = 0.3f; //game world meters

			V3 position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0, -2.0f));

			gameState->tempTransform.pos = position;

			gameState->tempTransform.scale.xy = v2(w, h);

			setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
			renderDrawQuad(globalRenderGroup, COLOR_LIGHT_GREY);

			gameState->tempTransform.scale.x = percent*w;
			gameState->tempTransform.pos = v3_plus(gameState->tempTransform.pos, v3_scale(-0.05f, cameraZ_axis));

			float overhang = w - percent*w;
			gameState->tempTransform.pos.x -= 0.5f*overhang;		

			setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
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


	if(entity->type == ENTITY_CHEST) {

		if(gameState->entityChestToDisplay == entity) {
			sprite = findTextureAsset("chest_gold.png");
		} else if(entity->chestIsOpen) {
			sprite = findTextureAsset("chest_open.png");
		}	
	}

	if(!sprite && !entity->model && entity->type != ENTITY_TERRAIN && entity->type != ENTITY_SWORD && entity->type != ENTITY_SHEILD) {
		//NOTE: If the program is crashing here, it most likely could find a model or sprite on load time that it was looking for 

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
	// if(entity->rb) {
	// 	if(entity->rb->dP.x < -0.1) {
	// 		entity->isFlipped = true;
	// 	} else if(entity->rb->dP.x > 0.1) {
	// 		entity->isFlipped = false;
	// 	}
	// }


	////////////////////
	//NOTE: Flip sprite
	if(entity->isFlipped) {
		// NOTE: flipSprite
		T.E_[0] *= -1;
		T.E_[1] *= -1;
		T.E_[2] *= -1;
	}

	/////////////////////

	setModelTransform(globalRenderGroup, T);
	
	if(sprite && DEBUG_DRAW_SCENERY_TEXTURES && !entity->renderFirstPass && !(entity->flags & ENTITY_SHOULD_NOT_RENDER)) { renderSetShader(globalRenderGroup, &pixelArtProgram); renderDrawSprite(globalRenderGroup, sprite, entity->colorTint); }



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
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1,  false);
    e->T.pos.z = -0.5f;
    return e;
}


static Entity *initEntityCreator(GameState *gameState, EntityManager *manager, V3 worldP) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_ENTITY_CREATOR, 0, &globalWhiteTexture, COLOR_WHITE, -1,  false);
    return e;
}


static Entity *initSeagull(GameState *gameState, EntityManager *manager, V3 worldP) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SEAGULL, 0, 0, COLOR_WHITE, -1, true);
    e->T.pos.z = -3.5f;
    e->rb->dP.x = -2.0f;
    e->T.scale = v3(2.5f, 2.5f, 2.5f);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, &gameState->seagullAnimation, EASY_ANIMATION_PERIOD);	
    e->maxLifeSpan = 180.0f;
    e->lifeSpanLeft = e->maxLifeSpan;
    return e;
}



static Entity *initEmptyTrigger(GameState *gameState, EntityManager *manager, V3 worldP, EntityTriggerType triggerType) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_TRIGGER, 0, &globalWhiteTexture, COLOR_WHITE, -1, true);
    e->T.pos.z = -0.5f;

    e->triggerType = triggerType;

    e->flags |= (int)ENTITY_SHOULD_NOT_RENDER;

    e->levelToLoad = gameState->emptyString;

    return e;
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

static Entity *initSign(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SIGN, 0, splatTexture, COLOR_WHITE, -1, true);
	return e;
}

static Entity *initHouse(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_HOUSE, 0, splatTexture, COLOR_WHITE, -1, true);
	float w = 6.0f;
	float h = e->sprite->aspectRatio_h_over_w*w;
	e->T.scale = v3(w,  h, 1);
	e->T.pos.z = -0.5f*h;

	e->collider->offset.y = -0.5f*w;
	e->collider->dim2f.y = 0.5f;
	e->collider->dim2f.x = 1.1f;


	e->collider1->offset.y = -w - 0.5f;
	e->collider1->dim2f = v2(0.3f, 0.1f);

	e->levelToLoad = gameState->emptyString;
	

	return e;
}

static Entity *initLampPost(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_LAMP_POST, 0, splatTexture, COLOR_WHITE, -1, true);
	
	float w = 3.0f;
	float h = e->sprite->aspectRatio_h_over_w*w;
	e->T.scale = v3(w, h, 1);
	e->T.pos.z = -0.5f*h;

	e->collider->offset.x = -1.0f;
	e->collider->offset.y = -1.5f;
	
	e->collider->dim2f.x = 0.5f;
	e->collider->dim2f.y = 0.3f;


	return e;
}

static Entity *initChest(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_CHEST, 0, findTextureAsset("chest.png"), COLOR_WHITE, -1, true);
	float w = 2;
	e->T.scale = v3(w, e->sprite->aspectRatio_h_over_w*w, 1);
	e->T.pos.z = -1;

	e->collider->offset.y = 0.4f;
	e->collider->dim2f.x = 1.5f;

	e->collider1->offset.y = -2.0f;
	e->collider1->dim2f = v2(1.0f, 0.5f);

	return e;
}

static Entity *initHorse(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_HORSE, 0, findTextureAsset("horse.png"), COLOR_WHITE, -1, true);
	e->T.scale = v3(5, 5, 5);
	e->T.pos.z = -2;
	e->model = findModelAsset_Safe("castle.obj");

	return e;
}


static Entity *initPushRock(GameState *gameState, EntityManager *manager, V3 worldP) {
	float blockDim = 2;

	Entity *e = initEntity(manager, 0, worldP, v2(blockDim, blockDim), v2(1, 1), gameState, ENTITY_BLOCK_TO_PUSH, gameState->inverse_weight, findTextureAsset("crate.jpg"), COLOR_WHITE, -1, true);
		
	e->T.pos.z = -0.5f;
	//Add triggers to see if player is pushing	

	float triggerSize = 0.4f;

	float offset = 0.5f*blockDim + 0.5f*triggerSize;

	float longTiggerSize = 0.8f;

	//Left
	e->collider1 = EasyPhysics_AddCollider(&gameState->physicsWorld, &e->T, e->rb, EASY_COLLIDER_RECTANGLE, v3(-offset, 0, 0), true, v3(triggerSize, longTiggerSize, 0));
	e->collider1->layer = EASY_COLLISION_LAYER_ITEMS;
	//Right
	e->collider2 = EasyPhysics_AddCollider(&gameState->physicsWorld, &e->T, e->rb, EASY_COLLIDER_RECTANGLE, v3(offset, 0, 0), true, v3(triggerSize, longTiggerSize, 0));
	e->collider2->layer = EASY_COLLISION_LAYER_ITEMS;
	//Top
	e->collider3 = EasyPhysics_AddCollider(&gameState->physicsWorld, &e->T, e->rb, EASY_COLLIDER_RECTANGLE, v3(0, offset, 0), true, v3(longTiggerSize, triggerSize, 0));
	e->collider3->layer = EASY_COLLISION_LAYER_ITEMS;
	//Bottom
	e->collider4 = EasyPhysics_AddCollider(&gameState->physicsWorld, &e->T, e->rb, EASY_COLLIDER_RECTANGLE, v3(0, -offset, 0), true, v3(longTiggerSize, triggerSize, 0));
	e->collider4->layer = EASY_COLLISION_LAYER_ITEMS;
	/////////////////////////////////////////////////////

	return e;
}



static Entity *initEntityOfType(GameState *gameState, EntityManager *manager, V3 position, Texture *splatTexture, EntityType entType, SubEntityType subtype, bool colliderSet, EntityTriggerType triggerType, char *audioFile) {
	Entity *newEntity = 0;

	switch(entType) {
		case ENTITY_SCENERY: {
			if(subtype & ENTITY_SUB_TYPE_TORCH) {
				newEntity = initTorch(gameState, manager, position);
			} else if(subtype & ENTITY_SUB_TYPE_ONE_WAY_UP_PLATFORM) {
                newEntity = initOneWayPlatform(gameState, manager, position, splatTexture);
            } else {
				if(colliderSet) {
					newEntity = initScenery_withRigidBody(gameState, manager, position, splatTexture);
				} else {
					newEntity = initScenery_noRigidBody(gameState, manager, position, splatTexture);	
				}	
			}
			
		} break;
		case ENTITY_WIZARD: {
			newEntity = initWizard(gameState, manager, position);
			manager->player = newEntity;

            easyAnimation_addAnimationToController(&newEntity->animationController, &gameState->animationFreeList, &gameState->wizardIdleForward, EASY_ANIMATION_PERIOD);      

		} break;
		case ENTITY_PLAYER_PROJECTILE: {
			assert(false);
		} break;
		case ENTITY_SKELETON: {
			newEntity = initSkeleton(gameState, manager, position);
		} break;
        case ENTITY_SEAGULL: {
            newEntity = initSeagull(gameState, manager, position);
        } break;
        case ENTITY_SWORD: {
            newEntity = initSword(gameState, manager, position);
        } break;
        case ENTITY_CHEST: {
            newEntity = initChest(gameState, manager, position);
        } break;
        case ENTITY_HORSE: {
            newEntity = initHorse(gameState, manager, position);
        } break;
        case ENTITY_TRIGGER: {
            newEntity = initEmptyTrigger(gameState, manager, position, triggerType);
        } break;
        case ENTITY_BLOCK_TO_PUSH: {
            newEntity = initPushRock(gameState, manager, position);
        } break;
        case ENTITY_SHEILD: {
            newEntity = initSheild(gameState, manager, position);
        } break;
        case ENTITY_SIGN: {
            newEntity = initSign(gameState, manager, position, splatTexture);
        } break;
        case ENTITY_LAMP_POST: {
            newEntity = initLampPost(gameState, manager, position, splatTexture);
        } break;
        case ENTITY_HOUSE: {
            newEntity = initHouse(gameState, manager, position, splatTexture);
        } break;
        case ENTITY_WEREWOLF: {
            newEntity = initWerewolf(gameState, manager, position);
        } break;
		case ENTITY_HEALTH_POTION_1: {
			assert(false);
		} break;
		case ENITY_AUDIO_CHECKPOINT: {
			newEntity = initAudioCheckPoint(gameState, manager, position);
			newEntity->audioFile = audioFile;
		} break;
		case ENITY_CHECKPOINT: {
			newEntity = initCheckPoint(gameState, manager, position);
		} break;
        case ENTITY_TERRAIN: {
            newEntity = initTerrain(gameState, manager, position);
        } break;
        case ENTITY_ENTITY_CREATOR: {
            newEntity = initEntityCreator(gameState, manager, position);
        } break;
		default: {

		}
	}

	return newEntity;
}