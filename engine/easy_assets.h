typedef enum {
    ASSET_TEXTURE,
    ASSET_SOUND, 
    ASSET_ANIMATION,
    ASSET_EVENT,
    ASSET_MATERIAL,
    ASSET_MODEL      
} AssetType;

typedef struct
{
    
} Event;

typedef struct Asset Asset;
typedef struct Asset {
    char *name;
    char *fullFilePath; //to load if it hasn't loaded yet 

	void *file; //can check this if loaded

	Asset *next;
} Asset;

typedef struct {
    char *name;
    AssetType type;
} EasyAssetIdentifier;

#define GLOBAL_ASSET_ARRAY_SIZE (1 << 12) //4096
//NOTE(ol): This gets allocated in the easy_os when starting up the app
static Asset **assets = 0;

#define EASY_ASSET_IDENTIFIER_INCREMENT 512

typedef struct {
    u32 totalCount;
    u32 count;

    EasyAssetIdentifier *identifiers;

} EasyAssetIdentifier_State;

static EasyAssetIdentifier_State global_easyArrayIdentifierstate;

static void easyAssets_initAssetIdentifier(EasyAssetIdentifier_State *state) {
    state->count = 0;
    state->totalCount = 0;
    state->identifiers = 0;
}

static void easyAssets_addAssetIdentifier(EasyAssetIdentifier_State *state, char *name, AssetType type) {
    EasyAssetIdentifier id = {0};
    id.name = name;
    id.type = type;

    if(state->count >= state->totalCount) {
        //NOTE(ollie): Resize the array 
        u32 oldSize = state->count*sizeof(EasyAssetIdentifier);

        state->totalCount += EASY_ASSET_IDENTIFIER_INCREMENT;

        u32 newSize = state->totalCount*sizeof(EasyAssetIdentifier);

        if(state->identifiers) {
            state->identifiers = (EasyAssetIdentifier *)easyPlatform_reallocMemory(state->identifiers, oldSize, newSize);
        } else {
            state->identifiers = (EasyAssetIdentifier *)easyPlatform_allocateMemory(newSize, EASY_PLATFORM_MEMORY_NONE);
        }

    }

    assert(state->count < state->totalCount);
    state->identifiers[state->count++] = id;
    
}

int getAssetHash(char *at, int maxSize) {
    DEBUG_TIME_BLOCK()
	int hashKey = 0;
    while(*at) {
        //Make the hash look up different prime numbers. 
        hashKey += (*at)*19;
        at++;
    }
    hashKey = hashKey & (maxSize - 1);
    return hashKey;
}

Asset *findAsset(char *fileName) {
    DEBUG_TIME_BLOCK()
    int hashKey = getAssetHash(fileName, GLOBAL_ASSET_ARRAY_SIZE);
    
    Asset *file = assets[hashKey];
    Asset *result = 0;
    
    bool found = false;
    
    while(!found && file) {
        assert(file);
        // assert(file->file);
        assert(file->name);
        if(cmpStrNull(fileName, file->name)) {
            result = file;
            found = true;
        } else {
            file = file->next;
        }
    }
    return result;
}

inline static EasyModel *findModelAsset_Safe(char *fileName) { 
    DEBUG_TIME_BLOCK()
    Asset *a = findAsset(fileName); 
    EasyModel *result = 0;
    if(a) {
        result = (EasyModel *)a->file;

        if(!result) {
            //Load asset 
        }
    }
    return result;

}

static Texture *getTextureAsset(Asset *assetPtr) {
    Texture *result = (Texture *)(assetPtr->file);

    if(!result) {
        bool premultiplyAlpha = true;
        
        Texture texOnStack = loadImage(assetPtr->fullFilePath, TEXTURE_FILTER_LINEAR, true, premultiplyAlpha);
        Texture *tex = (Texture *)calloc(sizeof(Texture), 1);
        memcpy(tex, &texOnStack, sizeof(Texture));
        
        assetPtr->file = tex;
        result = (Texture *)(assetPtr->file);
    }

    assert(result);
    return result;
}

#define findModelAsset(fileName) (EasyModel *)findAsset(fileName)->file
#define findMaterialAsset(fileName) (EasyMaterial *)findAsset(fileName)->file
#define findTextureAsset(fileName) getTextureAsset(findAsset(fileName))
#define findSoundAsset(fileName) (WavFile *)findAsset(fileName)->file
#define findEventAsset(fileName) (Event *)findAsset(fileName)->file

