typedef enum {
	ENTITY_SHOW_HEALTH_BAR = 1 << 0,
	ENTITY_SHOULD_SAVE = 1 << 1,
	ENTITY_SHOULD_NOT_RENDER = 1 << 2,
	ENTITY_SHOULD_SAVE_ANIMATION = 1 << 3,
} EntityFlags;

typedef enum {
	ENTITY_SUB_TYPE_NONE = 0,
	ENTITY_SUB_TYPE_TORCH = 1 << 0,
	ENTITY_SUB_TYPE_ONE_WAY_UP_PLATFORM = 1 << 1,
	ENTITY_SUB_TYPE_SWORD = 1 << 2,
	ENTITY_SUB_TYPE_CAT = 1 << 3,
	ENEMY_IGNORE_PLAYER = 1 << 4
} SubEntityType;

typedef enum {
	ENEMY_FIRE_BALL,
} EntityEnemyType;

typedef enum {
	ENTITY_DIRECTION_LEFT,
	ENTITY_DIRECTION_RIGHT,
	ENTITY_DIRECTION_UP,
	ENTITY_DIRECTION_DOWN,
} EntityDirection;

typedef struct {
	EntityType type;

	u64 flags;

	EntityDirection direction;

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
	bool chestIsOpen;
	ChestType chestType;
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

	//For the ai stuff
	EasyAiController *aiController;
	V3 lastSetPos; //the position the A* path find is travelling to
	V3 beginSetPos;
	float travelTimer; //timer to lerp between A* positions
	V3 lastGoalPos;
	///


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

	int animationIndexOn; //for ai state machines

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

	float innerRadius;
	float outerRadius;

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

	//For the push block
	float moveTimer;
	V3 startMovePos;
	V3 endMovePos;
	////////////////////


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


	initArray(&manager->activeParticleSystems, ParticleSystemListItem);

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
    
    for(int i = 0; i < collider->collisions.count && !result.found; ++i) {
    	EasyCollisionInfo *info = getElementFromAlloc(&collider->collisions, i, EasyCollisionInfo);

        if(info->type == colType) {
            int id = info->objectId; 
            for(int j = 0; j < manager->entities.count && !result.found; ++j) {
                Entity *e = (Entity *)getElement(&manager->entities, j);
                if(e) { //can be null
                    if(e->T.id == id && e->type == type) {
                        result.e = e;
                        result.collisionType = info->type;
                        result.found = true;
                        break;
                    }
                }
            }
        }
    } 
    
    return result;
}


static Entity *findEntityById(EntityManager *manager, int id) {
	Entity *result = 0;
	for(int j = 0; j < manager->entities.count && !result; ++j) {
	    Entity *e = (Entity *)getElement(&manager->entities, j);
	    if(e) { //can be null
	        if(e->T.id == id) {
	            result = e;
	            break;
	        }
	    }
	}
	return result;
}

typedef struct {
	int count;
	EntityType type;
} ChestContents;

static ChestContents getChestContents(ChestType type) {
	ChestContents result = {};

	if(type == CHEST_TYPE_HEALTH_POTION) {
		result.count = (int)randomBetween(1, 4);
		result.type = ENTITY_HEALTH_POTION_1;

	} else if(type == CHEST_TYPE_STAMINA_POTION) {
		result.count = (int)randomBetween(1, 4);
		result.type = ENTITY_STAMINA_POTION_1;
	} else if(type == CHEST_TYPE_KEY) {
		result.count = 1;
		result.type = ENTITY_KEY;
	} else {
		//nothing
	}

	return result;
}

static bool isBlockOnSquare(EntityManager *manager, V3 testP, Entity *self) {
	bool found = false;
	for(int i = 0; i < manager->entities.count && !found; ++i) {
	    Entity *e = (Entity *)getElement(&manager->entities, i);
	    if(e && e->type == ENTITY_BLOCK_TO_PUSH && e != self) {
	    	V3 pos = roundToGridBoard(easyTransform_getWorldPos(&e->T), 1);

	    	if((int)pos.x == (int)testP.x && (int)pos.y == (int)testP.y && (int)pos.z == (int)testP.z) {
	    		found = true;
	    		break;
	    	}
	    }
	}
	return found;
}

static ParticleSystemListItem *getParticleSystem(EntityManager *m) {
	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&m->activeParticleSystems);
	ParticleSystemListItem *result = (ParticleSystemListItem *)arrayInfo.elm;
	assert(result);

	easyConsole_pushInt(DEBUG_globalEasyConsole, m->activeParticleSystems.count);

	//init the particle system

	particle_system_settings ps_set1 = InitParticlesSettings(PARTICLE_SYS_DEFAULT, 0.2f);

	ps_set1.posBias = rect3f(0, 0, 0, 0, 0, 0);

	ps_set1.bitmapScale = 0.1f;

	pushParticleBitmap(&ps_set1, findTextureAsset("smoke_01.png"), "particle1");
	pushParticleBitmap(&ps_set1, findTextureAsset("smoke_02.png"), "particle2");

	InitParticleSystem(&result->ps, &ps_set1, 16);

	setParticleLifeSpan(&result->ps, 0.6f);

	result->ps.Active = false;
	result->ps.Set.Loop = false;

	///////////////////////////////////////////

	Reactivate(&result->ps);

	result->color = COLOR_BROWN;//hexARGBTo01Color(0xFFF5DEB3);

	return result;	
}


static void renderKeyPromptHover(GameState *gameState, Texture *keyTexture, Entity *entity, float dt, bool useScaleY, RenderGroup *group) {
	entity->tBob += dt;

	V3 scale = easyTransform_getWorldScale(&entity->T);

	float sy = 0.11f;
	if(useScaleY) {
		sy = scale.y + 0.01f;
	}

	V3 position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0, -(sy + 0.1f*sin(entity->tBob))));

	gameState->tempTransform.pos = position;
	float width = 0.5f;
	float height = keyTexture->aspectRatio_h_over_w*width;

	gameState->tempTransform.scale.xy = v2(width, height);

	setModelTransform(group, easyTransform_getTransform(&gameState->tempTransform));
	renderDrawSprite(group, keyTexture, COLOR_WHITE);

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

	if(type == ENTITY_ENEMY_PROJECTILE) { return 5; }

	return 1.0f; //default value
}

