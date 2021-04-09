#define MY_ENTITY_TYPE(FUNC) \
FUNC(ENTITY_NULL)\
FUNC(ENTITY_SCENERY)\
FUNC(ENTITY_WIZARD)\
FUNC(ENTITY_PLAYER_PROJECTILE)\
FUNC(ENTITY_SKELETON)\
FUNC(ENTITY_HEALTH_POTION_1)\
FUNC(ENITY_AUDIO_CHECKPOINT)\
FUNC(ENITY_CHECKPOINT)\
FUNC(ENTITY_TERRAIN)\
FUNC(ENTITY_WEREWOLF)\
FUNC(ENTITY_SWORD)\
FUNC(ENTITY_SIGN)\
FUNC(ENTITY_SHEILD)\
FUNC(ENTITY_PLAYER_HIT_BOX)\
FUNC(ENTITY_STAMINA_POTION_1)\
FUNC(ENTITY_BLOCK_TO_PUSH)\
FUNC(ENTITY_HORSE)\
FUNC(ENTITY_CHEST)\
FUNC(ENTITY_HOUSE)\
FUNC(ENTITY_LAMP_POST)\
FUNC(ENTITY_TILE_MAP)\
FUNC(ENTITY_TRIGGER)\
FUNC(ENTITY_SEAGULL)\
FUNC(ENTITY_ENTITY_CREATOR)\
FUNC(ENTITY_AI_ANIMATION)\



typedef enum {
	GAME_MODE_PLAY,
	GAME_MODE_GAME_OVER,
	GAME_MODE_READING_TEXT,
	GAME_MODE_MAIN_MENU,
	GAME_MODE_PAUSE_MENU,
	GAME_MODE_ITEM_COLLECT,
} GameModeType;

typedef enum {
	GAME_PAUSE_MENU_MAIN,
	GAME_PAUSE_MENU_SETTINGS
} PauseMenuSubType;


//// Entity Trigger Types ////////////
#define MY_TRIGGER_TYPE(FUNC) \
FUNC(ENTITY_TRIGGER_NULL)\
FUNC(ENTITY_TRIGGER_START_SWIM)\
FUNC(ENTITY_TRIGGER_LOCATION_SOUND)\
FUNC(ENTITY_TRIGGER_LOAD_SCENE)\


typedef enum {
    MY_TRIGGER_TYPE(ENUM)
} EntityTriggerType;

static char *MyEntity_TriggerTypeStrings[] = { MY_TRIGGER_TYPE(STRING) };

/////////////////////////////////////////////////////////////////

//// Entity Sound Types ////////////
#define MY_LOCATION_SOUND_TYPE(FUNC) \
FUNC(ENTITY_SOUND_NULL)\
FUNC(ENTITY_SOUND_SEASIDE)\


typedef enum {
    MY_LOCATION_SOUND_TYPE(ENUM)
} EntityLocationSoundType;

static char *MyEntity_LocationSoundTypeStrings[] = { MY_LOCATION_SOUND_TYPE(STRING) };

/////////////////////////////////////////////////////////////////



typedef enum {
    MY_ENTITY_TYPE(ENUM)
} EntityType;

static char *MyEntity_EntityTypeStrings[] = { MY_ENTITY_TYPE(STRING) };

#define MAX_PLAYER_ITEM_COUNT 12
#define UI_ITEM_PICKER_MIN_SIZE 1.0f
#define UI_ITEM_PICKER_MAX_SIZE 1.5f

#define UI_ITEM_RADIUS_MIN 0.6f
#define UI_ITEM_RADIUS_MAX 1.0f

typedef struct {
	EntityType type;

	V2 startP;
	V2 endP;
	float tAt;
} ItemAnimationInfo;

typedef struct {
	EntityType type;
	int count; //number of items you have
	bool isDisposable; //decrements each time you use it. 
} ItemInfo;

typedef struct {
	float current;
	float target;
} ItemGrowTimerUI;


