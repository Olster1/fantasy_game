 
char *ah_adventurerText[] = { "Ah an adventuer?", "It\'s not often we see your kind through these parts...", "Go on, go into the forest...", "You might not like what you find there...", "Heh heh heh heh heh...." };
char *ah_adventurerAudio[] = { "ah an adventurer.wav", "its not often.wav", "go on then.wav", "might not like.wav", "heheheh.wav" };


char *werewolfSign[] = { "Werewolfs seen here. BEWARE! "};


char *welcomeSign[] = { "{s: 1}Welcome to Illoway. Town of exceptional beauty. {s: 2.0}This is a faster sentence. {s: 0.5 c: 1 0 0 1}This is red and slow. "};

#define MY_DIALOG_TYPE(FUNC) \
FUNC(ENTITY_DIALOG_NULL)\
FUNC(ENTITY_DIALOG_ADVENTURER)\
FUNC(ENTITY_DIALOG_WEREWOLF_SIGN)\
FUNC(ENTITY_DIALOG_WELCOME_SIGN)\
FUNC(ENTITY_DIALOG_HOUSE)\


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
} GameDialogs;


static void initDialogTrees(GameDialogs *gd) {
	{

		EntityDialogNode *n = pushStruct(&globalLongTermArena, EntityDialogNode);	
		gd->houseDialog = n;

		pushTextToNode(n, "{s: 2}Welcome to Illoway. Town of exceptional beauty.");
		pushTextToNode(n, "Where are you from?");
		pushChoiceToNode(n, "The greater upland region.");
		pushChoiceToNode(n, "None of your business.");
		pushChoiceToNode(n, "The highlands.");

		EntityDialogNode *n1 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		pushTextToNode(n1, "{s: 2}You know what. There's some secrets up there.");
		pushTextToNode(n1, "{s: 2}Hidden deep in the caves of Moria you'll find something.");
		pushChoiceToNode(n1, "Wow, tell me more.");
		pushChoiceToNode(n1, "That doesn't sound great at all.");
		pushChoiceToNode(n1, "I need some dough. Can we do this together?");

		EntityDialogNode *n3 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		pushTextToNode(n3, "{s: 2}Ok, forget I asked. Suit yourself.");

		pushConnectionNode(n, n1, 0);
		pushConnectionNode(n, n3, 1);
		pushConnectionNode(n, n1, 2);

		EntityDialogNode *n2 = pushStruct(&globalLongTermArena, EntityDialogNode);	
		pushTextToNode(n2, "{s: 2}I've got nothing else to say.");

		pushConnectionNode(n1, n2, 0);
		pushConnectionNode(n1, n2, 1);
		pushConnectionNode(n1, n2, 2);

	}
	
}


static EntityDialogNode *findDialogInfo(DialogInfoType type, GameDialogs *gd) {
	EntityDialogNode *result = 0;

	if(false) {

	} else if(type == ENTITY_DIALOG_ADVENTURER) {
	} else if(type == ENTITY_DIALOG_WEREWOLF_SIGN) {
	} else if(type == ENTITY_DIALOG_WELCOME_SIGN) {
	} else if(type == ENTITY_DIALOG_HOUSE) {
		result = gd->houseDialog;
	}

	return result;
}
//////////////////////////////////////////////////////////////////////////