/*

TODO: 

Save the doors correctly when they are open and if they have been activated

Clear sokobon puzzles with a key that prompts you. Have clear place of to reset to and where the bricks start at

Fade out when you press the button



*/


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


//// Entity Trigger Types ////////////
#define MY_ENTITY_ENEMY_TYPE(FUNC) \
FUNC(ENEMY_SKELETON)\
FUNC(ENEMY_BOLT_BALL)\

typedef enum {
    MY_ENTITY_ENEMY_TYPE(ENUM)
} EntityEnemyType;

static char *MyEntity_EnemyTypeStrings[] = { MY_ENTITY_ENEMY_TYPE(STRING) };

typedef enum {
	ENTITY_ANIMATION_IDLE,
	ENTITY_ANIMATION_WALK,
	ENTITY_ANIMATION_ATTACK,
	ENTITY_ANIMATION_HURT,
	ENTITY_ANIMATION_DIE,
} EntityAnimationState;


static Animation *getAnimationForAnimal(GameState *gameState, EntityAnimationState state, EntityAnimalType type) {
	Animation *animation = 0;
	if(false) {

	} else if(type == ENTITY_ANIMAL_CHICKEN) {
		switch(state) {
			case ENTITY_ANIMATION_IDLE: {
				animation = gameState_findSplatAnimation(gameState, "chicken_3.png");
			} break;
		}
		
	} 

	return animation;
}

static Animation *getAnimationForEnemy(GameState *gameState, EntityAnimationState state, EntityEnemyType type) {
	Animation *animation = 0;
	if(false) {

	} else if(type == ENEMY_SKELETON) {
		switch(state) {
			case ENTITY_ANIMATION_IDLE: {
				animation = gameState_findSplatAnimation(gameState, "SIdle_4.png");
			} break;
			case ENTITY_ANIMATION_WALK: {
				animation = gameState_findSplatAnimation(gameState, "SWalk_4.png");
			} break;
			case ENTITY_ANIMATION_ATTACK: {
				animation = gameState_findSplatAnimation(gameState, "SAttack_8.png");
			} break;
			case ENTITY_ANIMATION_HURT: {
				animation = gameState_findSplatAnimation(gameState, "SHit_4.png");
			} break;
			case ENTITY_ANIMATION_DIE: {
				animation = gameState_findSplatAnimation(gameState, "SDeath_4.png");
			} break;
		}
		
	} else if(type == ENEMY_BOLT_BALL) {
		switch(state) {
			case ENTITY_ANIMATION_ATTACK: 
			case ENTITY_ANIMATION_HURT: 
			case ENTITY_ANIMATION_DIE: {
				//bogus animation
				animation = &gameState->wizardForward;
			} break;
			default: {
				animation = gameState_findSplatAnimation(gameState, "boltBall4.png");
			}
		}
	}

	return animation;
}


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
	bool isFalling; //For player falling through the floor

	EntityEnemyType enemyType; 

	//FOr empty triggers that do stuff
	EntityTriggerType triggerType;
	char *levelToLoad; //for triggers that load levels

	//Location Sound type to retrieve string
	EntityLocationSoundType locationSoundType;

	///NOTE: CHest information
	bool chestIsOpen;
	ChestType chestType;
	/////

	EntityAnimalType animalType;

	//NOTE: For things that are have a switch that gets turns on and stay on. Using for a door when you exit it it closes.
	bool isActivated;

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

	//NOTE: For loading to other scenes through a door
	int partnerId;
	V2 moveDirection;
	//////////////////////


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
	int footstepAt;
	float footStepTimer;

	//////////////////

	float lightIntensity;
	V3 lightColor;

	float innerRadius;
	float outerRadius;

	bool renderFirstPass; //Render before entities like terrain so the doesn't affect the depth buffer ordering 

	PlayingSound *currentSound;

	//NOTE: For the button that needs to know things are on top of it
	int refCount;


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

	int particleSystemIndexToEndOnDelete;
	float enemyMoveSpeed;


} Entity;

///////////////////////////// Save State Entity ///////////////////////////////////


static EntitySaveStateData saveStateEntity_makeSaveEntity(Entity *e) {
	EntitySaveStateData result = {};

	result.pos = e->T.pos;

	result.isOpen = e->chestIsOpen;
	result.isDead = e->isDead;
	result.isActivated = e->isActivated;
	result.health = e->health;
	result.stamina = e->stamina;

	if(e->collider) {
		result.colliderActive = e->collider->isActive;	
	}
	

	return result;
}


static SaveStateEntity *saveStateEntity_findEntity(GameState *gameState, int id, char *sceneName) {
	SaveStateEntity *result = 0;
	for(int i = 0; i < gameState->playerSaveProgress.saveStateEntities.count && !result; ++i) {
		SaveStateEntity *saveEntity = getElementFromAlloc(&gameState->playerSaveProgress.saveStateEntities, i, SaveStateEntity);

		if(easyString_stringsMatch_nullTerminated(sceneName, saveEntity->sceneName) && saveEntity->id == id) {
			result = saveEntity;
			break;
		}
	}

	return result;
}

static void saveStateEntity_addEntity(GameState *gameState, int id, char *sceneName, EntitySaveStateData data) {
	SaveStateEntity entity = {};

	entity.id = id;

	easyMemory_zeroArray(entity.sceneName);

	nullTerminateBuffer(entity.sceneName, sceneName, easyString_getSizeInBytes_utf8(sceneName));
	entity.data = data;

	addElementInfinteAlloc_notPointer(&gameState->playerSaveProgress.saveStateEntities, entity);
}


//NOTE: Helper function to combine the ones above to make it easier
static void saveStateEntity_addOrEditEntity(GameState *gameState, int id, char *sceneName, Entity *entity) {
	SaveStateEntity *saveEntity = saveStateEntity_findEntity(gameState, id, sceneName);

	EntitySaveStateData data = saveStateEntity_makeSaveEntity(entity);

	if(saveEntity) {
		saveEntity->data = data;

	} else {
		saveStateEntity_addEntity(gameState, id, sceneName, data);	
	}
}




#pragma pack(push, 1)
typedef struct {
    u8 magicNumber;
    u8 version;
    u32 sizeOfData;
    u32 offsetToData;
    u32 offsetToPlayerData;
} SaveProgressFileFormat;
#pragma pack(pop)

#define SAVE_FILE_MAGIC_NUMBER ('S' | 'A' | 'V' | 'E')

static void playerGameState_saveProgress(GameState *gameState, EntityManager *manager, char *filePath_) {

    char *filePath = concatInArena(filePath_, concatInArena(platformGetTimeStamp_withSlashAtFront(&globalPerFrameArena), ".save", &globalPerFrameArena), &globalPerFrameArena);

    SaveProgressFileFormat format = {};

    //NOTE: Fill out player info 
    {
	    Entity *e = (Entity *)manager->player;

	    gameState->playerSaveProgress.playerInfo.isValid = true; 
	    gameState->playerSaveProgress.playerInfo.position = easyTransform_getWorldPos(&e->T);
	    gameState->playerSaveProgress.playerInfo.health = e->health;
	    gameState->playerSaveProgress.playerInfo.stamina = e->stamina;

	    nullTerminateBuffer(gameState->playerSaveProgress.playerInfo.sceneName, gameState->currentSceneName, easyString_getSizeInBytes_utf8(gameState->currentSceneName));
	}
    /////////////////////////

    format.magicNumber = SAVE_FILE_MAGIC_NUMBER;
    format.version = 1;
    format.sizeOfData = gameState->playerSaveProgress.saveStateEntities.count * gameState->playerSaveProgress.saveStateEntities.sizeOfMember;
    
    format.offsetToData = sizeof(SaveProgressFileFormat);

    format.offsetToPlayerData = format.offsetToData + format.sizeOfData;

    ///////////////////////************ Write the file to disk *************////////////////////
    game_file_handle handle = platformBeginFileWrite(filePath);
    if(!handle.HasErrors) {
        platformWriteFile(&handle, &format, sizeof(SaveProgressFileFormat), 0);

        if(gameState->playerSaveProgress.saveStateEntities.count > 0) {
        	platformWriteFile(&handle, gameState->playerSaveProgress.saveStateEntities.memory, format.sizeOfData, format.offsetToData);
    	}

    	platformWriteFile(&handle, &gameState->playerSaveProgress.playerInfo, sizeof(SaveEntity_PlayerInfo), format.offsetToPlayerData);
    	

    } else {
        easyConsole_addToStream(DEBUG_globalEasyConsole, "Can't save entity file. Handle has Errors.");             
    }

    platformEndFile(handle);    

}