static ItemAnimationInfo items_initItemAnimation(V2 startP, V2 endP, EntityType type) {
	ItemAnimationInfo result = {};
	result.tAt = 0;
	result.startP = startP;
	result.endP = endP;
	result.type = type;
	return result;
}


#define MY_TILE_TYPE(FUNC) \
FUNC(WORLD_TILE_GRASS)\
FUNC(WORLD_TILE_LAVA)\
FUNC(WORLD_TILE_ROCK)\
FUNC(WORLD_TILE_COBBLE)\
FUNC(WORLD_TILE_PATH)\
FUNC(WORLD_TILE_PATH1)\
FUNC(WORLD_TILE_PATH2)\
FUNC(WORLD_TILE_PATH3)\
FUNC(WORLD_TILE_PATH4)\
FUNC(WORLD_TILE_PATH5)\
FUNC(WORLD_TILE_BEACH)\
FUNC(WORLD_TILE_SEA)\
FUNC(WORLD_TILE_SEA1)\
FUNC(WORLD_TILE_BEACH_GRASS)\
FUNC(WORLD_TILE_SAND_WATER)\
FUNC(WORLD_TILE_SAND_WATER1)\
FUNC(WORLD_TILE_PIER_MIDDLE)\
FUNC(WORLD_TILE_PIER_SAND)\
FUNC(WORLD_TILE_PIER_SIDE_LEFT)\
FUNC(WORLD_TILE_PIER_SIDE_RIGHT)\
FUNC(WORLD_TILE_PIER_SAND_CORNER_LEFT)\
FUNC(WORLD_TILE_PIER_SAND_CORNER_RIGHT)\
FUNC(WORLD_TILE_PIER_SEA_CORNER_RIGHT)\
FUNC(WORLD_TILE_PIER_SEA_CORNER_LEFT)\


typedef enum {
    MY_TILE_TYPE(ENUM)
} WorldTileType;

static char *MyTiles_TileTypeStrings[] = { MY_TILE_TYPE(STRING) };

typedef struct {
	WorldTileType type;
	int x;
	int y;

	EasyAnimation_Controller animationController;
} WorldTile;

typedef struct {
	int tileCount;
	WorldTile tiles[10000];
} TileSheet;