static inline void damagePlayer(EntityManager *manager, Entity *e, GameState *gameState, EasyCamera *cam, float damage, V3 worldP) {
	if(e->health > 0.0f) {
		e->health -= damage;
		createDamageNumbers(manager, damage, v3_plus(worldP, v3(0, 0, -1.0f)));	

		easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);
	}

	if(e->health <= 0.0f) {
		gameState->gameModeType = GAME_MODE_GAME_OVER;
	}
}

static inline void player_useAttackItem(GameState *gameState, EntityManager *manager, float damage, Entity *entity) {
	assert(entity->collider1->isTrigger);
    if(entity->collider1->collisions.count > 0) {
    	
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

        		easyConsole_addToStream(DEBUG_globalEasyConsole, "Skeleton Hit 1");
        		if(enemy->type == ENTITY_WEREWOLF) {
    				easyAnimation_emptyAnimationContoller(&enemy->animationController, &gameState->animationFreeList);
    				easyAnimation_addAnimationToController(&enemy->animationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "SHit_4.png"), EASY_ANIMATION_PERIOD);	
    				if(enemy->health > 0.0f) {
    					easyAnimation_addAnimationToController(&enemy->animationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "SIdle_4.png"), EASY_ANIMATION_PERIOD);		
    				}
    				
        			easyConsole_addToStream(DEBUG_globalEasyConsole, "Skeleton Hit");

        			assert(easyAnimation_getCurrentAnimation(&enemy->animationController, gameState_findSplatAnimation(gameState, "SHit_4.png")));
        		}
        		
        	}

        	//See if the enemey is dead
        	if(enemy->health <= 0.0f) {
        		// enemy->isDead = true;

        		easyAnimation_addAnimationToController(&enemy->animationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "SDeath_4.png"), EASY_ANIMATION_PERIOD);	

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
			gameState->playerPotionParticleSystemColor = COLOR_GREEN;
			
			
		} break;
		case ENTITY_STAMINA_POTION_1: {
			entity->stamina++;

			if(entity->stamina > entity->maxStamina) {
				entity->stamina = entity->maxStamina;
			}
			
			drawParticleSystem = true;
			gameState->playerPotionParticleSystemColor = COLOR_BLUE;

			
			
		} break;
		case ENTITY_SWORD: {
			easyConsole_pushInt(DEBUG_globalEasyConsole, (int)entity->shieldInUse);
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Used Sword");
			if(entity->stamina >= 3.0f && !entity->shieldInUse) {

				player_useAttackItem(gameState, manager, findDamage(type), entity);

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



//For when you collect your item
static char *getInventoryCollectString(EntityType type, int count, Arena *arena) {
    char *result = "Empty";
    switch(type) {
        case ENTITY_HEALTH_POTION_1: {
            result = easy_createString_printf(arena, "{s: 3}You found {c: 0 1 0 1 }%d{c: 0 0 0 1} health potions. It looks like it might be good to replenish health.", count);
            // assert(false);
        } break;
        case ENTITY_STAMINA_POTION_1: {
            result = easy_createString_printf(arena, "{s: 3}You found {c: 0 1 0 1 }%d{c: 0 0 0 1}potions. It looks like it might be good to replenish stamina.", count);
            // assert(false);
        } break;
        case ENTITY_SWORD: {
            result = "Sharp sword";
        } break;
        case ENTITY_KEY: {
            result = "{s: 3}You found a brass key. It might open something?";
        } break;
        case ENTITY_SHEILD: {
            result = "Protective shield";
        } break;
        default: {

        }
    }
    return result;
}


static void entityManager_emptyEntityManager(EntityManager *manager, EasyPhysics_World *physicsWorld) {
	easyArray_clear(&manager->entitiesToDeleteForFrame);
	easyArray_clear(&manager->entities);
	easyArray_clear(&manager->entitiesToAddForFrame);
	easyArray_clear(&manager->damageNumbers);
	easyArray_clear(&manager->activeParticleSystems);
	assert(manager->activeParticleSystems.count == 0);
	

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

typedef struct {
	GameState *gameState;
	V3 playerStartP;
	Entity *player;
} EntityResetOnDieData;

static void playerDiedReset(void *data_) {
	EntityResetOnDieData *data = (EntityResetOnDieData *)data_;

	data->gameState->gameModeType = GAME_MODE_PLAY;
	data->player->health = data->player->maxHealth;
	data->player->T.pos = data->playerStartP;
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

	entity->moveTimer = -1;

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

	entity->travelTimer = -1.0f;
	
	entity->layer = layer;
	entity->sprite = sprite;
	entity->lifeSpanLeft = -1.0f;
	entity->type = type;
	entity->rotation = 0;
	entity->shieldInUse = false;
	entity->isDeleted = false;
	entity->tBob = 0;
	entity->chestIsOpen = false;

	entity->innerRadius = 0.2f;
	entity->outerRadius = 4.0f;

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

	if(type == ENTITY_FOG) { isTrigger = true; gravityFactor = 0; inverse_weight = 0; dragFactor = 0;} 

	if(type == ENTITY_ENEMY_PROJECTILE) { isTrigger = true; gravityFactor = 0; inverse_weight = 1.f/20.f; dragFactor = 0; }


	if(type == ENTITY_SEAGULL) { gravityFactor = 0; inverse_weight = 1.f / 20.0f; dragFactor = 0; }

	if(canCollide) {
		// char string[256];
		// sprintf(string, "%f", gravityFactor);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, string);

		entity->rb = EasyPhysics_AddRigidBody(&gameState->physicsWorld, inverse_weight, 0, dragFactor, gravityFactor);
		entity->collider = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), isTrigger, v3(physicsDim.x, physicsDim.y, 0));
		
		if(type == ENTITY_HEALTH_POTION_1 || type == ENTITY_SIGN || type == ENTITY_WEREWOLF || type == ENTITY_WIZARD || type == ENTITY_HORSE || type == ENTITY_CHEST || type == ENTITY_HOUSE || type == ENTITY_SHOOT_TRIGGER || type == ENTITY_TRIGGER_WITH_RIGID_BODY) { 
			//Add a TRIGGER aswell
			entity->collider1 = EasyPhysics_AddCollider(&gameState->physicsWorld, &entity->T, entity->rb, EASY_COLLIDER_RECTANGLE, v3(0, 0, 0), true, v3(physicsDim.x, physicsDim.y, 0));
			entity->collider1->layer = EASY_COLLISION_LAYER_ITEMS;

			//FOR WIZARD CREATE THE HIT BOX WHEN ENTITIES ARE INSIDE IT 
			if(type == ENTITY_WIZARD) {

				//Offset the trigger in front of the player
				entity->collider1->offset.y += 2.5f;
				entity->collider1->layer = EASY_COLLISION_LAYER_PLAYER_BULLET;
			}

		}


		switch(entity->type) {
			case ENTITY_TRIGGER_WITH_RIGID_BODY: {
				entity->collider->layer = EASY_COLLISION_LAYER_WORLD;
			} break;
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
			case ENTITY_ENEMY_PROJECTILE:
			case ENTITY_FOG:
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
			default: {
				entity->collider->layer = EASY_COLLISION_LAYER_WORLD;
			}
		}

	}
	
	return entity;
}



////////////////////////////////////////////////////////////////////


void updateEntity(EntityManager *manager, Entity *entity, GameState *gameState, float dt, AppKeyStates *keyStates, EasyConsole *console, EasyCamera *cam, Entity *player, bool isPaused, V3 cameraZ_axis, EasyTransitionState *transitionState, EasySound_SoundState *soundState, EditorState *editorState, RenderGroup *shadowMapGroup, RenderGroup *entitiesRenderGroup) {

	RenderProgram *shaderProgram = &pixelArtProgram;

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

		V3 wP = easyTransform_getWorldPos(&entity->T);

		//Update listener position which is the wizard
		easySound_updateListenerPosition(soundState, wP);

		Animation *animToAdd = 0;


		easy_setPlayerPosition(entitiesRenderGroup, wP);
		

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


			float maxOffsetValue = Max(absVal(entity->collider1->offset.y), absVal(entity->collider1->offset.x));
			float maxDim = Max(absVal(entity->collider1->dim2f.y), absVal(entity->collider1->dim2f.x));
			float minDim = Min(absVal(entity->collider1->dim2f.y), absVal(entity->collider1->dim2f.x));

			// easyConsole_pushFloat(DEBUG_globalEasyConsole, entity->collider1->offset.x);
			// easyConsole_pushFloat(DEBUG_globalEasyConsole, entity->collider1->offset.y);

			float staminaFactor = 0.01f;

			if(isDown(keyStates->gameButtons, BUTTON_LEFT) && !isPaused) {
				entity->rb->accumForce.x += -gameState->walkPower*walkModifier;

				if(entity->isSwimming) {
					animToAdd = &gameState->wizardSwimLeft;
				} else {
					animToAdd = &gameState->wizardLeft;
					idleAnimation = &gameState->wizardIdleLeft;	
				}

				entity->collider1->offset.x = -maxOffsetValue;
				entity->collider1->offset.y = 0;

				entity->collider1->dim2f = v2(maxDim, minDim);

				staminaFactor = 0.1f;

				entity->direction = ENTITY_DIRECTION_LEFT;
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

				entity->collider1->offset.x = maxOffsetValue;
				entity->collider1->offset.y = 0;

				entity->collider1->dim2f = v2(maxDim, minDim);

				entity->direction = ENTITY_DIRECTION_RIGHT;

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

				entity->collider1->offset.x = 0;
				entity->collider1->offset.y = maxOffsetValue;

				entity->collider1->dim2f = v2(minDim, maxDim);

				entity->direction = ENTITY_DIRECTION_UP;

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

				entity->collider1->offset.x = 0;
				entity->collider1->offset.y = -maxOffsetValue;

				entity->collider1->dim2f = v2(minDim, maxDim);

				entity->direction = ENTITY_DIRECTION_DOWN;
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


		if(wasPressed(keyStates->gameButtons, BUTTON_Q) && !isPaused) {
			if(entity->direction == ENTITY_DIRECTION_LEFT) {
				animToAdd = &gameState->wizardSwordAttackLeft;
				idleAnimation = &gameState->wizardIdleLeft;
			} else if(entity->direction == ENTITY_DIRECTION_RIGHT) {
				animToAdd = &gameState->wizardSwordAttackRight;
				idleAnimation = &gameState->wizardIdleRight;
			} else if(entity->direction == ENTITY_DIRECTION_UP) {
				animToAdd = &gameState->wizardSwordAttackBack;
				idleAnimation = &gameState->wizardIdleForward;
			} else if(entity->direction == ENTITY_DIRECTION_DOWN) {
				animToAdd = &gameState->wizardSwordAttackFront;
				idleAnimation = &gameState->wizardIdleBottom;
			}  

			player_useAttackItem(gameState, manager, findDamage(entity->type), entity);

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
		if(entity->collider1->collisions.count > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	gameState->itemSpots[gameState->itemCount++].type = ENTITY_HEALTH_POTION_1;

            	entity->isDead = true; //remove from entity list
            	
            }
		}
		
	}

	if(entity->type == ENTITY_HORSE) {
		if(entity->collider1->collisions.count > 0) {

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

		if(entity->collider->collisions.count > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	gameState->itemSpots[gameState->itemCount++].type = entity->type;
            	
            	playGameSound(&globalLongTermArena, gameState->successSound, 0, AUDIO_FOREGROUND);

            	entity->isDead = true; //remove from entity list
            	
            }
		}
	}


	

	if(entity->type == ENTITY_HOUSE) {
		if(entity->collider1->collisions.count > 0) {

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


            			easyConsole_addToStream(DEBUG_globalEasyConsole, "Push Trasition 2");

            			EasySceneTransition *transition = EasyTransition_PushTransition(transitionState, loadScene, loadSceneData, EASY_TRANSITION_CIRCLE_N64);
            			gameState->gameIsPaused = true;	
            		}
        			
            	}
            }
        }
	}


		if(entity->type == ENTITY_SIGN || entity->type == ENTITY_CHEST) {
			if(entity->collider1->collisions.count > 0) {

	            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
	            if(info.found) {

	            	bool canInteractWith = gameState->gameModeType != GAME_MODE_READING_TEXT;

	            	if(entity->type == ENTITY_CHEST && entity->chestIsOpen) {
	            		canInteractWith = false;
	            	}
	            	
	            	if(canInteractWith) {
	            		renderKeyPromptHover(gameState, gameState->spacePrompt, entity, dt, true, entitiesRenderGroup);
	            		
	            		
	            		if(wasPressed(keyStates->gameButtons, BUTTON_SPACE)) 
	            		{	
	            			
	            			if(entity->type == ENTITY_CHEST) {
	            				assert(!entity->chestIsOpen);
	            				entity->chestIsOpen = true;

	            				ChestContents chestContents = getChestContents(entity->chestType);

	            				if(chestContents.count > 0) {
	            					addItemToPlayer(gameState, chestContents.type, chestContents.count, true);

	            					//Assign the information for what the player collected
	            					gameState->gameModeType = GAME_MODE_ITEM_COLLECT;
	            					gameState->entityChestToDisplay = entity;
	            					gameState->itemCollectType = chestContents.type;
	            					
	            					//////

	            					//The dialog options 
	            					// Make the temporary talk node
	            					{
		            					gameState->gameDialogs.perDialogArenaMark = takeMemoryMark(&gameState->gameDialogs.perDialogArena);

		            					gameState->currentTalkText = constructDialogNode(&gameState->gameDialogs);
		            					pushTextToNode(gameState->currentTalkText, getInventoryCollectString(chestContents.type, chestContents.count, &gameState->gameDialogs.perDialogArena));

	            					}


	            					gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING;
	            					gameState->enteredTalkModeThisFrame = true;
	            					
	            					gameState->messageIndex = 0; //start at the begining
	            					easyFontWriter_resetFontWriter(&gameState->fontWriter);
									///////////////////////////////////////////////////////
	            				}
	            				
	            				playGameSound(&globalLongTermArena, gameState->chestOpenSound, 0, AUDIO_FOREGROUND);
	            				playGameSound(&globalLongTermArena, gameState->successSound, 0, AUDIO_FOREGROUND);

	            			} else {
		            			if(entity->dialogType != ENTITY_DIALOG_NULL) {
									gameState->gameModeType = GAME_MODE_READING_TEXT;
									gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING;
									gameState->enteredTalkModeThisFrame = true;
									
									gameState->currentTalkText = findDialogInfo(entity->dialogType, &gameState->gameDialogs);
									gameState->messageIndex = 0; //start at the begining
									easyFontWriter_resetFontWriter(&gameState->fontWriter);

									// WavFile *sound = 0;
									// if(gameState->currentTalkText.audioArray) {
									// 	Asset *asset = findAsset(gameState->currentTalkText.audioArray[0]);
									// 	if(asset) {
									// 		sound = (WavFile *)asset->file;
									// 	}
									// }
									

									// if(sound) {
									// 	gameState->talkingNPC = playGameSound(&globalLongTermArena, sound, 0, AUDIO_FOREGROUND);
									// 	gameState->talkingNPC->volume = 3.0f;
									// }
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

		

		if(!entity->aiController) {
			entity->aiController = easyAi_initController(&globalPerSceneArena);
		}	

		bool isDying = easyAnimation_getCurrentAnimation(&entity->animationController, gameState_findSplatAnimation(gameState, "SDeath_4.png"));
		bool isHurt = easyAnimation_getCurrentAnimation(&entity->animationController, gameState_findSplatAnimation(gameState, "SHit_4.png")) || isDying;

		//TODO: make this an array for all enemy types 





		if(!isHurt) {
			// easyConsole_addToStream(DEBUG_globalEasyConsole, "werewolf is not hurt");

			V3 worldP = easyTransform_getWorldPos(&player->T);
			V3 entP = easyTransform_getWorldPos(&entity->T);
			Animation *animToAdd = 0;

			float werewolfMoveSpeed = gameState->werewolf_attackSpeed;

			//WEREWOLF ATTACKING//
			if(entity->stamina < entity->maxStamina) {
				werewolfMoveSpeed = gameState->werewolf_restSpeed;
			}

			V3 playerInWorldP = roundToGridBoard(worldP, 1);
			V3 entP_inWorld = roundToGridBoard(entP, 1);

			if(entity->subEntityType & ENEMY_IGNORE_PLAYER) {
				werewolfMoveSpeed = 1;
				worldP = playerInWorldP = entity->aiController->searchBouys[entity->aiController->bouyIndexAt];
				if(getLengthSqrV3(v3_minus(playerInWorldP, entP_inWorld)) < 1.0f) {

					entity->aiController->bouyIndexAt++;

					if(entity->aiController->bouyIndexAt >= entity->aiController->searchBouysCount) {
						entity->aiController->bouyIndexAt = 0;
					}
				}
			} 


			EasyAi_A_Star_Result aiResult = easyAi_update_A_star(entity->aiController, entP_inWorld,  playerInWorldP);


			if(aiResult.found) {
				entity->lastSetPos = aiResult.nextPos;

				if(v3Equal(entity->lastSetPos, playerInWorldP)) {
					entity->lastSetPos = worldP; //get floating point version
				}

				// entity->T.pos = lerpV3(, clamp01(dt*werewolfMoveSpeed), entity->lastSetPos);
				
				V3 diff = v3_minus(entity->lastSetPos, entity->T.pos);

				V2 dir = normalizeV2(diff.xy);

				animToAdd = gameState_findSplatAnimation(gameState, "SWalk_4.png");

				entity->rb->dP.xy = v2_scale(werewolfMoveSpeed, dir);
			} else {
				animToAdd = gameState_findSplatAnimation(gameState, "SIdle_4.png");
			}


			V3 diff = v3_minus(worldP, entP);

			V2 dir = normalizeV2(diff.xy);


			//WEREWOLF HURT PLAYER//
			if(entity->collider1->collisions.count > 0) {
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

	            	
	            	damagePlayer(manager, info.e, gameState, cam, findDamage(entity->type), worldP);

	            }
	        }
	        //////////////////////////////////////////////


	        if(animToAdd) {

	        	if(!easyAnimation_getCurrentAnimation(&entity->animationController, animToAdd)) {
	        		easyConsole_addToStream(DEBUG_globalEasyConsole, "add anim");
	        		easyAnimation_emptyAnimationContoller(&entity->animationController, &gameState->animationFreeList);
	        		easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, animToAdd, EASY_ANIMATION_PERIOD);	
	        	}
	        }
    	} else {
    		easyConsole_addToStream(DEBUG_globalEasyConsole, "werewolf is hurt");
    	}
		
	}

	if(entity->type == ENTITY_AI_ANIMATION) {
		if(entity->subEntityType & (int)ENTITY_SUB_TYPE_CAT) {
			//	 
			
			if(!entity->animationController.finishedAnimationLastUpdate && !easyAnimation_isControllerEmpty(&entity->animationController)) {
				//keep animation going
			} else {
				Animation *anim = 0;
				if(entity->animationIndexOn == 0) {
					anim  = gameState_findSplatAnimation(gameState, "cheshire_cat_sitting_tail6.png"); 
					entity->animationIndexOn++;
				} else if(entity->animationIndexOn == 1) {
					anim  = gameState_findSplatAnimation(gameState, "cheshire_cat_sitting6.png"); 
					entity->animationIndexOn++;
				} else if(entity->animationIndexOn == 2) {
					anim  = gameState_findSplatAnimation(gameState, "cheshire_cat_sittinglickingpaw9.png"); 
					entity->animationIndexOn++;
				} else if(entity->animationIndexOn == 3) {
					anim  = gameState_findSplatAnimation(gameState, "cheshire_cat_sittingwaggingtail9.png"); 
					entity->animationIndexOn++;
				} else if(entity->animationIndexOn == 4) {
					anim  = gameState_findSplatAnimation(gameState, "cheshire_cat_standing_licking_paw7.png"); 
					entity->animationIndexOn++;
				} else if(entity->animationIndexOn == 5) {
					anim  = gameState_findSplatAnimation(gameState, "cheshire_cat_walking4.png"); 
					entity->animationIndexOn = 0;
				}
				easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, anim, EASY_ANIMATION_PERIOD);	
			}
		}
		
	}

	if(entity->type == ENTITY_TRIGGER_WITH_RIGID_BODY) {
		if(entity->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE || entity->triggerType == ENTITY_TRIGGER_FIRE_POST) {

			if(entity->chestIsOpen) {
				//do nothing				
			} else {

				Animation *fireAnimation = 0;
				if(entity->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE) {
					entity->sprite = findTextureAsset("woodFire.png");
					fireAnimation = gameState_findSplatAnimation(gameState, "woodFire5.png");
				} else {
					entity->sprite = gameState_findSplatTexture(gameState, "firePost");
					fireAnimation = gameState_findSplatAnimation(gameState, "firePostAniamted_5.png");
				}
				

				if(entity->collider1->collisions.count > 0) {
					MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
					if(info.found && gameState->lastCheckPointId != entity->T.id) {
						
						// easyConsole_addToStream(DEBUG_globalEasyConsole, "Playing ANimation");
						//turn fire on
						entity->sprite = 0;
						
						easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, fireAnimation, EASY_ANIMATION_PERIOD);	
						
						entity->chestIsOpen = true;

						ParticleSystemListItem *ps = getParticleSystem(manager);
						// ps->ps.Set.VelBias = rect3f(-0.4f, -0.1f, -1, 0.4f, 0.1f, -0.4f);
						ps->ps.Set.VelBias = rect3f(0, 0, -0.3f, 0, 0, -0.7f);
						ps->ps.Set.posBias = rect3f(-0.3f, -0.1f, 0, 0.3f, 0.1f, 0);
						ps->ps.Set.bitmapScale = 0.6f;

						easyParticles_setSystemLifeSpan(&ps->ps, 30.0f);
						setParticleLifeSpan(&ps->ps, 5.6f);

						ps->color = v4(0.3f, 0.3f, 0.3f, 1);
						ps->ps.Set.Loop = true;

						Reactivate(&ps->ps);

						ps->position = &entity->T.pos;
						ps->offset = v3(0, 0, -0.1f);

						//place fire sound
						entity->currentSound = playLocationSound(&globalLongTermArena, easyAudio_findSound("fireplace.wav"), 0, AUDIO_FOREGROUND, easyTransform_getWorldPos(&entity->T));
						EasySound_LoopSound(entity->currentSound);

						entity->currentSound->volume = 3.0f;
						entity->currentSound->innerRadius = 0.4f;
						entity->currentSound->outerRadius = 1.0f;


						//extra stuff it is a save fire
						if(entity->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE) {
							//play sound to say saved checkpoint
							playGameSound(&globalLongTermArena, gameState->gongSound, 0, AUDIO_FOREGROUND);

							//save your checkpoint state
							gameState->lastCheckPointId = entity->T.id;
							gameState->lastCheckPointSceneName = gameState->currentSceneName;

							///// Show help message
	    					//The dialog options 
	    					// Make the temporary talk node
	    					{
	        					gameState->gameDialogs.perDialogArenaMark = takeMemoryMark(&gameState->gameDialogs.perDialogArena);

	        					gameState->currentTalkText = constructDialogNode(&gameState->gameDialogs);
	        					pushTextToNode(gameState->currentTalkText, "{s: 3}Sleeping at campfires will save your progress. Once lit, you can navigate back to them.");

	    					}

	    					gameState->gameModeType = GAME_MODE_READING_TEXT;
	    					gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING;
	    					gameState->enteredTalkModeThisFrame = true;
	    					
	    					gameState->messageIndex = 0; //start at the begining
	    					easyFontWriter_resetFontWriter(&gameState->fontWriter);
							///////////////////////////////////////////////////////
	    				}

					} 
				}
			}
		}
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

				easyConsole_addToStream(DEBUG_globalEasyConsole, "Push Trasition 1");

				EasySceneTransition *transition = EasyTransition_PushTransition(transitionState, loadScene, loadSceneData, EASY_TRANSITION_CIRCLE_N64);
				gameState->gameIsPaused = true;	
			}
		}

		MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
		if(info.found) {
			if(entity->triggerType == ENTITY_TRIGGER_START_SWIM) {
				info.e->isSwimming = true;
				// playGameSound(&globalLongTermArena, gameState->waterInSound, 0, AUDIO_FOREGROUND);
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
				// playGameSound(&globalLongTermArena, gameState->waterInSound, 0, AUDIO_FOREGROUND);
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

		//////////////////////////////////////////

		V3 playerInWorldP = roundToGridBoard(easyTransform_getWorldPos(&player->T), 1);
		V3 entP_inWorld = roundToGridBoard(easyTransform_getWorldPos(&entity->T), 1);

		entP_inWorld.z = 0;


		if(!entity->aiController) {
			entity->aiController = easyAi_initController(&globalPerSceneArena);
		}	
		


		/////////////////////////////////

		//left collider
		MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found && entity->moveTimer < 0) {
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Left");
			V3 moveDirection = v3(1, 0, 0);
			V3 targetP = v3_plus(entP_inWorld, moveDirection);
			if(isDown(keyStates->gameButtons, BUTTON_RIGHT) && easyAi_hasNode(entity->aiController, targetP, entity->aiController->boardHash, true) && !isBlockOnSquare(manager, targetP, entity)) {
				//push left
				entity->moveTimer = 0;
				entity->startMovePos = entP_inWorld;
				entity->endMovePos = targetP;
				playGameSound(&globalLongTermArena, gameState->blockSlideSound, 0, AUDIO_FOREGROUND);

				ParticleSystemListItem *ps = getParticleSystem(manager);
				ps->ps.Set.VelBias = rect3f(-1, 0, -1, -0.3f, 0, -1.3f);
				ps->ps.Set.posBias = rect3f(0, -0.3f, 0, 0, 0.3f, 0);

				ps->position = &entity->T.pos;
				ps->offset = v3(-0.2f, 0, 0.5f);
				
			}
		}

		//right collider
		info = MyEntity_hadCollisionWithType(manager, entity->collider2, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found && entity->moveTimer < 0) {
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Right");
			V3 moveDirection = v3(-1, 0, 0);
			V3 targetP = v3_plus(entP_inWorld, moveDirection);
			if(isDown(keyStates->gameButtons, BUTTON_LEFT) && easyAi_hasNode(entity->aiController, targetP, entity->aiController->boardHash, true) && !isBlockOnSquare(manager, targetP, entity)) {
				//push right
				entity->moveTimer = 0;
				entity->startMovePos = entP_inWorld;
				entity->endMovePos = targetP;
				playGameSound(&globalLongTermArena, gameState->blockSlideSound, 0, AUDIO_FOREGROUND);

				ParticleSystemListItem *ps = getParticleSystem(manager);
				ps->ps.Set.VelBias = rect3f(0.3f, 0, -1, 1.f, 0, -1.3f);
				ps->ps.Set.posBias = rect3f(0, -0.3f, 0, 0, 0.3f, 0);
				ps->position = &entity->T.pos;
				ps->offset = v3(0.2f, 0, 0.5f);
				
			}
		}

		//Top collider
		info = MyEntity_hadCollisionWithType(manager, entity->collider3, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found && entity->moveTimer < 0) {
			V3 moveDirection = v3(0, -1, 0);
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Top");
			V3 targetP = v3_plus(entP_inWorld, moveDirection);
			if(isDown(keyStates->gameButtons, BUTTON_DOWN) && easyAi_hasNode(entity->aiController, targetP, entity->aiController->boardHash, true) && !isBlockOnSquare(manager, targetP, entity)) {
				//push up
				entity->moveTimer = 0;
				entity->startMovePos = entP_inWorld;
				entity->endMovePos = targetP;
				playGameSound(&globalLongTermArena, gameState->blockSlideSound, 0, AUDIO_FOREGROUND);

				ParticleSystemListItem *ps = getParticleSystem(manager);
				ps->ps.Set.VelBias = rect3f(0, 0.3f, -1, 0, 1, -1.3f);
				ps->ps.Set.posBias = rect3f(-0.3f, 0, 0, 0.3f, 0, 0);
				ps->position = &entity->T.pos;
				ps->offset = v3(0, 0.5f, 0.5f);
				
			}
		}

		//bottom collider
		info = MyEntity_hadCollisionWithType(manager, entity->collider4, ENTITY_WIZARD, EASY_COLLISION_STAY);	
		if(info.found && entity->moveTimer < 0) {
			V3 moveDirection = v3(0, 1, 0);
			V3 targetP = v3_plus(entP_inWorld, moveDirection);
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Bottom");
			if(isDown(keyStates->gameButtons, BUTTON_UP) && easyAi_hasNode(entity->aiController, targetP, entity->aiController->boardHash, true) && !isBlockOnSquare(manager, targetP, entity)) {
				//push bottom
				entity->moveTimer = 0;
				entity->startMovePos = entP_inWorld;
				entity->endMovePos = targetP;
				playGameSound(&globalLongTermArena, gameState->blockSlideSound, 0, AUDIO_FOREGROUND);

				ParticleSystemListItem *ps = getParticleSystem(manager);
				ps->ps.Set.VelBias = rect3f(0, -1.f, -1, 0, -0.3f, -1.3f);
				ps->ps.Set.posBias = rect3f(-0.3f, 0, 0, 0.3f, 0, 0);
				ps->position = &entity->T.pos;
				ps->offset = v3(0, -0.5f, 0.5f);
				
			}
		}


		if(entity->moveTimer >= 0) {
			entity->moveTimer += dt;

			float canVal = entity->moveTimer / 0.5f;

			V2 position =  lerpV3(entity->startMovePos, smoothStep01(0, canVal, 1), entity->endMovePos).xy;

			entity->T.pos.xy = position;

			if(canVal >= 1.f) {
				entity->moveTimer = -1;				
			}
		}

	}

	if(entity->type == ENTITY_SEAGULL) {
		entity->T.pos = v3_plus(entity->T.pos, v3_scale(dt, v3(-2.0f, 0, 0))); 


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
				easyMemory_zeroStruct(entityToAdd, EntityToAdd);
				entityToAdd->type = entity->typeToCreate;
				entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, randomBetween(-5, 5), 0));

				// entityToAdd->dP.y = 0;
				// entityToAdd->dP.x = -2;//randomBetween(-5, 5);
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

		if(entity->collider->collisions.count > 0 && entity->health > 0) {
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

	if(entity->type == ENTITY_ENEMY_PROJECTILE) {
		// easyConsole_addToStream(console, "updateing projectileP");

		easyConsole_pushFloat(DEBUG_globalEasyConsole, entity->rb->dP.y);

 		if(entity->collider->collisions.count > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	easyConsole_addToStream(console, "player hurt by bullet");
            	damagePlayer(manager, info.e, gameState, cam, findDamage(entity->type), easyTransform_getWorldPos(&info.e->T));
            	entity->isDead = true;
            }
		}

		entity->lifeSpanLeft -= dt;

		if(entity->lifeSpanLeft <= 0.0f) {
			entity->isDead = true;
		}
	}

	if(entity->type == ENTITY_SHOOT_TRIGGER) {

 		if(entity->collider1->collisions.count > 0) {
            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	easyConsole_addToStream(console, "player entered");

            	//spawn bullet
            	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
            	EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
            	entityToAdd->type = ENTITY_ENEMY_PROJECTILE;
            	entityToAdd->position = easyTransform_getWorldPos(&entity->T);
            	entityToAdd->position.z = -1.5f;

            	entityToAdd->dP.y = -13;
            	entityToAdd->dP.x = 0;

            	playGameSound(&globalLongTermArena, gameState->dartSound, 0, AUDIO_FOREGROUND);
            }
		}

	}

	if(entity->type == ENTITY_PLAYER_PROJECTILE) {
			
		// char str[256];
		// sprintf(str, "%f", entity->rb->inverseWeight);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, str);

 		if(entity->collider->collisions.count > 0) {
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

			setModelTransform(entitiesRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
			renderDrawQuad(entitiesRenderGroup, COLOR_LIGHT_GREY);

			gameState->tempTransform.scale.x = percent*w;
			gameState->tempTransform.pos = v3_plus(gameState->tempTransform.pos, v3_scale(-0.05f, cameraZ_axis));

			float overhang = w - percent*w;
			gameState->tempTransform.pos.x -= 0.5f*overhang;		

			setModelTransform(entitiesRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
			renderDrawQuad(entitiesRenderGroup, color);

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

		assert(&entity->animationController.parent != entity->animationController.parent.next);

		char *animationFileName = easyAnimation_updateAnimation(&entity->animationController, &gameState->animationFreeList, dt);
		sprite = findTextureAsset(animationFileName);	

		//We snap alter the width based on the aspect ratio of the sprite, this is based on that the y is the same across frames
		entity->T.scale.x = (1.0f / sprite->aspectRatio_h_over_w)*entity->T.scale.y;

		if(entity->type == ENTITY_WEREWOLF) {
			Animation *deathAnimation = gameState_findSplatAnimation(gameState, "SDeath_4.png");
			if(entity->animationController.finishedAnimationLastUpdate && entity->animationController.lastAnimationOn == deathAnimation && easyAnimation_getCurrentAnimation(&entity->animationController, deathAnimation)) {
				entity->isDead = true;
				easyConsole_addToStream(DEBUG_globalEasyConsole, "werewolf died");

				//drop any items
				// ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
				// EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
				// entityToAdd->type = ENTITY_HEALTH_POTION_1;
				// entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(0.0f, 0.0f, 0));

				// entityToAdd->dP.y = 10;
				// entityToAdd->dP.x = randomBetween(-5, 5);
			}
		}
	} else if(!DEBUG_DRAW_SCENERY_TEXTURES || entity->type == ENTITY_TERRAIN) {
		sprite = 0;
	}

	if(entity->type == ENTITY_FOG) {
		shaderProgram = &fogProgram;

		EasyCollider *col = entity->collider;
		if(col->collisions.count > 0) {
			assert(col->isTrigger);

            MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, col, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
            if(info.found) {
            	entity->chestIsOpen = false;
            	entity->lifeSpanLeft = 0;

            }

            info = MyEntity_hadCollisionWithType(manager, col, ENTITY_WIZARD, EASY_COLLISION_EXIT);	
            if(info.found) {
            	entity->chestIsOpen = true;
            	entity->lifeSpanLeft = 0;

            } 
        }

	    if(entity->lifeSpanLeft >= 0.0f) {
	    	entity->lifeSpanLeft += dt;

	    	float canVal = clamp01(entity->lifeSpanLeft / 1.0f);

	    	if(entity->chestIsOpen) {
	    		entity->colorTint.w = 1.0f - canVal;	
	    	} else {
	    		entity->colorTint.w = canVal;	
	    	}
 
	    	if(canVal >= 1.0f) {
	    		entity->lifeSpanLeft = -1.0f;
	    	}
	    } else if(!entity->chestIsOpen) {
	    	entity->colorTint = COLOR_WHITE;
	    }
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

	setModelTransform(entitiesRenderGroup, T);
	setModelTransform(shadowMapGroup, T);
		

	if(sprite && DEBUG_DRAW_SCENERY_TEXTURES && !entity->renderFirstPass && !(entity->flags & ENTITY_SHOULD_NOT_RENDER)) {
		if(sprite->isAlpha) {
			EntityRender_Alpha alphaStruct = {};
			alphaStruct.sprite = sprite;
			alphaStruct.T = T;
			alphaStruct.shader = shaderProgram;
			alphaStruct.colorTint = entity->colorTint; 

			addElementInfinteAlloc_notPointer(&gameState->alphaSpritesToRender, alphaStruct);
		} else {
			renderSetShader(entitiesRenderGroup, shaderProgram); renderDrawSprite(entitiesRenderGroup, sprite, entity->colorTint);
			renderSetShader(shadowMapGroup, &shadowMapProgram); renderDrawSprite(shadowMapGroup, sprite, entity->colorTint);
		}
		
	}
	
}




//Init entity types

static Entity *initScenery_noRigidBody(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1,  false);
    e->flags |= ENTITY_SHOULD_SAVE_ANIMATION;
    return e;
}


static Entity *initFog(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_FOG, 0, findTextureAsset("fog1.png"), COLOR_WHITE, -1,  true);
    e->T.pos.z = -0.5f;
    e->flags |= ENTITY_SHOULD_NOT_RENDER;
    // gameState->currentFogEntity = e;
    return e;
}


static Entity *initEntityCreator(GameState *gameState, EntityManager *manager, V3 worldP) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_ENTITY_CREATOR, 0, &globalWhiteTexture, COLOR_WHITE, -1,  false);
    return e;
}


