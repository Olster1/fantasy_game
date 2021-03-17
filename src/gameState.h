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


typedef enum {
	GAME_MODE_PLAY,
	GAME_MODE_GAME_OVER,
	GAME_MODE_READING_TEXT,
	GAME_MODE_MAIN_MENU,
	GAME_MODE_PAUSE_MENU,
} GameModeType;



typedef enum {
    MY_ENTITY_TYPE(ENUM)
} EntityType;

static char *MyEntity_EntityTypeStrings[] = { MY_ENTITY_TYPE(STRING) };

#define MAX_PLAYER_ITEM_COUNT 8
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

typedef struct {
	Matrix4 orthoFuaxMatrix;
	V2 fuaxResolution;

	EasyAnimation_ListItem *animationFreeList;

	//ANIMATIONS
	Animation wizardRun;
	Animation wizardAttack;
	Animation firePitAnimation;
	Animation torchAnimation;

	Animation wizardAttack2;
	Animation wizardDeath;
	Animation wizardHit;
	Animation wizardIdle;
	Animation wizardJump;
	Animation wizardFall;

	Animation wizardBottom;
	Animation wizardLeft;
	Animation wizardRight;

	Animation skeletonAttack;
	Animation skeletonDeath;
	Animation skeltonIdle;
	Animation skeltonShield;
	Animation skeltonHit;
	Animation skeltonWalk;

	Animation werewolfIdle;

	Texture *playerTexture;

	GameModeType gameModeType;


	///// INVENTORY MENU attributes ///////

	//
	bool isLookingAtItems;
	//This is for the menu to grow as you open it
	// ItemGrowTimerUI lookingAt_animTimer;
	int indexInItems;

	//What items the player has equiped to the x, y keys
	EntityType playerHolding[2];
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

	//SOUNDS

	WavFile *clickSound;
	WavFile *equipItemSound;
	WavFile *openMenuSound;

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

	//Message for the message box
	char *currentTalkText;


	InfiniteAlloc splatList;
	InfiniteAlloc splatTextures;

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

	WavFile *playerAttackSounds[3];

	void *currentTerrainEntity;

	float swordSwingTimer;

	EasyTerrainDataPacket terrainPacket;

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
	state->playerHolding[0] = ENTITY_SWORD;
	state->playerHolding[1] = ENTITY_SHEILD;

	state->itemAnimationCount = 0;

	state->indexInItems = 0;
	state->gameIsPaused = false;

	state->openMenuSound = state->equipItemSound = state->clickSound = findSoundAsset("click2.wav");


	//NOTE: This is used for the key prompts in a IMGUI fashion
	state->angledQ = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);

	easyTransform_initTransform(&state->tempTransform, v3(0, 0, 0), EASY_TRANSFORM_TRANSIENT_ID);
	state->tempTransform.Q = state->angledQ;
	state->spacePrompt = findTextureAsset("space_prompt.png");
	//////



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

	state->walkPower = 400;

	state->gravityScale = 150;

	state->swordSwingTimer = -1.0f;

	EasyPhysics_beginWorld(&state->physicsWorld);


	state->playerAttackSounds[0] = easyAudio_findSound("player_attack1.wav");
	state->playerAttackSounds[1] = easyAudio_findSound("player_attack2.wav");
	state->playerAttackSounds[2] = easyAudio_findSound("player_attack3.wav");

	state->successSound = findSoundAsset("success.wav");


	state->terrainPacket.textureCount = 4;;
	state->terrainPacket.textures[0] = findTextureAsset("grass_block.png");
	state->terrainPacket.textures[1] = findTextureAsset("mud_block.png");
	state->terrainPacket.textures[2] = findTextureAsset("path_block.png");
	state->terrainPacket.textures[3] = findTextureAsset("floor_texture.jpg");

	state->terrainPacket.blendMap = findTextureAsset("black_blend.png");
	state->currentTerrainEntity = 0;

	state->werewolf_attackSpeed = 5;
	state->werewolf_restSpeed = 2;
	state->werewolf_knockback_distance = 700000;
	state->player_knockback_distance = 600000;


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
		found = &globalPinkTexture;
	}

	return found;
}