static void playerGameState_loadProgress(GameState *gameState, EntityManager *manager, char *filePath_) {
	char *saveFileTypes[] = {"save"};
	FileNameOfType saveFiles = getDirectoryFilesOfType(concatInArena(filePath_, "\\", &globalPerFrameArena), saveFileTypes, arrayCount(saveFileTypes));
		
	if(saveFiles.count > 0) {
		//NOTE: Just get the most recent save file
		FileContents contents = platformReadEntireFile(saveFiles.names[saveFiles.count - 1], false);

		if(contents.valid) {
			SaveProgressFileFormat *format = (SaveProgressFileFormat *)contents.memory;

			assert(contents.fileSize == (sizeof(SaveProgressFileFormat) + format->sizeOfData + sizeof(SaveEntity_PlayerInfo)));

			assert(format->magicNumber == SAVE_FILE_MAGIC_NUMBER);

			if(format->version == 1) {
				SaveStateEntity *entities = (SaveStateEntity *)(((u8 *)contents.memory) + format->offsetToData);				
				int entityCount = format->sizeOfData / sizeof(SaveStateEntity); 

				//NOTE: Loop through entities and add them to the save state
				for(int i = 0; i < entityCount; ++i) {
					SaveStateEntity saveEntity = entities[i];

 					addElementInfinteAlloc_notPointer(&gameState->playerSaveProgress.saveStateEntities, saveEntity);
				}

				//Get the player info
				SaveEntity_PlayerInfo *playerInfo = (SaveEntity_PlayerInfo *)(((u8 *)contents.memory) + format->offsetToPlayerData);
				gameState->playerSaveProgress.playerInfo = *playerInfo;

			}
		}

		//NOTE: Free the file contents
		easyFile_endFileContents(&contents);


		//NOTE: Free all the file names
		for(int i = 0; i < saveFiles.count; ++i) {
			free(saveFiles.names[i]);
		}
	}


}


