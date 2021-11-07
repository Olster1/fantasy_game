typedef enum {
	EDITOR_CREATE_SELECT_MODE,
	EDITOR_CREATE_TILE_MODE,
	EDITOR_CREATE_ENTITY_BOARD_MODE,
	EDITOR_CREATE_SCENERY,
	EDITOR_CREATE_SCENERY_RIGID_BODY,
	EDITOR_CREATE_TRIGGER_WITH_RIGID_BODY,
	EDITOR_CREATE_EMPTY_TRIGGER,
	EDITOR_CREATE_CHEST,
	EDITOR_CREATE_ENEMY,
	EDITOR_CREATE_SIGN,
	EDITOR_CREATE_BLOCK_TO_PUSH,
	EDITOR_CREATE_ENTITY_CREATOR,
	EDITOR_CREATE_ENTITY_FOG,
	EDITOR_CREATE_SHOOT_TRIGGER,
	EDITOR_CREATE_LAMP_POST,
} EditorCreateMode;

#define MY_TILE_EDITOR_OPTION(FUNC) \
FUNC(EDITOR_TILE_SINGLE)\
FUNC(EDITOR_TILE_DRAG)\

typedef enum {
    MY_TILE_EDITOR_OPTION(ENUM)
} EditorTileOption;

static char *MyTiles_editorOptionStrings[] = { MY_TILE_EDITOR_OPTION(STRING) };

typedef enum {
	EDITOR_GIZMO_NONE,
	EDITOR_GIZMO_TOP_RIGHT,
	EDITOR_GIZMO_TOP_LEFT,
	EDITOR_GIZMO_BOTTOM_LEFT,
	EDITOR_GIZMO_BOTTOM_RIGHT,
	EDITOR_GIZMO_ANGLE
} EditorGizmoSelect;

char *EditorCreateModesStrings[] = { "Select", "Tile Mode", "Board Mode", "Scenery", "Scenery with RB",  "Trigger With Rigid Body", "Empty Trigger", "Chest", "Enemy", "Dialog", "push block", "Entity Creator", "Fog", "Shoot Trigger", "Lamp Post"};

typedef struct {
	void *entitySelected;
	int entityIndex;
	V3 grabOffset;

	EditorCreateMode createMode;

	//For Gizmo Control
	EditorGizmoSelect gizmoSelect;
	V2 startMouseP_inWorldP;
	//

	float lerpTowardsTimer;
	V2 startLerpP;

	Array_Dynamic entitiesDeletedBuffer;


	//Used so you can click things on top of each other
	int idsLastSelectedCount;
	int idsLastSelected[512];
	//////////////////////////////////


	//// For making walls on a vertical plane
	V3 lastcreatedTileP_onGround;



	EditorTileOption tileOption;
	WorldTileType tileType;
	V2 topLeftCornerOfTile;
} EditorState;


typedef enum {
	EDITOR_UNDO_HAS_COLLIDER_FLAG,
	EDITOR_UNDO_HAS_RIGID_BODY_FLAG
} EditorUndoEntityFlag;

typedef enum {
	EDITOR_UNDO_ENTITY_DELETE,
} EditorUndoType;


typedef struct {
	EditorUndoType type;

	void *e;

	//we block copy the collider and rigid body
	int flags;
	EasyCollider c;
	EasyRigidBody rb;
} EditorUndoState;

static EditorState *initEditorState(Arena *arena) {
	EditorState *result = pushStruct(arena, EditorState);

	result->entitySelected = 0;
	result->entityIndex = 0;
	result->idsLastSelectedCount = 0;

	result->grabOffset = v3(0, 0, 0);

	result->gizmoSelect = EDITOR_GIZMO_NONE;
	result->startMouseP_inWorldP = v2(0, 0);

	result->lerpTowardsTimer = -1;
	result->startLerpP = v2(0, 0);

	initArray(&result->entitiesDeletedBuffer, EditorUndoState);

	return result;
}


static bool editorState_idInList(EditorState *state, int id) {
	bool result = false;
	for(int i = 0; i < state->idsLastSelectedCount && !result; i++) {
		if(state->idsLastSelected[i] == id) {
			result = true;
		}
	}

	return result;
}


static bool editorState_addToEndOfList(EditorState *state, int id) {
	bool result = false; 
	if(state->idsLastSelectedCount < arrayCount(state->idsLastSelected)) {
		state->idsLastSelected[state->idsLastSelectedCount++] = id;
		result = true;
	} 

	return result;
}

static void editorState_moveToEnd(EditorState *state, int id) {
	bool found = false;
	int indexAt = 0;
	for(int i = 0; i < state->idsLastSelectedCount && !found; i++) {
		if(state->idsLastSelected[i] == id) {
			indexAt = i;
			found = true;
			break;
		}
	}
	
	assert(found);

	//Now move them down
	for(int i = indexAt + 1; i < state->idsLastSelectedCount; i++) {
		state->idsLastSelected[i - 1] = state->idsLastSelected[i];
	}

	state->idsLastSelected[state->idsLastSelectedCount - 1] = id;

	
}

static void letGoOfSelectedEntity(EditorState *state) {
	state->entitySelected = 0;
	state->idsLastSelectedCount = 0;
}

typedef struct {
    int id;
    void *e;
    int index;
    V3 hitP;
    V3 entP;
} EditorEntitySelectInfo;

static void lerpToSelectedEntity(EditorState *editorState, V2 pos) {
	editorState->startLerpP = pos;
	editorState->lerpTowardsTimer = 0;
}