typedef struct {
	Matrix4 orthoFuaxMatrix;
	V2 fuaxResolution;

	EasyAnimation_ListItem *animationFreeList;

	//ANIMATIONS
	Animation wizardForward;
	Animation wizardAttack;
	Animation firePitAnimation;
	Animation torchAnimation;

	Animation wizardAttack2;
	Animation wizardDeath;
	Animation wizardHit;
	Animation wizardIdle;
	Animation wizardJump;
	Animation wizardFall;

	Animation wizardSwordAttackLeft;
	Animation wizardSwordAttackRight;
	Animation wizardSwordAttackFront;
	Animation wizardSwordAttackBack;

	Animation wizardBottom;
	Animation wizardLeft;
	Animation wizardRight;
	Animation wizardGetItem;

	Animation wizardSwimLeft;
	Animation wizardSwimRight;
	Animation wizardSwimUp;
	Animation wizardSwimDown;

	Animation wizardIdleForward;
	Animation wizardIdleBottom;
	Animation wizardIdleLeft;
	Animation wizardIdleRight;

	Animation skeletonAttack;
	Animation skeletonDeath;
	Animation skeltonIdle;
	Animation skeltonShield;
	Animation skeltonHit;
	Animation skeltonWalk;


	Animation barrelWater;
	Animation seagullAnimation;

	Animation walkAnimation;

	Animation werewolfIdle;

	///// TIle Animations ///
	Animation seaTileAnimation;
	Animation sandWaterTileAnimation;

	/////

	Texture *playerTexture;

	GameModeType gameModeType;

	//Cached Models
	EasyModel *potionModel;

	///////////

	char *emptyString;

	

	///// INVENTORY MENU attributes ///////

	//
	bool isLookingAtItems;
	//This is for the menu to grow as you open it
	// ItemGrowTimerUI lookingAt_animTimer;
	int indexInItems;
	float inventoryBreathSelector;

	//What items the player has equiped to the x, y keys
	ItemInfo *playerHolding[2]; //pointing to the itemSpots array

	//The animation timer to make them grow when you equip items
	float animationItemTimersHUD[2];


	//NOTE: The items the player has in their inventory
	int itemCount;
	ItemInfo itemSpots[MAX_PLAYER_ITEM_COUNT]; //player items


	//	
	ItemGrowTimerUI animationItemTimers[MAX_PLAYER_ITEM_COUNT];
	

	int itemAnimationCount;
	ItemAnimationInfo item_animations[32];


	//////

	//PAUSE MENU ////////
	int currentMenuIndex;
	PauseMenuSubType pauseMenu_subType;

	//SOUNDS

	WavFile *clickSound;
	WavFile *equipItemSound;
	WavFile *openMenuSound;
	WavFile *chestOpenSound;
	WavFile *bubbleSound;
	WavFile *doorSound;
	WavFile *waterInSound;
	WavFile *seagullsSound;
	////

	float werewolf_attackSpeed;
	float werewolf_restSpeed;
	float werewolf_knockback_distance;
	float player_knockback_distance;


	//EDITOR STATE
	bool isEditorOpen;

	///////////////////

	EasyPhysics_World physicsWorld;

	char *currentSceneName;
	char *sceneFileNameTryingToSave;

	float inverse_weight;


	////For when collecting item in the Gamestate = GAME_MODE_ITEM_COLLECT
	EntityType itemCollectType;
	void *entityChestToDisplay;
	int itemCollectCount;

	particle_system collectParticleSystem;
	/////////////////////////

	//
	particle_system playerUseItemParticleSystem;
	///////

	//Message for the message box
	int messageIndex;
	EntityDialogInfo currentTalkText;
	PlayingSound *talkingNPC;


	InfiniteAlloc splatListAnimations;
	InfiniteAlloc splatAnimations;

	InfiniteAlloc splatList;
	InfiniteAlloc splatTextures;

	InfiniteAlloc splatList_tiles;
	InfiniteAlloc splatTextures_tiles;

	//Player variables loaded in tweak file
	float jumpPower;
	float walkPower;
	float gravityScale;
	float cameraSnapDistance;
	//

	//Button prompts ////
	Texture *spacePrompt;

	Quaternion angledQ;
	EasyTransform tempTransform;

	///////////////////////////

	/// Default transform for models
	EasyTransform tempTransform_model;
	float rotationUpdate;
	///////////

	WavFile *playerAttackSounds[3];

	void *currentTerrainEntity;

	float swordSwingTimer;

	EasyTerrainDataPacket terrainPacket;

	TileSheet tileSheet;

	WavFile *successSound;
	bool gameIsPaused;

} GameState; 

