#include <sys/stat.h>

typedef struct {
	char *name;
	InfiniteAlloc data;
	VarType type;
} TweakVar;

typedef struct {
	TweakVar vars[32];
	int varCount;
	long modTime;
} Tweaker;


TweakVar *addTweakVar(Tweaker *tweaker, char *name, InfiniteAlloc *data) {
	//int hashValue = getHashForString(name, arrayCount(tweaker->vars));
	assert(tweaker->varCount < arrayCount(tweaker->vars));
	TweakVar *var = tweaker->vars + tweaker->varCount++;

	var->name = name;
	easyArray_copyInfiniteAlloc(data, &var->data);
	assert(var->data.memory);

	return var;
}

TweakVar *findTweakVar(Tweaker *tweaker, char *name) {
	TweakVar *result = 0;
	for(int i = 0; i < tweaker->varCount && !result; ++i) {
		TweakVar *var = tweaker->vars + i;
		if(cmpStrNull(var->name, name)) {
			result = var;
			break;
		}
	}
	return result;
}

V3 getV3FromTweakData(Tweaker *tweaker, char *name) {
	TweakVar *var = findTweakVar(tweaker, name);
	assert(var);
	DataObject *objs = (DataObject *)var->data.memory;
	assert(objs[0].type == VAR_FLOAT);
	assert(objs[1].type == VAR_FLOAT);
	assert(objs[2].type == VAR_FLOAT);
	assert(var->data.count == 3);

	V3 result = v3(objs[0].floatVal, objs[1].floatVal, objs[2].floatVal);
	return result;
}

V2 getV2FromTweakData(Tweaker *tweaker, char *name) {
	TweakVar *var = findTweakVar(tweaker, name);
	assert(var);
	DataObject *objs = (DataObject *)var->data.memory;
	assert(objs[0].type == VAR_FLOAT);
	assert(objs[1].type == VAR_FLOAT);
	assert(var->data.count == 2);

	V2 result = v2(objs[0].floatVal, objs[1].floatVal);
	return result;
}

float getFloatFromTweakData(Tweaker *tweaker, char *name) {
	TweakVar *var = findTweakVar(tweaker, name);
	assert(var);
	DataObject *objs = (DataObject *)var->data.memory;
	assert(objs[0].type == VAR_FLOAT || objs[0].type == VAR_INT);
	assert(var->data.count == 1);

	float result = 0;
	if(objs[0].type == VAR_FLOAT) {
		result = objs[0].floatVal;
	} else if(objs[0].type == VAR_INT) {
		result = objs[0].intVal;
	}
	
	return result;
}

char *getStringFromTweakData(Tweaker *tweaker, char *name) {
    TweakVar *var = findTweakVar(tweaker, name);
    assert(var);
	DataObject *objs = (DataObject *)var->data.memory;
    assert(objs[0].type == VAR_CHAR_STAR);

    char *result = objs[0].stringVal;

    return result;
}
#define getIntFromTweakData(tweaker, name) (int)getIntFromTweakData_(tweaker, name)
#define getULongFromTweakData(tweaker, name) getIntFromTweakData_(tweaker, name)
#define getLongFromTweakData(tweaker, name) (long)getIntFromTweakData_(tweaker, name)

unsigned long getIntFromTweakData_(Tweaker *tweaker, char *name) {
    TweakVar *var = findTweakVar(tweaker, name);
    assert(var);
	DataObject *objs = (DataObject *)var->data.memory;
    assert(objs[0].type == VAR_INT);
    
    unsigned long result = objs[0].intVal;

    return result;
}

bool getBoolFromTweakData(Tweaker *tweaker, char *name) {
    TweakVar *var = findTweakVar(tweaker, name);
    assert(var);
	DataObject *objs = (DataObject *)var->data.memory;
    assert(objs[0].type == VAR_BOOL);
    
    bool result = objs[0].boolVal;

    return result;
}


static bool easyFile_canOpenFile_forRead(char *FileName) {
    assert(FileName);
    SDL_RWops* FileHandle = SDL_RWFromFile(FileName, "r");
    bool result = false;

    if(FileHandle) {
        result = true;
        SDL_RWclose(FileHandle);
    }
    return result;
}

bool refreshTweakFile(char *fileName, Tweaker *tweaker) {
	bool refreshed = false;
	struct stat result;
	long mod_time = 0;
	if(stat(fileName, &result) == 0) {
	    mod_time = result.st_mtime;
	}

	if((mod_time > tweaker->modTime) && easyFile_canOpenFile_forRead(fileName)) {
		refreshed = true;
		if(tweaker->varCount) {
			for(int i = 0; i < tweaker->varCount; ++i) {
				TweakVar *var = tweaker->vars + i;
				free(var->name);
				releaseInfiniteAlloc(&var->data);
			}
			tweaker->varCount = 0;
		}

		FileContents contents = getFileContentsNullTerminate(fileName);

	   tweaker->modTime = mod_time;

		assert(contents.memory);
		assert(contents.valid);

		EasyTokenizer tokenizer = lexBeginParsing((char *)contents.memory, EASY_LEX_OPTION_EAT_WHITE_SPACE);

		bool parsing = true;
		while(parsing) {
		    EasyToken token = lexGetNextToken(&tokenizer);
		    
		    switch(token.type) {
		        case TOKEN_NULL_TERMINATOR: {
		            parsing = false;
		        } break;
		        case TOKEN_WORD: {
		        	InfiniteAlloc *data = getDataObjects(&tokenizer);

		        	char *name = nullTerminate(token.at, token.size);
		        	addTweakVar(tweaker, name, data);

		        	tokenizer.typesArray.count = 0;
		        } break;
		        default: {

		        }
		    }
		}

		releaseInfiniteAlloc(&tokenizer.typesArray);

		free(contents.memory);
	}
	return refreshed;
}