static Entity *initSeagull(GameState *gameState, EntityManager *manager, V3 worldP) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SEAGULL, 0, 0, COLOR_WHITE, -1, false);
    e->T.pos.z = -15.0f;
    e->T.scale = v3(2.5f, 2.5f, 2.5f);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, &gameState->seagullAnimation, EASY_ANIMATION_PERIOD);	
    e->maxLifeSpan = 30.0f;
    e->lifeSpanLeft = e->maxLifeSpan;
    return e;
}



static Entity *initEmptyTriggerWithRigidBody(GameState *gameState, EntityManager *manager, V3 worldP, Texture *texture) {
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(0.3f, 0.3f), gameState, ENTITY_TRIGGER_WITH_RIGID_BODY, 0, texture, COLOR_WHITE, -1, true);
    e->triggerType = ENTITY_TRIGGER_NULL;
    e->collider1->dim2f = v2(2, 2);
    e->T.pos.z = -0.5f;
    e->flags |= (int)ENTITY_SHOULD_SAVE_ANIMATION;
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
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, true);
	e->flags |= ENTITY_SHOULD_SAVE_ANIMATION;
	return e;
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

static Entity *initAiAnimation(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_AI_ANIMATION, 0, 0, COLOR_WHITE, -1, false);
	e->subEntityType |= ENTITY_SUB_TYPE_CAT;
	return e;
}

