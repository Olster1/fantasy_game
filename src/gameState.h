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

	return state;
}	