static WavFile *getSoundAsset(Asset *assetPtr) {
    WavFile *result = (WavFile *)(assetPtr->file);
    assert(result);
    return result;
}

static Event *getEventAsset(Asset *assetPtr) {
    Event *result = (Event *)(assetPtr->file);
    assert(result);
    return result;
}

static Asset *addAsset_(char *fileName, char *fullFilePath_, void *asset) { 
    DEBUG_TIME_BLOCK()
    char *truncName = getFileLastPortion(fileName);
    int hashKey = getAssetHash(truncName, GLOBAL_ASSET_ARRAY_SIZE);
    assert(fileName != truncName);
    Asset **filePtr = assets + hashKey;

    char *fullFilePath = easyString_copyToHeap(fullFilePath_);
    
    bool found = false; 
    Asset *result = 0;
    while(!found) {
        Asset *file = *filePtr;
        if(!file) {
            file = (Asset *)easyPlatform_allocateMemory(sizeof(Asset), EASY_PLATFORM_MEMORY_ZERO);
            file->file = asset;
            file->name = truncName;
            file->fullFilePath = fullFilePath;
            file->next = 0;
            *filePtr = file;
            result = file;
            found = true;
        } else {
            filePtr = &file->next;
        }
    }
    assert(found);
    return result;
}

static void easyAsset_removeAsset(char *fileName) {
    DEBUG_TIME_BLOCK()

    int hashKey = getAssetHash(fileName, GLOBAL_ASSET_ARRAY_SIZE);
    
    Asset **file = &assets[hashKey];
    Asset *result = 0;
    
    bool found = false;
    
    while(!found && *file) {
        if(cmpStrNull(fileName, (*file)->name)) {
            Asset *asset = *file;
            easyPlatform_freeMemory(asset->name);
            easyPlatform_freeMemory(asset->fullFilePath);

            *file = asset->next;

            if(asset) {
                easyPlatform_freeMemory(asset);    
            }
            
        } else {
            file = &(*file)->next;
        }
    }
}

Asset *addAssetTexture(char *fileName, char *fullFileName, Texture *asset) { // we have these for type checking
    Asset *result = addAsset_(fileName, fullFileName, asset);

    easyAssets_addAssetIdentifier(&global_easyArrayIdentifierstate, result->name, ASSET_TEXTURE);
    return result;
}

Asset *addAssetSound(char *fileName, char *fullFileName, WavFile *asset) { // we have these for type checking
    Asset *result = addAsset_(fileName, fullFileName, asset);
    easyAssets_addAssetIdentifier(&global_easyArrayIdentifierstate, result->name, ASSET_SOUND);
    return result;
}

Asset *addAssetEvent(char *fileName, char *fullFileName, Event *asset) { // we have these for type checking
    Asset *result = addAsset_(fileName, fullFileName, asset);
    easyAssets_addAssetIdentifier(&global_easyArrayIdentifierstate, result->name, ASSET_EVENT);
    return result;
}

Asset *addAssetMaterial(char *fileName, char *fullFileName, EasyMaterial *asset) { // we have these for type checking
    assert(asset);
    Asset *result = addAsset_(fileName, fullFileName, asset);
    easyAssets_addAssetIdentifier(&global_easyArrayIdentifierstate, result->name, ASSET_MATERIAL);
    return result;
}

Asset *addAssetModel(char *fileName,char *fullFileName,  EasyModel *asset) { // we have these for type checking
    Asset *result = addAsset_(fileName, fullFileName, asset);
    easyAssets_addAssetIdentifier(&global_easyArrayIdentifierstate, result->name, ASSET_MODEL);
    return result;
}

Asset *loadImageAsset(char *fileName, bool premultiplyAlpha) {
    DEBUG_TIME_BLOCK()
    
    // Texture texOnStack = loadImage(fileName, TEXTURE_FILTER_LINEAR, true, premultiplyAlpha);
    // Texture *tex = (Texture *)calloc(sizeof(Texture), 1);
    // memcpy(tex, &texOnStack, sizeof(Texture));


    //We don't load on command now, we only load when asset is used
    Asset *result = addAssetTexture(fileName, fileName, 0);
    assert(result);
    return result;
}

Asset *loadSoundAsset(char *fileName, SDL_AudioSpec *audioSpec) {
    DEBUG_TIME_BLOCK()
    WavFile *sound = (WavFile *)calloc(sizeof(WavFile), 1);
    loadWavFile(sound, fileName, audioSpec);
    Asset *result = addAssetSound(fileName, fileName, sound);
    assert(result);
    //free(fileName);
    return result;
}