static Entity *initSheild(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SHEILD, 0, 0, COLOR_WHITE, -1, true);
	e->model =  findModelAsset_Safe("shield.obj"); 
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
	e->flags |= ENTITY_SHOULD_SAVE_ANIMATION;
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

	e->innerRadius = 0.2f;
	e->outerRadius = 9.0f;


	return e;
}

static Entity *initChest(GameState *gameState, EntityManager *manager, V3 worldP) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_CHEST, 0, findTextureAsset("chest.png"), COLOR_WHITE, -1, true);
	float w = 0.7f;
	e->T.scale = v3(w, e->sprite->aspectRatio_h_over_w*w, 1);
	e->T.pos.z = -0.37;

	e->collider->dim2f.x = 0.7f;
	e->collider->dim2f.y = 0.7f;

	e->collider1->offset.x = 0;
	e->collider1->offset.y = -0.7f;
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

static Entity *initEnemyProjectile(GameState *gameState, EntityManager *manager, V3 worldP) {
	Texture *dartT = findTextureAsset("dart.png");
	Entity *e = initEntity(manager, 0, worldP, v2(0.1f, 0.1f), v2(1.5f, 1.5f), gameState, ENTITY_ENEMY_PROJECTILE, 0, dartT, COLOR_WHITE, -1, true);

	e->T.scale.y = e->T.scale.x*dartT->aspectRatio_h_over_w;
	e->lifeSpanLeft = 3.0f;
	return e;
}



