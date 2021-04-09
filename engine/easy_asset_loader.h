#define loadAndAddImagesToAssets(folderName, loadImmediate) loadAndAddImagesToAssets_(concatInArena(globalExeBasePath, folderName, &globalPerFrameArena), loadImmediate)
int loadAndAddImagesToAssets_(char *folderNameAbsolute, bool loadImmediate) {
	DEBUG_TIME_BLOCK()
	char *imgFileTypes[] = {"jpg", "jpeg", "png", "bmp", "PNG"};
	FileNameOfType fileNames = getDirectoryFilesOfType(folderNameAbsolute, imgFileTypes, arrayCount(imgFileTypes));
	int result = fileNames.count;
	
	for(int i = 0; i < fileNames.count; ++i) {
	    char *fullName = fileNames.names[i];
	    char *shortName = getFileLastPortion(fullName);
	    if(shortName[0] != '.') { //don't load hidden file 
	        Asset *asset = findAsset(shortName);
	        assert(!asset);
	        if(!asset) {
	        	bool premultiplyAlpha = true;
	            asset = loadImageAsset(fullName, premultiplyAlpha, loadImmediate);
	        }
	        asset = findAsset(shortName);
	        assert(shortName);
	    }
	    free(fullName);
	    free(shortName);
	}
	return result;
}

#define loadAndAddImagesStripToAssets(animation, fileName, widthPerImage, hasMipMaps) loadAndAddImagesStripToAssets_(animation, concatInArena(globalExeBasePath, fileName, &globalPerFrameArena), widthPerImage, hasMipMaps, false)
void loadAndAddImagesStripToAssets_(Animation *animation, char *folderNameAbsolute, float widthPerImage, bool hasMipMaps, bool useWidth) {
	DEBUG_TIME_BLOCK()

	bool premultiplyAlpha = true;
    Texture texOnStack = loadImage(folderNameAbsolute, TEXTURE_FILTER_LINEAR, hasMipMaps, premultiplyAlpha);
    
    if(useWidth) {
    	char *shortName = getFileLastPortionWithoutExtension_arena(folderNameAbsolute, &globalPerFrameArena);
    	int strLength = easyString_getSizeInBytes_utf8(shortName);
    	char *at = shortName;

    	int byteLeft = strLength;

    	char *result = 0;
    	while(byteLeft > 0) {
    		if(at[byteLeft - 1] >= '0' && at[byteLeft - 1] <= '9') {
    			result = &at[byteLeft - 1];
    		} else {
    			break;
    		}
    		byteLeft--;
    	}
    	assert(result);
    	int frameCount = atoi(result); //get the number of frames from the end of the filename
    	assert(frameCount > 0);
    	widthPerImage = texOnStack.width / frameCount; 
    }

    int count = 0;

    float xAt = 0;

    char *shortName = getFileLastPortionWithoutExtension_arena(folderNameAbsolute, &globalPerFrameArena);
    float widthTruncated = ((int)(texOnStack.width / widthPerImage))*widthPerImage;
    while(xAt < widthTruncated) {
    	Texture *tex = pushStruct(&globalLongTermArena, Texture);
    	easyPlatform_copyMemory(tex, &texOnStack, sizeof(Texture));

    	tex->uvCoords.minX = xAt / texOnStack.width;



    	xAt += widthPerImage;

    	tex->uvCoords.maxX = xAt / texOnStack.width;

    	tex->aspectRatio_h_over_w = ((float)texOnStack.height) / ((float)(tex->uvCoords.maxX - tex->uvCoords.minX)*(float)texOnStack.width);

    	char countAsString[128];

    	sprintf_s(countAsString, arrayCount(countAsString), "_%d", count);

    	char *tempFileName = concatInArena(shortName, countAsString, &globalPerFrameArena);

    	easyAnimation_pushFrame(animation, tempFileName);

    	Asset *result = addAssetTexture(tempFileName, folderNameAbsolute, tex);

    	count++;
    }

}