typedef struct {
	EntityType type;
	V3 position;
	V3 dP;
	float rotation;
	EasyTransform *parentT;
	SubEntityType subType;
	EntityAnimalType animalType;
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


#define ENTITY_MULTI_COLLISION_COUNT 64
typedef struct {
	int count;

    Entity *e[ENTITY_MULTI_COLLISION_COUNT];
    EasyCollisionType collisionType[ENTITY_MULTI_COLLISION_COUNT];

    bool found;
} MyEntity_CollisionInfo_Multi;

static MyEntity_CollisionInfo_Multi MyEntity_hadCollisionWithType_multi(EntityManager *manager, EasyCollider *collider, EntityType type, EasyCollisionType colType) {
    
    MyEntity_CollisionInfo_Multi result = {};
    result.found = false;
    
    for(int i = 0; i < collider->collisions.count; ++i) {
    	EasyCollisionInfo *info = getElementFromAlloc(&collider->collisions, i, EasyCollisionInfo);

        if(info->type == colType) {
            int id = info->objectId; 
            bool foundEntity = false;
            for(int j = 0; j < manager->entities.count && !foundEntity; ++j) {
                Entity *e = (Entity *)getElement(&manager->entities, j);
                if(e) { //can be null
                    if(result.count < arrayCount(result.e) && e->T.id == id && e->type == type) {
                        int index = result.count++;
                        result.e[index] = e;
                        result.collisionType[index] = info->type;
                        result.found = true;
                        foundEntity = true;
                    }
                }
            }
        }
    } 
    
    return result;
}

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

static ParticleSystemListItem *getParticleSystem(EntityManager *m, Entity *entity) {
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

	entity->particleSystemIndexToEndOnDelete = arrayInfo.absIndex;
	result->position = &entity->T.pos;

	return result;	
}


static void renderKeyPromptHover(GameState *gameState, Texture *keyTexture, Entity *entity, float dt, bool useScaleY, RenderGroup *group) {
	if(!gameState->gameIsPaused) {
		// easyConsole_addToStream(DEBUG_globalEasyConsole, "RENDER PROMPT");
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

	if(type == ENTITY_ENEMY) { return 3; }

	if(type == ENTITY_WIZARD) { return 10; }

	return 10.0f; //default value
}

static inline float findMaxHealth(EntityType type) {

	if(type == ENTITY_ENEMY) { return 10; }

	if(type == ENTITY_WIZARD) { return 10; }

	return 10.0f; //default value
}


static inline float findDamage(EntityType type) {

	if(type == ENTITY_SWORD) { return 1; }

	if(type == ENTITY_ENEMY) { return 1; }

	if(type == ENTITY_ENEMY_PROJECTILE) { return 1; }

	if(type == ENTITY_PLAYER_PROJECTILE) { return 2; }

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

static inline void hurtEnemy(GameState *gameState, EntityManager *manager, float damage, Entity *enemy, V3 worldP_ofEntityAttacking) {
	easyConsole_addToStream(DEBUG_globalEasyConsole, "Enemy got hurt");

	enemy->flashHurtTimer = 0;
	enemy->healthBarTimer = 0;

	V3 worldP = easyTransform_getWorldPos(&enemy->T);

	//Knock the enemy back
	V2 dir = normalizeV2(v2_minus(worldP.xy, worldP_ofEntityAttacking.xy));
	enemy->rb->accumForceOnce.xy = v2_plus(enemy->rb->accumForceOnce.xy, v2_scale(gameState->werewolf_knockback_distance, dir));
	///

	//Damage the enemy
	if(enemy->health > 0.0f) {
		enemy->health -= damage;

		createDamageNumbers(manager, damage, v3_plus(worldP, v3(0, 0, -1.0f)));	

		easyConsole_addToStream(DEBUG_globalEasyConsole, "Skeleton Hit 1");
		if(enemy->type == ENTITY_ENEMY) {
			easyAnimation_emptyAnimationContoller(&enemy->animationController, &gameState->animationFreeList);
			easyAnimation_addAnimationToController(&enemy->animationController, &gameState->animationFreeList, getAnimationForEnemy(gameState, ENTITY_ANIMATION_HURT, enemy->enemyType), EASY_ANIMATION_PERIOD);	
			if(enemy->health > 0.0f) {
				easyAnimation_addAnimationToController(&enemy->animationController, &gameState->animationFreeList, getAnimationForEnemy(gameState, ENTITY_ANIMATION_IDLE, enemy->enemyType), EASY_ANIMATION_PERIOD);		
			}
			
			easyConsole_addToStream(DEBUG_globalEasyConsole, "Skeleton Hit");

			assert(easyAnimation_getCurrentAnimation(&enemy->animationController, getAnimationForEnemy(gameState, ENTITY_ANIMATION_HURT, enemy->enemyType)));
		}
		
	}

	//See if the enemey is dead
	if(enemy->health <= 0.0f) {

		easyConsole_addToStream(DEBUG_globalEasyConsole, "ENEMY DEAD");
		// enemy->isDead = true;

		easyAnimation_addAnimationToController(&enemy->animationController, &gameState->animationFreeList, getAnimationForEnemy(gameState, ENTITY_ANIMATION_DIE, enemy->enemyType), EASY_ANIMATION_PERIOD);	

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

static inline void player_useAttackItem(GameState *gameState, EntityManager *manager, float damage, Entity *entity) {
	assert(entity->collider1->isTrigger);
    if(entity->collider1->collisions.count > 0) {
    	
    	easyConsole_addToStream(DEBUG_globalEasyConsole, "had collision");

        //Check if it got hurt 
        MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_ENEMY, EASY_COLLISION_STAY);	
        if(info.found) {
        	// easyConsole_addToStream(DEBUG_globalEasyConsole, "In bounds");
        	Entity *enemy = info.e;
        	hurtEnemy(gameState, manager, damage, enemy, easyTransform_getWorldPos(&entity->T));

        	
        }
	}
}


static bool playerUsedKey(GameState *gameState, AppKeyStates *keyStates) {
	bool result = false;
	bool pressedSpace = false;


	int index = -1;
	if(gameState->playerHolding[0] && gameState->playerHolding[0]->type == ENTITY_KEY) {
		index = 0;
		pressedSpace = wasPressed(keyStates->gameButtons, BUTTON_Z) && !gameState->gameIsPaused;
	}

	if(gameState->playerHolding[1] && gameState->playerHolding[1]->type == ENTITY_KEY) {
		index = 1;
		pressedSpace = wasPressed(keyStates->gameButtons, BUTTON_X) && !gameState->gameIsPaused;
	}

	if(index >= 0 && pressedSpace) {
		result = true; 
	}

	return result;
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
		case ENTITY_BOMB: {

			float offsetX = 0;
			float offsetY = 0;

			if(entity->direction == ENTITY_DIRECTION_LEFT) {
				offsetX = -0.5f;
			} else if(entity->direction == ENTITY_DIRECTION_RIGHT) {
				offsetX = 0.5f;
			} else if(entity->direction == ENTITY_DIRECTION_UP) {
				offsetY = 0.5f;
			} else if(entity->direction == ENTITY_DIRECTION_DOWN) {
				offsetY = -0.5f;
			}

			ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
			EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
			easyMemory_zeroStruct(entityToAdd, EntityToAdd);
			entityToAdd->type = ENTITY_BOMB;
			entityToAdd->position = v3_plus(easyTransform_getWorldPos(&entity->T), v3(offsetX, offsetY, 0));
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

	assert(manager->entitiesToDeleteForFrame.count == 0);
	assert(manager->entities.count == 0);
	assert(manager->entitiesToAddForFrame.count == 0);
	assert(manager->damageNumbers.count == 0);
	assert(manager->activeParticleSystems.count == 0);

	

	EasyPhysics_emptyPhysicsWorld(physicsWorld);
	
	manager->lastEntityIndex = 0;
	manager->player = 0;
}

typedef struct {
	GameWeatherState *weatherState;
	GameState *gameState;

	float hoursToSleep;
} EntitySleepInBedData;

static void sleepInBed(void *data_) {
	EntitySleepInBedData *data = (EntitySleepInBedData *)data_;

	// easyConsole_pushFloat(DEBUG_globalEasyConsole, data->weatherState->timeOfDay);


	//NOTE: add hours to weather state
	data->weatherState->timeOfDay += data->hoursToSleep/24.f;

	while(data->weatherState->timeOfDay > 1.f) {
		data->weatherState->timeOfDay -= 1.f;
	}


	easyConsole_pushFloat(DEBUG_globalEasyConsole, data->weatherState->timeOfDay);


	data->gameState->gameIsPaused = false; 

	//////////////
	free(data_);
}


typedef struct {
	GameState *gameState;
	EntityManager *manager;	
	char *sceneToLoad;
	EditorState *editorState;
	GameWeatherState *weatherState;
	int partnerId;
	V2 moveDirection;

	EasyCamera *camera;
} EntityLoadSceneData;

static void loadScene(void *data_) {
	EntityLoadSceneData *data = (EntityLoadSceneData *)data_;

	if(gameScene_doesSceneExist(data->sceneToLoad)) {
		letGoOfSelectedEntity(data->editorState);

		entityManager_emptyEntityManager(data->manager, &data->gameState->physicsWorld);

		gameScene_loadScene(data->gameState, data->manager, data->sceneToLoad, data->weatherState);


		Entity *partnerEntity = findEntityById(data->manager, data->partnerId);
		if(partnerEntity) {
			V3 pos = easyTransform_getWorldPos(&partnerEntity->T);
			Entity *player = (Entity *)data->manager->player;
			player->T.pos.xy = pos.xy;

			setCameraPosition(data->camera, pos);
		}

		data->gameState->preventSceneLoadTimer = 0;
		data->gameState->moveDirectionAfterSceneLoad = data->moveDirection;
		
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

static void entity_turnFireSavePlaceOn(GameState *gameState, EntityManager *manager, Entity *entity, bool prewarm) {
	Animation *fireAnimation = 0;
	if(entity->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE) {
		entity->sprite = findTextureAsset("woodFire.png");
		fireAnimation = gameState_findSplatAnimation(gameState, "woodFire5.png");
	} else {						
		entity->sprite = gameState_findSplatTexture(gameState, "firePost");
		fireAnimation = gameState_findSplatAnimation(gameState, "firePostAniamted_5.png");	
		
	}

	entity->sprite = 0;

	easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, fireAnimation, EASY_ANIMATION_PERIOD);	
	
	entity->chestIsOpen = true;

	ParticleSystemListItem *ps = getParticleSystem(manager, entity);
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

	if(prewarm) {
		prewarmParticleSystem(&ps->ps, v3(0, 0, 0));
	}


	//place fire sound
	entity->currentSound = playLocationSound(&globalLongTermArena, easyAudio_findSound("fireplace.wav"), 0, AUDIO_FOREGROUND, easyTransform_getWorldPos(&entity->T));
	EasySound_LoopSound(entity->currentSound);

	entity->currentSound->volume = 3.0f;
	entity->currentSound->innerRadius = 0.4f;
	entity->currentSound->outerRadius = 1.0f;
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

	entity->animalType = ENTITY_ANIMAL_NULL;

	entity->travelTimer = -1.0f;
	
	entity->layer = layer;
	entity->sprite = sprite;
	entity->lifeSpanLeft = -1.0f;
	entity->type = type;
	entity->rotation = 0;
	entity->enemyMoveSpeed = 2;
	entity->shieldInUse = false;
	entity->isDeleted = false;
	entity->tBob = 0;
	entity->chestIsOpen = false;
	entity->isActivated = false;

	entity->innerRadius = 0.2f;
	entity->outerRadius = 4.0f;

	entity->footStepTimer = 0;

	entity->lightIntensity = 2.0f;
	entity->lightColor = v3(1, 0.5f, 0);

	entity->flashHurtTimer = -1.0f;
	entity->refCount = 0;

	entity->particleSystemIndexToEndOnDelete = -1;


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

	if(type == ENTITY_ENEMY) {
		entity->flags |= (u64)ENTITY_SHOW_HEALTH_BAR;
	}

	if(type == ENTITY_PLAYER_PROJECTILE || type == ENTITY_HEALTH_POTION_1 || type == ENTITY_SEAGULL || type == ENTITY_BOMB) {

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
		
		if(type == ENTITY_HEALTH_POTION_1 || type == ENTITY_SIGN || type == ENTITY_ENEMY || type == ENTITY_WIZARD || type == ENTITY_HORSE || type == ENTITY_CHEST || type == ENTITY_SHOOT_TRIGGER || type == ENTITY_TRIGGER_WITH_RIGID_BODY || type == ENTITY_PLAYER_PROJECTILE || type == ENTITY_BOMB) { 
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
			case ENTITY_BOMB: {
				entity->collider->layer = EASY_COLLISION_LAYER_WORLD;
			} break;
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
				//NOTE: This also has a collider1 which the player can choose to pick up
				entity->collider->layer = EASY_COLLISION_LAYER_PLAYER_BULLET;
			} break;
			case ENTITY_ANIMAL: {
				entity->collider->layer = EASY_COLLISION_LAYER_WORLD;
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
			case ENTITY_ENEMY: {
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

static void activateDoor(Entity *entity) {
	entity->isActivated = true;
	entity->moveTimer = 0;
	entity->chestIsOpen = false;
	entity->collider->isActive = true;

}


static void tryPlayFootstep(Entity *entity) {
	if(entity->footStepTimer == 0) {

		char *files[3] = { "footstep_1.wav", "footstep_2.wav", "footstep_3.wav" }; 

		easyConsole_addToStream(DEBUG_globalEasyConsole, files[entity->footstepAt]);

		PlayingSound *s = playGameSound(&globalLongTermArena, easyAudio_findSound(files[entity->footstepAt++]), 0, AUDIO_FOREGROUND);

		if(entity->footstepAt >= 2) {
			entity->footstepAt = 0;
		}

		s->volume = 1.5f;

		entity->footStepTimer = 0.35f + randomBetween(-0.1f, 0.1f);
	}
}


////////////////////////////////////////////////////////////////////


void updateEntity(EntityManager *manager, Entity *entity, GameState *gameState, float dt, AppKeyStates *keyStates, EasyConsole *console, EasyCamera *cam, Entity *player, bool isPaused, V3 cameraZ_axis, EasyTransitionState *transitionState, EasySound_SoundState *soundState, EditorState *editorState, RenderGroup *shadowMapGroup, RenderGroup *entitiesRenderGroup, GameWeatherState *weatherState, char *engine_saveFilePath) {

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
		
		float walkModifier = 1;
		Animation *idleAnimation = 0;
		// if(easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardIdle) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRun) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardBottom) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardRight) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardLeft) || easyAnimation_getCurrentAnimation(&entity->animationController, &gameState->wizardJump))
		{
			
			

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

			if(entity->footStepTimer > 0) {
				entity->footStepTimer -= dt;

				if(entity->footStepTimer < 0) {
					entity->footStepTimer = 0;
				}
			}


			if(!entity->isFalling) {
				if(isDown(keyStates->gameButtons, BUTTON_LEFT) && !isPaused) {
					entity->rb->accumForce.x += -gameState->walkPower*walkModifier;

					if(entity->isSwimming) {
						animToAdd = &gameState->wizardSwimLeft;
					} else {
						animToAdd = &gameState->wizardLeft;
						idleAnimation = &gameState->wizardIdleLeft;	
					}

					tryPlayFootstep(entity);

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

					tryPlayFootstep(entity);

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
					tryPlayFootstep(entity);

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

					tryPlayFootstep(entity);

					entity->collider1->offset.x = 0;
					entity->collider1->offset.y = -maxOffsetValue;

					entity->collider1->dim2f = v2(minDim, maxDim);

					entity->direction = ENTITY_DIRECTION_DOWN;
				}
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

		//NOTE: Fall through the floor
		if(entity->isFalling && !gameState->gameIsPaused) {
			entity->lifeSpanLeft -= dt;
			if(entity->lifeSpanLeft <= 0.f) {
				gameState->gameModeType = GAME_MODE_GAME_OVER;
			}
			entity->rb->accumForce.z += 500;
		}

		//NOTE: On a level load we move to the partner id and walk for a bit and run a timer to prevent going back in the door we came out of 
		if(gameState->preventSceneLoadTimer >= 0) {
			entity->rb->accumForce.xy = v2_scale(gameState->walkPower*walkModifier, gameState->moveDirectionAfterSceneLoad);

			gameState->preventSceneLoadTimer += dt;

			float canVal = gameState->preventSceneLoadTimer / 0.5f;

			if(gameState->moveDirectionAfterSceneLoad.x > 0) {
				entity->direction = ENTITY_DIRECTION_RIGHT;
				animToAdd = &gameState->wizardRight;
				idleAnimation = &gameState->wizardIdleRight;	
			} else if(gameState->moveDirectionAfterSceneLoad.x < 0) {
				entity->direction = ENTITY_DIRECTION_LEFT;
				animToAdd = &gameState->wizardLeft;
				idleAnimation = &gameState->wizardIdleLeft;	

			} else if(gameState->moveDirectionAfterSceneLoad.y < 0) {
				entity->direction = ENTITY_DIRECTION_DOWN;
				animToAdd = &gameState->wizardBottom;
				idleAnimation = &gameState->wizardIdleBottom;	
			} else if(gameState->moveDirectionAfterSceneLoad.y > 0) {
				entity->direction = ENTITY_DIRECTION_UP;
				animToAdd = &gameState->wizardForward;
				idleAnimation = &gameState->wizardIdleForward;
			}

			if(canVal >= 1) {
				gameState->preventSceneLoadTimer = -1;
			} 
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

		//NOTE: CAST A SPELL
		if(wasPressed(keyStates->gameButtons, BUTTON_C)) {
			//spawn bullet
			ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToAddForFrame);
			EntityToAdd *entityToAdd = (EntityToAdd *)arrayInfo.elm;
			easyMemory_zeroStruct(entityToAdd, EntityToAdd);

			entityToAdd->type = ENTITY_PLAYER_PROJECTILE;
			entityToAdd->position = easyTransform_getWorldPos(&entity->T);
			entityToAdd->position.z = -0.5f;

			playGameSound(&globalLongTermArena, gameState->bowArrowSound, 0, AUDIO_FOREGROUND);

			V2 shootDirection = v2_minus(v2(keyStates->mouseP_01.x, 1.0f - keyStates->mouseP_01.y), v2(0.5f, 0.5f));

			float shootPower = 7.0f;
			shootDirection = v2_scale(shootPower, normalizeV2(shootDirection));

			entityToAdd->dP.xy = shootDirection;
			entityToAdd->rotation = ATan2_0toTau(shootDirection.y, shootDirection.x) + PI32/2;

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

		// if(entity->rb->isGrounded && wasPressed(keyStates->gameButtons, BUTTON_SPACE) && !isPaused) {
		// 	entity->rb->accumForceOnce.y += gameState->jumpPower;
		// 	animToAdd = &gameState->wizardJump;
		// }

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

	            				//NOTE: Update the chest state in the save file so the chest is open when the player comes back here on new scene load
	            				saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);

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

	

	if(entity->type == ENTITY_ENEMY) {

		if(entity->enemyType == ENEMY_BOLT_BALL && entity->particleSystemIndexToEndOnDelete < 0) {
			//add particle effects if haven't
			ParticleSystemListItem *ps = getParticleSystem(manager, entity);

			// ps->ps.Set.VelBias = rect3f(-0.4f, -0.1f, -1, 0.4f, 0.1f, -0.4f);
			ps->ps.Set.VelBias = rect3f(-0.4f, 0, -1.f, 0.4f, 0, -1.7f);
			ps->ps.Set.posBias = rect3f(-0.3f, -0.1f, 0, 0.3f, 0.1f, 0);
			ps->ps.Set.bitmapScale = 0.6f;
			removeAllParticleBitmaps(&ps->ps.Set);

			pushParticleBitmap(&ps->ps.Set, findTextureAsset("spark_04.png"), "particle1");
			pushParticleBitmap(&ps->ps.Set, findTextureAsset("spark_05.png"), "particle1");

			easyParticles_setSystemLifeSpan(&ps->ps, 30.0f);
			setParticleLifeSpan(&ps->ps, 1.6f);

			entity->currentSound = playLocationSound(&globalLongTermArena, easyAudio_findSound("electricty.wav"), 0, AUDIO_FOREGROUND, easyTransform_getWorldPos(&entity->T));
			EasySound_LoopSound(entity->currentSound);

			entity->currentSound->volume = 3.0f;
			entity->currentSound->innerRadius = 0.4f;
			entity->currentSound->outerRadius = 4.f;

			ps->color = COLOR_GREEN;
			ps->ps.Set.Loop = true;

			Reactivate(&ps->ps);

			ps->offset = v3(0, 0, -0.1f);

		}


		if(!entity->aiController) {
			entity->aiController = easyAi_initController(&globalPerSceneArena);
		}	

		bool isDying = easyAnimation_getCurrentAnimation(&entity->animationController, getAnimationForEnemy(gameState, ENTITY_ANIMATION_DIE, entity->enemyType));
		bool isHurt = easyAnimation_getCurrentAnimation(&entity->animationController, getAnimationForEnemy(gameState, ENTITY_ANIMATION_HURT, entity->enemyType)) || isDying;

		//TODO: make this an array for all enemy types 

		V3 entP = easyTransform_getWorldPos(&entity->T);

		if(entity->currentSound) {
			entity->currentSound->location = entP;
		}



		if(!isHurt) {
			// easyConsole_addToStream(DEBUG_globalEasyConsole, "werewolf is not hurt");

			V3 worldP = easyTransform_getWorldPos(&player->T);
			
			Animation *animToAdd = 0;

			float werewolfMoveSpeed = gameState->werewolf_attackSpeed;

			//WEREWOLF ATTACKING//
			if(entity->stamina < entity->maxStamina) {
				werewolfMoveSpeed = gameState->werewolf_restSpeed;
			}

			V3 playerInWorldP = roundToGridBoard(worldP, 1);
			V3 entP_inWorld = roundToGridBoard(entP, 1);

			if(entity->subEntityType & ENEMY_IGNORE_PLAYER) {
				werewolfMoveSpeed = entity->enemyMoveSpeed;
				worldP = playerInWorldP = entity->aiController->searchBouys[entity->aiController->bouyIndexAt];
				float searchDist = 0.4f;
				if(getLengthSqrV3(v3_minus(playerInWorldP, entP_inWorld)) < sqr(searchDist)) {

					entity->rb->dP.xy = v2(0, 0);
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

				animToAdd = getAnimationForEnemy(gameState, ENTITY_ANIMATION_WALK, entity->enemyType);

				entity->rb->dP.xy = v2_scale(werewolfMoveSpeed, dir);
			} else {
				animToAdd = getAnimationForEnemy(gameState, ENTITY_ANIMATION_IDLE, entity->enemyType);
			}


			V3 diff = v3_minus(worldP, entP);

			V2 dir = normalizeV2(diff.xy);


			//Enemy HURT PLAYER//
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
    		easyConsole_addToStream(DEBUG_globalEasyConsole, "enemy is hurt");


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
		if(entity->triggerType == ENTITY_TRIGGER_OPEN_DOOR_WITH_BUTTON || entity->triggerType == ENTITY_TRIGGER_OPEN_DOOR_WITH_BUTTON_WITH_TRIGGER_CLOSE) {

			//NOTE: This is the Door

			float zOffset = 1.f;
			if(!entity->chestIsOpen) {
				zOffset = 0;
			}

			if(entity->triggerType == ENTITY_TRIGGER_OPEN_DOOR_WITH_BUTTON_WITH_TRIGGER_CLOSE && !entity->isActivated) {
				zOffset = 1.f;	
			}

			//NOTE: Animate the door opening and closing
			if(entity->moveTimer >= 0.0f) {
				float timeScale = 2.1f;
				entity->moveTimer += timeScale*dt;

				zOffset = entity->moveTimer;

				if(!entity->chestIsOpen) {
					zOffset = 1.0f - entity->moveTimer;
				}  

				if(entity->moveTimer >= 1.0f) {
					entity->moveTimer = -1.0f;
				}
			}

			entity->T.pos.z = zOffset;

			//NOTE: See if we need to close the door on exit
			if(entity->triggerType == ENTITY_TRIGGER_OPEN_DOOR_WITH_BUTTON_WITH_TRIGGER_CLOSE) {
				if(entity->collider1->collisions.count > 0 && !entity->isActivated) {
					MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_EXIT);
					
					V3 entityP = easyTransform_getWorldPos(&entity->T);
					Entity *player = (Entity *)manager->player;
					V3 diff = v3_minus(easyTransform_getWorldPos(&player->T), entityP);

					if(info.found && diff.x < 0) { //TODO: This check has to be specific to the door
						//NOTE: Won't come back in this code path

						
 						activateDoor(entity);

 						assert(!entity->chestIsOpen);

						playGameSound(&globalLongTermArena, gameState->metalDoorSlamSound, 0, AUDIO_FOREGROUND);

						//NOTE: Update the chest state in the save file so the chest is open when the player comes back here on new scene load
						saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);

						
					}
				}
			}
		} else if(entity->triggerType == ENTITY_TRIGGER_OPEN_GATE_WITH_KEY) {
			if(entity->chestIsOpen) {
				//do nothing, already open				
			} else {
				if(entity->collider1->collisions.count > 0) {
					MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);
					if(info.found && playerUsedKey(gameState, keyStates)) {
						// Animation *openAnimation = gameState_findSplatAnimation(gameState, "woodFire5.png");
						
						entity->sprite = gameState_findSplatTexture(gameState, "ironGate_open");
						// easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, openAnimation, EASY_ANIMATION_PERIOD);	

						playGameSound(&globalLongTermArena, gameState->chestOpenSound, 0, AUDIO_FOREGROUND);

						entity->chestIsOpen = true;
						//turn collider off. 
						entity->collider->isActive = false;

						//NOTE: Update the chest state in the save file so the chest is open when the player comes back here on new scene load
						saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);
 
					}
				} else {
					entity->sprite = gameState_findSplatTexture(gameState, "ironGate");
				}
			}

		} else if(entity->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE || entity->triggerType == ENTITY_TRIGGER_FIRE_POST) {

					
			{

				if(!entity->chestIsOpen) {
					if(entity->triggerType == ENTITY_TRIGGER_FIRE_POST) {
						entity->sprite = gameState_findSplatTexture(gameState, "firePost");
					} else {
						entity->sprite = findTextureAsset("woodFire.png");	
					}
				}
				

				if(entity->collider1->collisions.count > 0) {
					MyEntity_CollisionInfo info_enter = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_ENTER);

					MyEntity_CollisionInfo info_stay = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);


					if((info_enter.found && !entity->chestIsOpen) || (info_stay.found && entity->chestIsOpen)) {
						// if(gameState->playerSaveProgress.playerInfo.lastCheckPointId != entity->T.id) 
						{
							bool shouldSave = false;
							//extra stuff is a save fire
							if(entity->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE) {

								if(entity->chestIsOpen) {
									//NOTE: Show hover for prompt to save
									renderKeyPromptHover(gameState, gameState->spacePrompt, entity, dt, true, entitiesRenderGroup);
								}

								if(!entity->chestIsOpen || (wasPressed(keyStates->gameButtons, BUTTON_SPACE) && !gameState->gameIsPaused)) {
									//ask user if they want to save their progress		

									if(!entity->chestIsOpen) {
										//play sound to say saved checkpoint only if a new fire
										playGameSound(&globalLongTermArena, gameState->gongSound, 0, AUDIO_FOREGROUND);
									}
									
									//save your checkpoint state by adding it as the lastest checkpoint
									gameState->playerSaveProgress.playerInfo.lastCheckPointId = entity->T.id;
									nullTerminateBuffer(gameState->playerSaveProgress.playerInfo.lastCheckPointSceneName, gameState->currentSceneName, easyString_getSizeInBytes_utf8(gameState->currentSceneName));
									

									///// Show help message
			    					//The dialog options 
			    					// Make the temporary talk node
			    					if(!entity->chestIsOpen) {
				    					if(!gameState->playerSaveProgress.playerInfo.hasShownFireplaceTutorial){
				        					gameState->gameDialogs.perDialogArenaMark = takeMemoryMark(&gameState->gameDialogs.perDialogArena);

				        					gameState->currentTalkText = constructDialogNode(&gameState->gameDialogs);
				        					pushTextToNode(gameState->currentTalkText, "{s: 3}Sleeping at campfires will save your progress. Once lit, you can navigate back to them.");
				    						
				    						gameState->playerSaveProgress.playerInfo.hasShownFireplaceTutorial = true;

				    					} else {
				    						gameState->gameDialogs.perDialogArenaMark = takeMemoryMark(&gameState->gameDialogs.perDialogArena);

				    						gameState->currentTalkText = constructDialogNode(&gameState->gameDialogs);
				    						pushTextToNode(gameState->currentTalkText, "{s: 3}You Rest. Progress Saved.");
				    					}

				    					shouldSave = true;

			    					} else {
			    						gameState->currentTalkText = findDialogInfo(ENTITY_DIALOG_ASK_USER_TO_SAVE, &gameState->gameDialogs);
			    					}

			    					

			    					gameState->gameModeType = GAME_MODE_READING_TEXT;
			    					gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING;
			    					gameState->enteredTalkModeThisFrame = true;
			    					
			    					gameState->messageIndex = 0; //start at the begining
			    					easyFontWriter_resetFontWriter(&gameState->fontWriter);
									///////////////////////////////////////////////////////

		    					}
		    				}

		    				if(!entity->chestIsOpen) {							
		    					//turn fire on
		    					entity_turnFireSavePlaceOn(gameState, manager, entity, false);
		    					//NOTE: Make fire stay on when you come back here
		    					saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);
		    				}

		    				if(shouldSave) {
		    					//NOTE: Save after we set the shownFirePlace boolean tutorial
		    					playerGameState_saveProgress(gameState, manager, engine_saveFilePath);
		    				}

	    				}

					} 
				}
			}
		} 
			// else if(entity->triggerType == ENTITY_TRIGGER_ENTER_SHOP) {
		// 	if(wasPressed(keyStates->gameButtons, BUTTON_SPACE) && !gameState->gameIsPaused) {
		// 		//Enter the shop
		// 		if(entity->chestType == CHEST_TYPE_SHOP_1) {
		// 			enterGameShop(gameState->townShop, gameState);
		// 		}
		// 	}
		// }
	}

	if(entity->type == ENTITY_TRIGGER) {


		if(entity->triggerType == ENTITY_TRIGGER_LOAD_SCENE && gameState->preventSceneLoadTimer < 0) { //NOTE: scene loader 
			MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);	
			if(info.found) {
				playGameSound(&globalLongTermArena, gameState->doorSound, 0, AUDIO_FOREGROUND);

				EntityLoadSceneData *loadSceneData = (EntityLoadSceneData *)easyPlatform_allocateMemory(sizeof(EntityLoadSceneData), EASY_PLATFORM_MEMORY_ZERO);
				loadSceneData->gameState = gameState;
				loadSceneData->manager = manager;
				loadSceneData->sceneToLoad = entity->levelToLoad;
				loadSceneData->editorState = editorState;
				loadSceneData->weatherState = weatherState;
				loadSceneData->partnerId = entity->partnerId;
				loadSceneData->moveDirection = entity->moveDirection;
				loadSceneData->camera = cam;

				easyConsole_addToStream(DEBUG_globalEasyConsole, "Push Trasition 1");

				EasySceneTransition *transition = EasyTransition_PushTransition(transitionState, loadScene, loadSceneData, EASY_TRANSITION_CIRCLE_N64);
				gameState->gameIsPaused = true;	
			}
		}	

		//TODO: Change to box aswell

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

		if(entity->triggerType == ENTITY_TRIGGER_BUTTON_FOR_DOOR) {

			//NOTE: The trigger button can have wizard and push blocks on it

			info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);
			MyEntity_CollisionInfo info1 = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_BLOCK_TO_PUSH, EASY_COLLISION_ENTER);	

			//Open the door partner
			Entity *partnerDoor = findEntityById(manager, entity->partnerId);

			if((info.found || info1.found) && partnerDoor) {

				if(info.found) {
					entity->refCount++;	
				}

				if(info1.found) {
					entity->refCount++;	
				}
				

				if(!partnerDoor->chestIsOpen) {

					playGameSound(&globalLongTermArena, gameState->metalDoorSlamSound, 0, AUDIO_FOREGROUND);

					entity->sprite = gameState_findSplatTexture(gameState, "button_down");
					
					partnerDoor->chestIsOpen = true;
					partnerDoor->collider->isActive = false;
					//open the door
					if(partnerDoor->moveTimer >= 0) {
						entity->moveTimer = 1.0f - partnerDoor->moveTimer;
					} else {
						partnerDoor->moveTimer = 0;	
					}

					//NOTE: Update the button state
					saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);
					//NOTE: Update the door state
					saveStateEntity_addOrEditEntity(gameState, entity->partnerId, gameState->currentSceneName, partnerDoor);

				}
			}

			info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_EXIT);
			info1 = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_BLOCK_TO_PUSH, EASY_COLLISION_EXIT);	

			//Close the door partner
			partnerDoor = findEntityById(manager, entity->partnerId);

			if((info.found || info1.found) && partnerDoor) {

				entity->refCount--;
				
				if(entity->refCount <= 0) {
					assert(entity->refCount == 0);
					assert(partnerDoor->chestIsOpen);
					//NOTE: CLOSE THE DOOR

					entity->sprite = gameState_findSplatTexture(gameState, "button_up");

					playGameSound(&globalLongTermArena, gameState->metalDoorSlamSound, 0, AUDIO_FOREGROUND);


					//NOTE: Update the chest state in the save file so the chest is open when the player comes back here on new scene load
					

					partnerDoor->collider->isActive = true;
					//close the door
					partnerDoor->chestIsOpen = false;
					//open the door
					if(partnerDoor->moveTimer >= 0) {
						partnerDoor->moveTimer = 1.0f - partnerDoor->moveTimer;
					} else {
						partnerDoor->moveTimer = 0;	
					}

					//NOTE: Update the button state
					saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);
					//NOTE: Update the door state
					saveStateEntity_addOrEditEntity(gameState, entity->partnerId, gameState->currentSceneName, partnerDoor);

				}
			}
		} else if(entity->triggerType == ENTITY_TRIGGER_FALL_THROUGH_FALL_ON_TIMER) {

			if(entity->animationController.finishedAnimationLastUpdate) {
				entity->flags |= ENTITY_SHOULD_NOT_RENDER;
			}

			if(entity->chestIsOpen) {


				//NOTE: Check if player has fallen through
				info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_STAY);

				if(info.found && !info.e->isFalling) {
					//FALL THROUGH

					//NOTE: Player now falling
					info.e->isFalling = true;
					//NOTE: Two second till game over after falling
					info.e->lifeSpanLeft = 2.f;

				}
			} else {

				//NOTE: Has been activated
				if(entity->lifeSpanLeft >= 0.f) {
					entity->lifeSpanLeft += dt;

					if(entity->lifeSpanLeft >= 2.f) {
						entity->lifeSpanLeft = -1;
						entity->chestIsOpen = true;
						saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);
					}

				} else {
					//NOTE: Check if player has entered
					info = MyEntity_hadCollisionWithType(manager, entity->collider, ENTITY_WIZARD, EASY_COLLISION_ENTER);

					if(info.found) {
						//NOTE: Start timer
						entity->lifeSpanLeft = 0;

						playGameSound(&globalLongTermArena, easyAudio_findSound("rocks_falling.wav"), 0, AUDIO_FOREGROUND);

						easyCamera_startShake(cam, EASY_CAMERA_SHAKE_DEFAULT, 0.5f);

						//NOTE: Add the new animation of it crubling
						easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "cobble_falling_11.png"), 0.18f);    
						entity->sprite = 0;

						ParticleSystemListItem *ps = getParticleSystem(manager, entity);

						// ps->ps.Set.VelBias = rect3f(-0.4f, -0.1f, -1, 0.4f, 0.1f, -0.4f);
						ps->ps.Set.VelBias = rect3f(0, 0, 0.3f, 0, 0, 0.7f);
						ps->ps.Set.posBias = rect3f(-0.4f, -0.4f, 0, 0.4f, 0.1f, 0);
						ps->ps.Set.bitmapScale = 0.6f;

						easyParticles_setSystemLifeSpan(&ps->ps, 1.4f);
						setParticleLifeSpan(&ps->ps, 1.6f);

						ps->color = v4(0.3f, 0.3f, 0.3f, 1);
						ps->ps.Set.Loop = false;

						Reactivate(&ps->ps);

						ps->position = &entity->T.pos;
						ps->offset = v3(0, 0, 0);


					}
				}
			}
		}
	} else if(entity->type == ENTITY_BLOCK_TO_PUSH) {

		float margin_dP = 0.0f;

		float speedFactor = 1.1f;

		//////////////////////////////////////////

		V3 playerInWorldP = roundToGridBoard(easyTransform_getWorldPos(&player->T), 1);
		V3 entP_inWorld = roundToGridBoard(easyTransform_getWorldPos(&entity->T), 1);

		entP_inWorld.z = 0;


		if(!entity->aiController) {
			entity->aiController = easyAi_initController(&globalPerSceneArena);
		}	
		
		bool saveEntity = false;

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

				ParticleSystemListItem *ps = getParticleSystem(manager, entity);
				ps->ps.Set.VelBias = rect3f(-1, 0, -1, -0.3f, 0, -1.3f);
				ps->ps.Set.posBias = rect3f(0, -0.3f, 0, 0, 0.3f, 0);

				ps->position = &entity->T.pos;
				ps->offset = v3(-0.2f, 0, 0.5f);
				saveEntity = true;
				
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

				ParticleSystemListItem *ps = getParticleSystem(manager, entity);
				ps->ps.Set.VelBias = rect3f(0.3f, 0, -1, 1.f, 0, -1.3f);
				ps->ps.Set.posBias = rect3f(0, -0.3f, 0, 0, 0.3f, 0);
				ps->position = &entity->T.pos;
				ps->offset = v3(0.2f, 0, 0.5f);
				saveEntity = true;
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

				ParticleSystemListItem *ps = getParticleSystem(manager, entity);
				ps->ps.Set.VelBias = rect3f(0, 0.3f, -1, 0, 1, -1.3f);
				ps->ps.Set.posBias = rect3f(-0.3f, 0, 0, 0.3f, 0, 0);
				ps->position = &entity->T.pos;
				ps->offset = v3(0, 0.5f, 0.5f);
				saveEntity = true;
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

				ParticleSystemListItem *ps = getParticleSystem(manager, entity);
				ps->ps.Set.VelBias = rect3f(0, -1.f, -1, 0, -0.3f, -1.3f);
				ps->ps.Set.posBias = rect3f(-0.3f, 0, 0, 0.3f, 0, 0);
				ps->position = &entity->T.pos;
				ps->offset = v3(0, -0.5f, 0.5f);

				saveEntity = true;

			}
		}

		if(saveEntity) {
			V3 lastP = entity->T.pos;
			entity->T.pos.xy = entity->endMovePos.xy;

			saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);
			entity->T.pos = lastP;
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

	if(entity->type == ENTITY_BOMB) {
		entity->lifeSpanLeft -= dt;

		if(entity->lifeSpanLeft <= 0) {
			if(entity->sprite) {
				entity->sprite = 0;
				entity->isDying = true;
				easyAnimation_addAnimationToController(&entity->animationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "bomb_explosion_9.png"), 0.02f);		
				
				playGameSound(&globalLongTermArena, easyAudio_findSound("bomb_explosion.wav"), 0, AUDIO_FOREGROUND);

				entity->colorTint = COLOR_WHITE;
			}
		} else {
			float timerValue = (1 / (entity->lifeSpanLeft / 3.f));
			entity->colorTint = smoothStep01010V4(COLOR_WHITE, timerValue, COLOR_RED);
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



	if(entity->type == ENTITY_ENEMY_PROJECTILE) {
		//NOTE: See if it hurt any entities

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
		entity->rb->dragFactor = 0.001f;
		entity->rb->accumForce.z = 20; //gravity for the projectile

		if(entity->isDying) {
			entity->rb->dragFactor = 0;
			entity->rb->dP = v3(0, 0, 0);
			entity->rb->accumForce = v3(0, 0, 0); //gravity for the projectile
		}

		V3 entityP = easyTransform_getWorldPos(&entity->T);

		if(entityP.z > 0 && !entity->isDying) {
			//NOTE: Add floor friction now
			entity->rb->dragFactor = 0.08;
			entity->T.pos.z = 0; //make sure it doesn't go through the floor

			//NOTE: Can pick up the arrow now that it's stopped

	 		if(entity->collider1->collisions.count > 0 && !entity->isDying) {
	 			MyEntity_CollisionInfo info = MyEntity_hadCollisionWithType(manager, entity->collider1, ENTITY_WIZARD, EASY_COLLISION_STAY);

	 			if(info.found) {
	 				renderKeyPromptHover(gameState, gameState->spacePrompt, entity, dt, true, entitiesRenderGroup);

	 				if(wasPressed(keyStates->gameButtons, BUTTON_SPACE) && !gameState->gameIsPaused) {
	 					//NOTE: Pickup the arrow
	 					//TODO: Should hover above the person's head briefly

	 					playGameSound(&globalLongTermArena, gameState->saveSuccessSound, 0, AUDIO_FOREGROUND);

	 					addItemToPlayer(gameState, ENTITY_PLAYER_PROJECTILE, 1, true);
	 					
	 					//Remove from the game
	 					entity->isDead = true;
	 				}

	 			}
			}

		}

 		if(entity->collider->collisions.count > 0 && !entity->isDying) {
 			MyEntity_CollisionInfo_Multi infos = MyEntity_hadCollisionWithType_multi(manager, entity->collider, ENTITY_ENEMY, EASY_COLLISION_ENTER);
            for(int i = 0; i < infos.count; ++i) {
            	// easyConsole_addToStream(console, "HIT Enemy");
            	Entity *enemy = infos.e[i];
            	hurtEnemy(gameState, manager, findDamage(entity->type), enemy, easyTransform_getWorldPos(&entity->T));

            	//NOTE: Make the arrow part of the entity so it walks around with it in
            	easyTransform_assignAsParent(&entity->T, &enemy->T);
            	
            	//Should walk around with the enemy now, so deavtivate the arrow. Piggy backing on the isDying bool
            	entity->isDying = true;
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

		if(entity->type == ENTITY_BOMB && entity->animationController.finishedAnimationLastUpdate) {
			assert(entity->isDying);
			entity->isDead = true;
		}

		if(entity->type == ENTITY_ENEMY) {
			Animation *deathAnimation = getAnimationForEnemy(gameState, ENTITY_ANIMATION_DIE, entity->enemyType);
			if(entity->animationController.finishedAnimationLastUpdate && entity->animationController.lastAnimationOn == deathAnimation && easyAnimation_getCurrentAnimation(&entity->animationController, deathAnimation)) {
				entity->isDead = true;

				//NOTE: Add it to the save file
				saveStateEntity_addOrEditEntity(gameState, entity->T.id, gameState->currentSceneName, entity);

				easyConsole_addToStream(DEBUG_globalEasyConsole, "enemy died");

				if(entity == editorState->entitySelected) {
					letGoOfSelectedEntity(editorState);
				}

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
	
	bool shouldDrawForShadows = sprite->shouldDrawShadows;

	// if(sprite && easyString_stringsMatch_nullTerminated(sprite->name, "grass_splat")) {
	// 	shouldDrawForShadows = false;
	// }


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
		// if(sprite->isAlpha) {
		// 	EntityRender_Alpha alphaStruct = {};
		// 	alphaStruct.sprite = sprite;
		// 	alphaStruct.T = T;
		// 	alphaStruct.shader = shaderProgram;
		// 	alphaStruct.colorTint = entity->colorTint; 

		// 	addElementInfinteAlloc_notPointer(&gameState->alphaSpritesToRender, alphaStruct);
		// } else 
		{
			renderSetShader(entitiesRenderGroup, shaderProgram); renderDrawSprite(entitiesRenderGroup, sprite, entity->colorTint);

			if(shouldDrawForShadows) {
				renderSetShader(shadowMapGroup, &shadowMapProgram); renderDrawSprite(shadowMapGroup, sprite, entity->colorTint);
			}
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

static Entity *initEmptyTrigger(GameState *gameState, EntityManager *manager, V3 worldP, EntityTriggerType triggerType, Texture *t) {
	if(!t) { t = &globalWhiteTexture; }
    Entity *e =  initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_TRIGGER, 0, t, COLOR_WHITE, -1, true);
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

static Entity *initEnemy(GameState *gameState, EntityManager *manager, V3 worldP) {
	return initEntity(manager, &gameState->werewolfIdle, worldP, v2(2.5f, 2.5f), v2(0.5f, 0.25f), gameState, ENTITY_ENEMY, gameState->inverse_weight, 0, COLOR_WHITE, 1, true);
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

static Entity *initLampPost(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture, Animation *animation) {
	//this is a bit of a hack, I just wanted the torches not to have a collision box
	bool shouldCollide = true;
	if(animation) {
		shouldCollide = false;
	}

	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_LAMP_POST, 0, splatTexture, COLOR_WHITE, -1, shouldCollide);
	
	float w = 3.0f;
	float h = 2.0f;
	if(e->sprite) {
		h = e->sprite->aspectRatio_h_over_w*w;	
	}
	
	e->T.scale = v3(w, h, 1);
	e->T.pos.z = -0.5f*h;

	e->innerRadius = 0.2f;
	e->outerRadius = 9.0f;

	if(animation) {
		e->sprite = 0;
		easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "castle_torch_fire4.png"), EASY_ANIMATION_PERIOD);
		
		e->innerRadius = 0.2f;
		e->outerRadius = 3.0f;
		
		e->flags |= ENTITY_SHOULD_SAVE_ANIMATION;
	}

	if(e->collider) {

		e->collider->offset.x = -1.0f;
		e->collider->offset.y = -1.5f;
		
		e->collider->dim2f.x = 0.5f;
		e->collider->dim2f.y = 0.3f;

	}	

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


static void assignAnimalAttribs(Entity *e, GameState *gameState, EntityAnimalType animalType) {
	if(animalType == ENTITY_ANIMAL_CHICKEN) {
		easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationFreeList);
		e->sprite = 0;
		Animation *animation = getAnimationForAnimal(gameState, ENTITY_ANIMATION_IDLE, animalType);
		easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, animation, EASY_ANIMATION_PERIOD);
		e->collider->dim2f = v2(0.5f, 0.5f);
		e->T.scale.xy = v2(0.5f, 0.5f);
		e->T.pos.z = -0.15f;
	}
}


static Entity *initShootTrigger(GameState *gameState, EntityManager *manager, V3 worldP, Texture *splatTexture) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SHOOT_TRIGGER, 0, splatTexture, COLOR_WHITE, -1, true);
	return e;
}

static Entity *initAnimal(GameState *gameState, EntityManager *manager, V3 worldP, EntityAnimalType animalType) {
	Entity *e = initEntity(manager, 0, worldP, v2(1, 1), v2(1, 1), gameState, ENTITY_ANIMAL, 0, &globalPinkTexture, COLOR_WHITE, -1, true);

	assignAnimalAttribs(e, gameState, animalType);

	return e;
}


static Entity *initBomb(GameState *gameState, EntityManager *manager, V3 worldP) {
	Texture *bombTexture = findTextureAsset("bomb_world.png");

    Entity *e =  initEntity(manager, 0, worldP, v2(2.5f, 2.5f*bombTexture->aspectRatio_h_over_w), v2(0.3f, 0.3f), gameState, ENTITY_BOMB, 0, bombTexture, COLOR_WHITE, -1, true);
    e->triggerType = ENTITY_TRIGGER_NULL;
    e->collider1->dim2f = v2(4, 4);
    e->T.pos.z = -0.4f;

    e->currentSound = playLocationSound(&globalLongTermArena, easyAudio_findSound("bomb_fuse.wav"), 0, AUDIO_FOREGROUND, easyTransform_getWorldPos(&e->T));
    e->currentSound->volume = 3.f;
	e->currentSound->outerRadius = 5.0f;;

    e->lifeSpanLeft = 3.0f;

    ParticleSystemListItem *ps = getParticleSystem(manager, e);

    // ps->ps.Set.VelBias = rect3f(-0.4f, -0.1f, -1, 0.4f, 0.1f, -0.4f);
    ps->ps.Set.VelBias = rect3f(0, 0, -0.3f, 0, 0, -0.7f);
    ps->ps.Set.posBias = rect3f(0, -0.1f, 0, 0, 0.1f, 0);
    ps->ps.Set.bitmapScale = 0.1f;

    easyParticles_setSystemLifeSpan(&ps->ps, 30.0f);
    setParticleLifeSpan(&ps->ps, 1.6f);

    ps->color = v4(0.3f, 0.3f, 0.3f, 1);
    ps->ps.Set.Loop = true;

    Reactivate(&ps->ps);

    ps->position = &e->T.pos;
    ps->offset = v3(0, 0, -0.1f);



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
		
	e->T.pos.z = -0.26f;
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



static Entity *initEntityOfType(GameState *gameState, EntityManager *manager, V3 position, Texture *splatTexture, EntityType entType, SubEntityType subtype, bool colliderSet, EntityTriggerType triggerType, char *audioFile, Animation *animation, EntityAnimalType animalType) {
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
			Texture *dartTexture = findTextureAsset("dart.png");
		    newEntity =  initEntity(manager, 0, position, v2(1, 1), v2(1, 1), gameState, ENTITY_PLAYER_PROJECTILE, 1.f / 20.f, dartTexture, COLOR_WHITE, -1, true);
		    newEntity->T.pos.z = -0.5f;
		    newEntity->lifeSpanLeft = 60.f*3; //3minutes: they have a really long life span so you can pick them back up
		    float spellScale = 0.15f;
		    newEntity->T.scale = v3(spellScale, dartTexture->aspectRatio_h_over_w*spellScale, spellScale);

		    assert(newEntity->collider1);

		    //NOTE: Make the pickup collider bigger
		    newEntity->collider1->dim2f = v2(6, 6);
		} break;
		case ENTITY_TRIGGER_WITH_RIGID_BODY: {
			newEntity = initEmptyTriggerWithRigidBody(gameState, manager, position, splatTexture);
			newEntity->triggerType = triggerType;
		} break;
		case ENTITY_SHOOT_TRIGGER: {
			newEntity = initShootTrigger(gameState, manager, position, splatTexture);
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
            newEntity = initEmptyTrigger(gameState, manager, position, triggerType, splatTexture);
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
            newEntity = initLampPost(gameState, manager, position, splatTexture, animation);
        } break;
        case ENTITY_ENEMY: {
            newEntity = initEnemy(gameState, manager, position);
        } break;
        case ENTITY_ANIMAL: {
        	newEntity = initAnimal(gameState, manager, position, animalType);
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
        case ENTITY_BOMB: {
        	newEntity = initBomb(gameState, manager, position);
        } break;
		default: {
			//do nothing
		}
	}

	return newEntity;
}