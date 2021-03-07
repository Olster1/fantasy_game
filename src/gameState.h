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

	Animation skeletonAttack;
	Animation skeletonDeath;
	Animation skeltonIdle;
	Animation skeltonShield;
	Animation skeltonHit;
	Animation skeltonWalk;

	Texture *playerTexture;

	bool isLookingAtItems;
	ItemGrowTimerUI lookingAt_animTimer;
	int indexInItems;

	//Extra player stuff we don't want to put on the entity
	EntityType playerHolding[2];


	ItemGrowTimerUI animationItemTimers[MAX_PLAYER_ITEM_COUNT];
	//NOTE: The two item spots the player can hold
	float animationItemTimersHUD[2];

	int itemAnimationCount;
	ItemAnimationInfo item_animations[32];
	//////


	//SOUNDS

	WavFile *clickSound;
	WavFile *equipItemSound;
	WavFile *openMenuSound;

	////


	//EDITOR STATE
	bool isEditorOpen;

	///////////////////

	EasyPhysics_World physicsWorld;

	char *currentSceneName;
	char *sceneFileNameTryingToSave;

	float inverse_weight;


	InfiniteAlloc splatList;
	InfiniteAlloc splatTextures;

	//Player variables loaded in tweak file
	float jumpPower;
	float walkPower;
	float gravityScale;
	float cameraSnapDistance;
	//

	void *currentTerrainEntity;

	EasyTerrainDataPacket terrainPacket;

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
	state->playerHolding[0] = ENTITY_NULL;
	state->playerHolding[1] = ENTITY_NULL;

	state->itemAnimationCount = 0;

	state->indexInItems = 0;

	state->openMenuSound = state->equipItemSound = state->clickSound = findSoundAsset("click2.wav");


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

	state->splatList = initInfinteAlloc(char *);
	state->splatTextures = initInfinteAlloc(Texture *);

	state->walkPower = 400;

	state->gravityScale = 150;

	EasyPhysics_beginWorld(&state->physicsWorld);



	state->terrainPacket.textureCount = 4;;
	state->terrainPacket.textures[0] = findTextureAsset("blend_grass.png");
	state->terrainPacket.textures[1] = findTextureAsset("blend_mud.png");
	state->terrainPacket.textures[2] = findTextureAsset("blend_path.png");
	state->terrainPacket.textures[3] = findTextureAsset("blend_grass2.jpg");

	state->terrainPacket.blendMap = findTextureAsset("blendMap.png");
	state->currentTerrainEntity = 0;

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

	return found;
}