#define loadAndAddImagesStripToAssets_count_offset(animation, fileName, widthPerImage, hasMipMaps, count, offset) loadAndAddImagesStripToAssets_count_offset_(animation, concatInArena(globalExeBasePath, fileName, &globalPerFrameArena), widthPerImage, hasMipMaps, count, offset)
void loadAndAddImagesStripToAssets_count_offset_(Animation *animation, char *folderNameAbsolute, float widthPerImage, bool hasMipMaps, int totalCount, int offset) {
	DEBUG_TIME_BLOCK()

	bool premultiplyAlpha = true;
    Texture texOnStack = loadImage(folderNameAbsolute, TEXTURE_FILTER_LINEAR, hasMipMaps, premultiplyAlpha);
    
    int count = 0;
    float xAt = 0;

    xAt = offset*widthPerImage;

    char *shortName = getFileLastPortionWithoutExtension_arena(folderNameAbsolute, &globalPerFrameArena);
    float widthTruncated = ((int)(texOnStack.width / widthPerImage))*widthPerImage;
    while(xAt < widthTruncated && count < totalCount) {
    	Texture *tex = pushStruct(&globalLongTermArena, Texture);
    	easyPlatform_copyMemory(tex, &texOnStack, sizeof(Texture));

    	tex->uvCoords.minX = xAt / texOnStack.width;

    	xAt += widthPerImage;

    	tex->uvCoords.maxX = xAt / texOnStack.width;

    	tex->aspectRatio_h_over_w = ((float)texOnStack.height) / ((tex->uvCoords.maxX - tex->uvCoords.minX)*(float)texOnStack.width);

    	char countAsString[128];

    	sprintf_s(countAsString, arrayCount(countAsString), "_%d", count);

    	char *tempFileName = concatInArena(shortName, countAsString, &globalPerFrameArena);

    	easyAnimation_pushFrame(animation, tempFileName);

    	Asset *result = addAssetTexture(tempFileName, folderNameAbsolute, tex);

    	count++;
    }

}






#define loadAndAddImagesStripToAssets_xy(animation, fileName, widthPerImage, heightPerImage, missImages) loadAndAddImagesStripToAssets_xy_(animation, concatInArena(globalExeBasePath, fileName, &globalPerFrameArena), widthPerImage, heightPerImage, missImages)
void loadAndAddImagesStripToAssets_xy_(Animation *animation, char *folderNameAbsolute, float widthPerImage, float heightPerImage, int missImages) {
	DEBUG_TIME_BLOCK()

	bool premultiplyAlpha = true;
    Texture texOnStack = loadImage(folderNameAbsolute, TEXTURE_FILTER_LINEAR, true, premultiplyAlpha);
    
    int count = 0;

    float xAt = 0;
    float yAt = 0;

    char *shortName = getFileLastPortionWithoutExtension_arena(folderNameAbsolute, &globalPerFrameArena);

    int totalCount = ((int)(texOnStack.height / heightPerImage))*((int)(texOnStack.width / widthPerImage));

    float heightTruncated = ((int)(texOnStack.height / heightPerImage))*heightPerImage;
    float widthTruncated = ((int)(texOnStack.width / widthPerImage))*widthPerImage;

   	while(yAt < heightTruncated) {
	    while(xAt < widthTruncated && (count < (totalCount - missImages))) {
	    	Texture *tex = pushStruct(&globalLongTermArena, Texture);
	    	easyPlatform_copyMemory(tex, &texOnStack, sizeof(Texture));

	    	tex->uvCoords.minY = yAt / texOnStack.height;
	    	tex->uvCoords.maxY = (yAt + heightPerImage) / texOnStack.height;

	    	tex->uvCoords.minX = xAt / texOnStack.width;

	    	xAt += widthPerImage;

	    	tex->uvCoords.maxX = xAt / texOnStack.width;

	    	tex->aspectRatio_h_over_w = ((tex->uvCoords.maxY - tex->uvCoords.minY)*(float)texOnStack.height) / ((tex->uvCoords.maxX - tex->uvCoords.minX)*(float)texOnStack.width);

	    	char countAsString[128];

	    	sprintf_s(countAsString, arrayCount(countAsString), "_%d", count);

	    	char *tempFileName = concatInArena(shortName, countAsString, &globalPerFrameArena);

	    	easyAnimation_pushFrame(animation, tempFileName);

	    	Asset *result = addAssetTexture(tempFileName, folderNameAbsolute, tex);

	    	count++;
	    }
	    yAt += heightPerImage;
	    xAt = 0;
	}

}

int loadAndAddSoundsToAssets(char *folderName, SDL_AudioSpec *audioSpec) {
	DEBUG_TIME_BLOCK()
	char *soundFileTypes[] = {"wav"};
	FileNameOfType soundFileNames = getDirectoryFilesOfType(concat(globalExeBasePath, folderName), soundFileTypes, arrayCount(soundFileTypes));
	int result = soundFileNames.count;
	for(int i = 0; i < soundFileNames.count; ++i) {
	    char *fullName = soundFileNames.names[i];
	    char *shortName = getFileLastPortion(fullName);
	    if(shortName[0] != '.') { //don't load hidden file 
	        Asset *asset = findAsset(shortName);
	        assert(!asset);
	        if(!asset) {
	            asset = loadSoundAsset(fullName, audioSpec);
	        }
	        asset = findAsset(shortName);
	        assert(shortName);
	    }
	    free(fullName);
	    free(shortName);
	}
	return result;
}

typedef struct {
	union {
		struct { //for the models
			float scale; 
			EasyModel *model;
		};
	};
	
	
} EasyAssetLoader_AssetInfo;

typedef struct {
	int count;
	AssetType assetType;
	EasyAssetLoader_AssetInfo array[512];
} EasyAssetLoader_AssetArray;

