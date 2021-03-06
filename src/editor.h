typedef enum {
	EDITOR_CREATE_SELECT_MODE,
	EDITOR_CREATE_SCENERY,
	EDITOR_CREATE_SCENERY_RIGID_BODY,
	EDITOR_CREATE_ONE_WAY_PLATFORM,
	EDITOR_CREATE_SKELETON,
	EDITOR_CREATE_CHECKPOINT,
	EDITOR_CREATE_TORCH,
	EDITOR_CREATE_AUDIO_CHECKPOINT,
	EDITOR_CREATE_TERRAIN
} EditorCreateMode;

typedef enum {
	EDITOR_GIZMO_NONE,
	EDITOR_GIZMO_TOP_RIGHT,
	EDITOR_GIZMO_TOP_LEFT,
	EDITOR_GIZMO_BOTTOM_LEFT,
	EDITOR_GIZMO_BOTTOM_RIGHT,
	EDITOR_GIZMO_ANGLE
} EditorGizmoSelect;

char *EditorCreateModesStrings[] = { "Select", "Scenery", "Scenery with RB", "Platform One Way", "Skeleton", "Checkpoint", "Torch", "Audio Checkpoint", "Terrain2d"};

typedef struct {
	void *entitySelected;
	int entityIndex;
	V2 grabOffset;

	EditorCreateMode createMode;

	//For Gizmo Control
	EditorGizmoSelect gizmoSelect;
	V2 startMouseP_inWorldP;
	//

} EditorState;

static EditorState *initEditorState(Arena *arena) {
	EditorState *result = pushStruct(arena, EditorState);

	result->entitySelected = 0;
	result->entityIndex = 0;

	result->grabOffset = v2(0, 0);

	result->gizmoSelect = EDITOR_GIZMO_NONE;
	result->startMouseP_inWorldP = v2(0, 0);

	return result;
}