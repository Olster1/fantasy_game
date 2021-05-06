
#define MY_DIALOG_TYPE(FUNC) \
FUNC(ENTITY_DIALOG_NULL)\
FUNC(ENTITY_DIALOG_ADVENTURER)\
FUNC(ENTITY_DIALOG_WEREWOLF_SIGN)\
FUNC(ENTITY_DIALOG_WELCOME_SIGN)\
FUNC(ENTITY_DIALOG_HOUSE)\
FUNC(ENTITY_DIALOG_FOREST_RIP)\

typedef enum {
    MY_DIALOG_TYPE(ENUM)
} DialogInfoType;

static char *MyDialog_DialogTypeStrings[] = { MY_DIALOG_TYPE(STRING) };

typedef struct EntityDialogNode EntityDialogNode;
typedef struct EntityDialogNode {
	int textCount;
	char *texts[4];

	int choiceCount;
	char *choices[4];

	//Add actions or items that the NPC might give the character here

	//to match the choices
	EntityDialogNode *next[4];
 } EntityDialogNode;



static void pushTextToNode(EntityDialogNode *n, char *text) {
	assert(n->textCount < arrayCount(n->texts));
	n->texts[n->textCount++] = text;
}

static void pushChoiceToNode(EntityDialogNode *n, char *text) {
	assert(n->choiceCount < arrayCount(n->choices));
	n->choices[n->choiceCount++] = text;
}

static void pushConnectionNode(EntityDialogNode *n, EntityDialogNode *n1, int index) {
	n->next[index] = n1;
}

typedef struct {
	//Fill out all the dialogs here
	EntityDialogNode *houseDialog;
	EntityDialogNode *philosophyDialog;
	EntityDialogNode *graveDialog;
	EntityDialogNode *errorDialogForDebugging;

	Arena perDialogArena;
	MemoryArenaMark perDialogArenaMark;
} GameDialogs;


static EntityDialogNode *constructDialogNode(GameDialogs *dialogs) {
	EntityDialogNode *result = pushStruct(&dialogs->perDialogArena, EntityDialogNode); 

	easyMemory_zeroStruct(result, EntityDialogNode);

	return result;
}


static void initDialogTrees(GameDialogs *gd) {

	gd->perDialogArena = createArena(Kilobytes(50));

	easyMemory_zeroStruct(&gd->perDialogArenaMark, MemoryArenaMark); 

	{

		EntityDialogNode *n = pushStruct(&globalLongTermArena, EntityDialogNode);	
		gd->houseDialog = n;

		pushTextToNode(n, "{s: 1}Hi, can you.{s: 1}.. {p: 1} can you do something for me?");
		pushChoiceToNode(n, "What is it?");
		pushChoiceToNode(n, "No way.");

		EntityDialogNode *n1 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		pushTextToNode(n1, "{s: 2}You know what. There's some secrets up there.");
		pushTextToNode(n1, "{s: 2}Hidden deep in the caves of Moria you'll find something.");
		pushChoiceToNode(n1, "Wow, tell me more.");
		pushChoiceToNode(n1, "That doesn't sound great at all.");
		pushChoiceToNode(n1, "I need some dough. Can we do this together?");

		EntityDialogNode *n3 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		pushTextToNode(n3, "{s: 2}Ok, forget I asked.");

		pushConnectionNode(n, n1, 0);
		pushConnectionNode(n, n3, 1);

		EntityDialogNode *n2 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		pushTextToNode(n2, "{s: 2}I've got nothing else to say.");

		pushConnectionNode(n1, n2, 0);
		pushConnectionNode(n1, n2, 1);
		pushConnectionNode(n1, n2, 2);

	}
	//graveDialog
	{
		EntityDialogNode *n = pushStruct(&globalLongTermArena, EntityDialogNode);	
		gd->graveDialog = n;

		pushTextToNode(n, "{s: 1}R{p: 0.5}.I{p: 0.5}.P{p: 0.5}. Here lays the protector of this forest.");
	}

	//Error dialog for debugging when you haven't set the type to link up with the 
	{
		EntityDialogNode *n = pushStruct(&globalLongTermArena, EntityDialogNode);	
		gd->errorDialogForDebugging = n;

		pushTextToNode(n, "ERROR: You didn't link up the dialog type to the actual dialog.");
	}
	


	//Philosophy dialog
	{

		// EntityDialogNode *n = pushStruct(&globalLongTermArena, EntityDialogNode);	
		// gd->houseDialog = n;

		// pushTextToNode(n, "{s: 2}How do you know what's real?.");
		// pushChoiceToNode(n, "I can see it, measure it and tomorrow I'll get the same result.");
		// pushChoiceToNode(n, "I don't know.");
		// pushChoiceToNode(n, "It just is");

		// EntityDialogNode *n1 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		// pushTextToNode(n1, "{s: 2}It's.");
		// pushTextToNode(n1, "{s: 2}Hidden deep in the caves of Moria you'll find something.");
		// pushChoiceToNode(n1, "Wow, tell me more.");
		// pushChoiceToNode(n1, "That doesn't sound great at all.");
		// pushChoiceToNode(n1, "I need some dough. Can we do this together?");

		// EntityDialogNode *n3 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		// pushTextToNode(n3, "{s: 2}Ok, forget I asked. Suit yourself.");

		// pushConnectionNode(n, n1, 0);
		// pushConnectionNode(n, n3, 1);
		// pushConnectionNode(n, n1, 2);

		// EntityDialogNode *n2 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		// pushTextToNode(n2, "{s: 2}I've got nothing else to say.");

		// pushConnectionNode(n1, n2, 0);
		// pushConnectionNode(n1, n2, 1);
		// pushConnectionNode(n1, n2, 2);

	}


	
}


static EntityDialogNode *findDialogInfo(DialogInfoType type, GameDialogs *gd) {
	EntityDialogNode *result = gd->errorDialogForDebugging;

	if(false) {

	} else if(type == ENTITY_DIALOG_ADVENTURER) {
	} else if(type == ENTITY_DIALOG_WEREWOLF_SIGN) {
	} else if(type == ENTITY_DIALOG_WELCOME_SIGN) {
	} else if(type == ENTITY_DIALOG_FOREST_RIP) {
		result = gd->graveDialog;
	} else if(type == ENTITY_DIALOG_HOUSE) {
		result = gd->houseDialog;
	}

	return result;
}
//////////////////////////////////////////////////////////////////////////