typedef enum {
	EASY_ASSET_LOADER_FLAGS_NULL = 0,
	EASY_ASSET_LOADER_FLAGS_COMPILED_OBJ = 1 << 0
} EasyAssetLoader_Flag;

//TODO(ollie): This should cover all asset types? 
static void easyAssetLoader_loadAndAddAssets(EasyAssetLoader_AssetArray *result, char *folderName, char **fileTypes, int fileTypeCount, AssetType resourceType, EasyAssetLoader_Flag flags) {
	DEBUG_TIME_BLOCK()
	//NOTE(ollie): Get the names
	FileNameOfType allFileNames = getDirectoryFilesOfType(concat(globalExeBasePath, folderName), fileTypes, fileTypeCount);
	if(result) {
		assert(result->assetType == resourceType);
	}
	//loop through all the names
	for(int i = 0; i < allFileNames.count; ++i) {
		//NOTE(ollie): Get the fullname
	    char *fullName = allFileNames.names[i];

	    //NOTE(ollie): Get the short name for testing purposes
	    char *shortName = getFileLastPortion(fullName);

	    
	    //NOTE(ollie): don't load hidden file 
	    if(shortName[0] != '.') { 

	        Asset *asset = findAsset(shortName);
	        assert(!asset);
	        if(!asset) {
	        	if(resourceType == ASSET_MATERIAL) {
	        		//NOTE(ollie): Load the material
	        		easy3d_loadMtl(fullName, EASY_FILE_NAME_GLOBAL);
	        	} else if(resourceType == ASSET_MODEL) {
	        		assert(result->count < arrayCount(result->array));

	        		EasyAssetLoader_AssetInfo *modelInfo = &result->array[result->count++];
	        		EasyModel *model = pushStruct(&globalLongTermArena, EasyModel);
	        		modelInfo->model = model;
	        		if(flags & EASY_ASSET_LOADER_FLAGS_COMPILED_OBJ) {
	        			easy3d_loadCompiledObj_version1(fullName, model);
	        		} else {
	        			easy3d_loadObj(fullName, model, EASY_FILE_NAME_GLOBAL);	
	        		}
	        		
	        		
	        		float axis = easyMath_getLargestAxis(model->bounds);
	        		float relAxis = 5.0f;
	        		modelInfo->scale = relAxis / axis;
	        	}
	        	
	        }
#if DEVELOPER_MODE
	        // asset = findAsset(shortName);
	        // assert(asset);
#endif
	        
	    }
	    free(fullName);
	    free(shortName);
	}
}



static void easyAssetLoader_loadAndCompileObjsFiles(char **modelDirs, u32 modelDirCount) {
	for(int dirIndex = 0; dirIndex < modelDirCount; ++dirIndex) {
	    char *dir = modelDirs[dirIndex];

	    char *folder = concatInArena(globalExeBasePath, dir, &globalPerFrameArena);
	    {
		    //NOTE(ollie): Load materials first
		    char *fileTypes[] = { "mtl"};

		    //NOTE(ollie): Get the names
		    FileNameOfType allFileNames = getDirectoryFilesOfType(folder, fileTypes, arrayCount(fileTypes));
		    //loop through all the names
		    for(int i = 0; i < allFileNames.count; ++i) {
		        //NOTE(ollie): Get the fullname
		        char *fullName = allFileNames.names[i];
		        char *shortName = getFileLastPortion(fullName);
		        //NOTE(ollie): don't load hidden file 
		        if(shortName[0] != '.') { 

		            //NOTE(ollie): Load the material
		            easy3d_loadMtl(fullName, EASY_FILE_NAME_GLOBAL);
		        }
		        free(fullName);
		        free(shortName);
		    }
		}

	    ///////////////////////*********** Load Objs **************////////////////////
	    {
		    //NOTE(ollie): Load materials first
		    char *fileTypes[] = { "obj"};

		    //NOTE(ollie): Get the names
		    FileNameOfType allFileNames = getDirectoryFilesOfType(folder, fileTypes, arrayCount(fileTypes));
		    //loop through all the names
		    for(int i = 0; i < allFileNames.count; ++i) {
		        //NOTE(ollie): Get the fullname
		        char *fullName = allFileNames.names[i];
		        char *shortName = getFileLastPortion(fullName);
		        //NOTE(ollie): don't load hidden file 
		        if(shortName[0] != '.') { 
		            //NOTE(ollie): Make obj file
		            char *name = getFileLastPortionWithoutExtension_arena(shortName, &globalPerFrameArena);
		            char *nameWithExtension = concatInArena(name, ".easy3d", &globalPerFrameArena);
		            char *folderAndName = concatInArena("compiled_models/", nameWithExtension, &globalPerFrameArena);
		            easy3d_compileObj(fullName, concatInArena(globalExeBasePath, folderAndName, &globalPerFrameArena));
		        }
		        free(shortName);
		        free(fullName);
		    }
		}
	    
	}
}