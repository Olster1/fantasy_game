typedef enum {
	ENTITY_NULL = 0,
	ENTITY_SCENERY = 1,
	ENTITY_WIZARD = 2,
	ENTITY_PLAYER_PROJECTILE = 3,
	ENTITY_SKELETON = 4,
	ENTITY_HEALTH_POTION_1 = 5,
} EntityType;

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

	////



	EasyPhysics_World physicsWorld;
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

	state->clickSound = findSoundAsset("click2.wav");


	//NOTE: Initialize the ui item pickers to nothing
	for(int i = 0; i < arrayCount(state->animationItemTimers); ++i) {
		state->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
		state->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
	}

	state->animationItemTimersHUD[0] = -1.0f;
	state->animationItemTimersHUD[1] = -1.0f;

	return state;
}	