static GameState *initGameState(float yOverX_aspectRatio) {
	GameState *state = pushStruct(&globalLongTermArena, GameState);


	///////////////////////************ Ortho matrix to use for rendering UI *************////////////////////
	float fuaxWidth = 1920.0f;
	state->fuaxResolution = v2(fuaxWidth, fuaxWidth*yOverX_aspectRatio);
	state->orthoFuaxMatrix = OrthoMatrixToScreen_BottomLeft(state->fuaxResolution.x, state->fuaxResolution.y); 

	////////////////////////////////////////////////////////////////////

	state->animationFreeList = 0;


	state->isLookingAtItems = false;


	//NOTE: Clear out what the player is holding
	state->playerHolding[0] = 0;
	state->playerHolding[1] = 0;

	state->itemAnimationCount = 0;

	state->indexInItems = 0;
	state->gameIsPaused = false;

	state->openMenuSound = state->equipItemSound = state->clickSound = findSoundAsset("click2.wav");
	state->chestOpenSound = findSoundAsset("chest_open.wav");

	state->bubbleSound = findSoundAsset("bubble1.wav");

	state->doorSound = findSoundAsset("door_close.wav");
	state->waterInSound = findSoundAsset("waterIn.wav");
	state->seagullsSound = findSoundAsset("seaside.wav");

	//NOTE: This is used for the key prompts in a IMGUI fashion
	state->angledQ = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);

	easyTransform_initTransform(&state->tempTransform, v3(0, 0, 0), EASY_TRANSFORM_TRANSIENT_ID);
	state->tempTransform.Q = state->angledQ;
	state->spacePrompt = findTextureAsset("space_prompt.png");
	//////

	// Default Transform to draw models not assigned to entities
	easyTransform_initTransform(&state->tempTransform_model, v3(0, 0, 0), EASY_TRANSFORM_TRANSIENT_ID);
	/////////////////////////


	//NOTE: Initialize the ui item pickers to nothing
	for(int i = 0; i < arrayCount(state->animationItemTimers); ++i) {
		state->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
		state->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
	}

	state->animationItemTimersHUD[0] = -1.0f;
	state->animationItemTimersHUD[1] = -1.0f;

	state->isEditorOpen = false;

	state->inverse_weight = 1 / 10.0f;

	state->currentSceneName = 0;
	state->sceneFileNameTryingToSave = 0;

	state->gameModeType = GAME_MODE_PLAY;

	state->splatList = initInfinteAlloc(char *);
	state->splatTextures = initInfinteAlloc(Texture *);

	state->splatList_tiles = initInfinteAlloc(char *);
	state->splatTextures_tiles = initInfinteAlloc(Texture *);

	state->splatListAnimations = initInfinteAlloc(char *);
	state->splatAnimations = initInfinteAlloc(Animation *);

	state->walkPower = 400;

	state->gravityScale = 150;

	state->swordSwingTimer = -1.0f;

	EasyPhysics_beginWorld(&state->physicsWorld);


	state->playerAttackSounds[0] = easyAudio_findSound("player_attack1.wav");
	state->playerAttackSounds[1] = easyAudio_findSound("player_attack2.wav");
	state->playerAttackSounds[2] = easyAudio_findSound("player_attack3.wav");

	state->successSound = findSoundAsset("success.wav");


	state->terrainPacket.textureCount = 4;;
	state->terrainPacket.textures[0] = findTextureAsset("blend_mud.png");
	state->terrainPacket.textures[1] = findTextureAsset("mud_block.png");
	state->terrainPacket.textures[2] = findTextureAsset("path_block.png");
	state->terrainPacket.textures[3] = findTextureAsset("floor_texture.jpg");

	state->terrainPacket.blendMap = findTextureAsset("blendmap_house.png");
	state->currentTerrainEntity = 0;

	state->werewolf_attackSpeed = 5;
	state->werewolf_restSpeed = 2;
	state->werewolf_knockback_distance = 700000;
	state->player_knockback_distance = 600000;

	//////////////////////////////////////
	state->rotationUpdate = 0;

	/////////////////// INVENTORY PARTICLE SYSTEM//////////////////////////////

    particle_system_settings ps_set = InitParticlesSettings(PARTICLE_SYS_DEFAULT, 3.0f);

    ps_set.VelBias = rect3f(-1, -1, -2, 1, 1, -8);
    ps_set.posBias = rect3f(-1, -1, 0, 1, 1, 0);

    // ps_set.angleForce = v2(1, 10);

    ps_set.bitmapScale = 0.1f;

    pushParticleBitmap(&ps_set, findTextureAsset("light_03.png"), "particle1");
    pushParticleBitmap(&ps_set, findTextureAsset("light_01.png"), "particle2");

    InitParticleSystem(&state->collectParticleSystem, &ps_set, 32);

    setParticleLifeSpan(&state->collectParticleSystem, 3.0f);

    state->collectParticleSystem.Active = true;
    state->collectParticleSystem.Set.Loop = true;

    prewarmParticleSystem(&state->collectParticleSystem, v3(0, 0, -1));

    ////////////////////////////////////////////////////////////////////////////

	/////////////////// USE ITEM PARTICLE SYSTEM//////////////////////////////

    particle_system_settings ps_set1 = InitParticlesSettings(PARTICLE_SYS_DEFAULT, 0.2f);

    ps_set1.VelBias = rect3f(-1, -1, -2, 1, 1, -8);
    ps_set.posBias = rect3f(-1, -1, 0, 1, 1, 0);

    ps_set1.bitmapScale = 0.3f;

    pushParticleBitmap(&ps_set1, findTextureAsset("light_03.png"), "particle1");
    pushParticleBitmap(&ps_set1, findTextureAsset("light_01.png"), "particle2");

    InitParticleSystem(&state->playerUseItemParticleSystem, &ps_set1, 16);

    setParticleLifeSpan(&state->playerUseItemParticleSystem, 0.6f);

    state->playerUseItemParticleSystem.Active = false;
    state->playerUseItemParticleSystem.Set.Loop = false;

    // prewarmParticleSystem(&state->playerUseItemParticleSystem, v3(0, 0, -1));

    ////////////////////////////////////////////////////////////////////////////

    state->emptyString = "emptyString"; //string used for entities that the string is altered using textboxes in editor, but just need it to not be null to preload the textbox
    


	return state;
}	