static Entity *initShootTrigger(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SHOOT_TRIGGER, 0, splatTexture, COLOR_WHITE, -1, true);
	return e;
}


static Entity *initPushRock(GameState *gameState, EntityManager *manager, V3 worldP) {
	float blockDim = 1.0;

	Texture *crateTexture = findTextureAsset("crate1.png");

	if(crateTexture->normalMapId <= 0) {
		Texture *normalT = findTextureAsset("crate1_n.png");
		crateTexture->normalMapId = normalT->id;
	}

	Entity *e = initEntity(manager, 0, worldP, v2(blockDim, blockDim), v2(0.5f, 0.5f), gameState, ENTITY_BLOCK_TO_PUSH, gameState->inverse_weight, crateTexture, COLOR_WHITE, -1, true);
		
	e->T.pos.z = -0.2f;
	//Add triggers to see if player is pushing	

	float triggerSize = 0.8f;

	float offset = 0.5f;

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
		case ENTITY_TRIGGER_WITH_RIGID_BODY: {
			newEntity = initEmptyTriggerWithRigidBody(gameState, manager, position, splatTexture);
			newEntity->triggerType = triggerType;
		} break;
		case ENTITY_SHOOT_TRIGGER: {
			newEntity = initShootTrigger(gameState, manager, position, splatTexture);
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
        case ENTITY_AI_ANIMATION: {
			newEntity = initAiAnimation(gameState, manager, position);
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
        case ENTITY_FOG: {
        	newEntity = initFog(gameState, manager, position, splatTexture);
        } break;
        case ENTITY_ENEMY_PROJECTILE: {
        	newEntity = initEnemyProjectile(gameState, manager, position);
        } break;
		default: {
			//do nothing
		}
	}

	return newEntity;
}