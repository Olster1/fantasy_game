
char *ah_adventurerText[] = { "Ah an adventuer?", "It\'s not often we see your kind through these parts...", "Go on, go into the forest...", "You might not like what you find there...", "Heh heh heh heh heh...." };
char *ah_adventurerAudio[] = { "ah an adventurer.wav", "its not often.wav", "go on then.wav", "might not like.wav", "heheheh.wav" };


char *werewolfSign[] = { "Werewolfs seen here. BEWARE! "};

#define MY_DIALOG_TYPE(FUNC) \
FUNC(ENTITY_DIALOG_NULL)\
FUNC(ENTITY_DIALOG_ADVENTURER)\
FUNC(ENTITY_DIALOG_WEREWOLF_SIGN)\


typedef enum {
    MY_DIALOG_TYPE(ENUM)
} DialogInfoType;

static char *MyDialog_DialogTypeStrings[] = { MY_DIALOG_TYPE(STRING) };


typedef struct {
	char **textArray;
	char **audioArray;
	int count;
} EntityDialogInfo;

static EntityDialogInfo findDialogInfo(DialogInfoType type) {
	EntityDialogInfo result = {};

	if(false) {

	} else if(type == ENTITY_DIALOG_ADVENTURER) {
		result.textArray = ah_adventurerText;
		result.audioArray = ah_adventurerAudio;
		result.count = arrayCount(ah_adventurerText);
	} else if(ENTITY_DIALOG_WEREWOLF_SIGN) {
		result.textArray = werewolfSign;
		result.audioArray = 0;
		result.count = arrayCount(werewolfSign);
	}

	return result;
}