static Texture *gameState_findSplatTexture(GameState *gameState, char *textureName) {
	Texture *found = 0;

	for(int i = 0; i < gameState->splatTextures.count && !found; ++i) {
		Texture *t = *getElementFromAlloc(&gameState->splatTextures, i , Texture *);

		if(easyString_stringsMatch_nullTerminated(textureName, t->name)) {
			found = t;
			break;
		}
	}	

	if(!found) {

		easyConsole_addToStream(DEBUG_globalEasyConsole, easy_createString_printf(&globalPerFrameArena, "Couldn't find texture: %s", textureName));
		found = pushStruct(&globalLongTermArena, Texture);
		*found = globalPinkTexture;

		//save the texture name so it doesn't get overwritten when we save the level 
		found->name = textureName;
	}

	return found;
}

static Animation *gameState_findSplatAnimation(GameState *gameState, char *name) {
	Animation *found = 0;

	for(int i = 0; i < gameState->splatListAnimations.count && !found; ++i) {
		char *t = *getElementFromAlloc(&gameState->splatListAnimations, i , char *);

		if(easyString_stringsMatch_nullTerminated(name, t)) {
			
			assert(i > 0);
			Animation *animation = ((Animation **)(gameState->splatAnimations.memory))[i - 1];
			found = animation;
			
			break;
		}
	}	

	if(!found) {

		easyConsole_addToStream(DEBUG_globalEasyConsole, easy_createString_printf(&globalPerFrameArena, "Couldn't find animation: %s", name));
	}

	return found;
}


static inline bool addWorldTile(GameState *gameState, int x, int y, WorldTileType type) {
	bool didAdd = false;

	x = x - (x % GLOBAL_WORLD_TILE_SIZE);
	y = y - (y % GLOBAL_WORLD_TILE_SIZE);
	WorldTile *foundTile = 0;
	//look for tile first
	for(int i = 0; i < gameState->tileSheet.tileCount && !foundTile; ++i) {
	    WorldTile *t = gameState->tileSheet.tiles + i;

	    if(t->x == x && t->y == y) {
	    	foundTile = t;
	    	didAdd = true;
	    	break;
	    }
	}


	if(!foundTile && gameState->tileSheet.tileCount < arrayCount(gameState->tileSheet.tiles)) {
		didAdd = true;
		foundTile = gameState->tileSheet.tiles + gameState->tileSheet.tileCount++;
	} 

	if(foundTile) {
		foundTile->x = x;
		foundTile->y = y;
		foundTile->type = type; 

		easyAnimation_initController(&foundTile->animationController);
	}

	return didAdd;
} 
