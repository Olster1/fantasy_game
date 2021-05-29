static char *getAllScenesFolderName() {
	return concatInArena(globalExeBasePath, "scenes/", &globalPerFrameArena);
}

static char *getFullSceneFolderPath(char *sceneName, char *allScenesFolderName) {
	return concatInArena(allScenesFolderName, sceneName, &globalPerFrameArena);
}

static char *getFullNameForEntityFile(int entityID, char *fullSceneFolderPath) {
	char *entityFileName = easy_createString_printf(&globalPerFrameArena, "entity_%d.txt", entityID);
	return concatInArena(fullSceneFolderPath, entityFileName, &globalPerFrameArena);

}

static void gameScene_saveScene(GameState *gameState, EntityManager *manager, char *sceneName_) {
	DEBUG_TIME_BLOCK()

	char *sceneName = sceneName_;
	if(sceneName_[easyString_getSizeInBytes_utf8(sceneName) - 1] != '/' || sceneName_[easyString_getSizeInBytes_utf8(sceneName) - 1] != '\\') {
		sceneName = concatInArena(sceneName_, "/", &globalPerFrameArena);
	}

	easyConsole_addToStream(DEBUG_globalEasyConsole, "Saving Scene");



	char *allScenesFolderName = getAllScenesFolderName();
	char *fullSceneFolderPath = getFullSceneFolderPath(sceneName, allScenesFolderName);


	if(!platformDoesDirectoryExist(allScenesFolderName)) {
		platformCreateDirectory(allScenesFolderName);	
		easyConsole_addToStream(DEBUG_globalEasyConsole, "Scene folder created");
	}
	

	// easyConsole_addToStream(DEBUG_globalEasyConsole, fullSceneFolderPath);

	if(!platformDoesDirectoryExist(fullSceneFolderPath)) {
		platformCreateDirectory(fullSceneFolderPath);	
	}
	
    InfiniteAlloc fileContents = initInfinteAlloc(char);

    InfiniteAlloc aiNodeData = initInfinteAlloc(float);


    
	for(int i = 0; i < manager->entities.count; ++i) {
	    Entity *e = (Entity *)getElement(&manager->entities, i);
	    if(e && (e->flags & ENTITY_SHOULD_SAVE)) {
	        
	    	

	        char buffer[32];
	        sprintf(buffer, "{\n\n");
	        addElementInifinteAllocWithCount_(&fileContents, buffer, strlen(buffer));
	        	
	        addVar(&fileContents, &e->T.id, "id", VAR_INT);	
	        addVar(&fileContents, MyEntity_EntityTypeStrings[(int)e->type], "type", VAR_CHAR_STAR);
	        addVar(&fileContents, &e->T.pos, "position", VAR_V3);
	        addVar(&fileContents, &e->T.scale, "scale", VAR_V3);
	        addVar(&fileContents, &e->colorTint, "color", VAR_V4);
	        addVar(&fileContents, &e->T.Q.E, "rotation", VAR_V4);
	        addVar(&fileContents, &e->layer, "layer", VAR_FLOAT);
	        addVar(&fileContents, (int *)(&e->subEntityType), "subtype", VAR_INT);

            addVar(&fileContents, MyEntity_EnemyTypeStrings[(int)e->enemyType], "enemyType", VAR_CHAR_STAR);

	        if(e->sprite) {
	        	addVar(&fileContents, e->sprite->name, "spriteName", VAR_CHAR_STAR);
	        }
			
			if(e->collider) {
				addVar(&fileContents, &e->collider->dim2f, "colliderScale", VAR_V2);
                addVar(&fileContents, &e->collider->offset, "colliderOffset", VAR_V3);	
			}	        


            if(e->aiController) {
                //walk the ai board
                EasyAiController *aiController = e->aiController;
                for(int i = 0; i < arrayCount(aiController->boardHash); ++i) {
                    EasyAi_Node *n = aiController->boardHash[i];

                    while(n) {

                        float canSeePlayerFrom = (n->canSeePlayerFrom) ? 1.0f : 0.0f;

                        addElementInfinteAlloc_notPointer(&aiNodeData, n->pos.x);
                        addElementInfinteAlloc_notPointer(&aiNodeData, n->pos.y);
                        addElementInfinteAlloc_notPointer(&aiNodeData, n->pos.z);
                        addElementInfinteAlloc_notPointer(&aiNodeData, canSeePlayerFrom);

                        n = n->next;
                    }
                }

                addVarArray(&fileContents, aiNodeData.memory, aiNodeData.count, "aiNodes", VAR_FLOAT);

                aiNodeData.count = 0;

                //the search bouys
                for(int i = 0; i < aiController->searchBouysCount; ++i) {
                    V3 *n = &aiController->searchBouys[i];

                    addElementInfinteAlloc_notPointer(&aiNodeData, n->x);
                    addElementInfinteAlloc_notPointer(&aiNodeData, n->y);
                    addElementInfinteAlloc_notPointer(&aiNodeData, n->z);
                    
                }

                addVarArray(&fileContents, aiNodeData.memory, aiNodeData.count, "searchBouys", VAR_FLOAT);
                aiNodeData.count = 0;
            }

	        if(e->collider1) {
	        	addVar(&fileContents, &e->collider1->dim2f, "colliderScale2", VAR_V2);
                addVar(&fileContents, &e->collider1->offset, "colliderOffset2", VAR_V3);	
	        }	

            if(e->levelToLoad) {
                addVar(&fileContents, e->levelToLoad, "levelToLoad", VAR_CHAR_STAR);    
            }

	        if(e->audioFile) {
	        	addVar(&fileContents, e->audioFile, "audioFileName", VAR_CHAR_STAR);	
	        } 


            addVar(&fileContents, &e->lightColor, "lightColor", VAR_V3);
            addVar(&fileContents, (float *)(&e->lightIntensity), "lightIntensity", VAR_FLOAT);


            if(e->type == ENTITY_ENEMY) {
                addVar(&fileContents, &e->enemyMoveSpeed, "enemyMoveSpeed", VAR_FLOAT);
                
            }

            addVar(&fileContents, &e->renderFirstPass, "renderFirstPass", VAR_INT);    


            addVar(&fileContents, MyDialog_DialogTypeStrings[(int)e->dialogType], "dialogType", VAR_CHAR_STAR);

            if(e->triggerType != ENTITY_TRIGGER_NULL) {
                addVar(&fileContents, MyEntity_TriggerTypeStrings[(int)e->triggerType], "triggerType", VAR_CHAR_STAR);
            }

            if(e->chestType != ENTITY_TRIGGER_NULL) {
                addVar(&fileContents, MyEntity_ChestTypeStrings[(int)e->chestType], "chestType", VAR_CHAR_STAR);
            }

            if(e->locationSoundType != ENTITY_SOUND_NULL) {
                addVar(&fileContents, MyEntity_LocationSoundTypeStrings[(int)e->locationSoundType], "locationSoundType", VAR_CHAR_STAR);
            }

            addVar(&fileContents, &e->rateOfCreation, "rateOfCreation", VAR_FLOAT);
            addVar(&fileContents, MyEntity_EntityTypeStrings[(int)e->typeToCreate], "typeToCreate", VAR_CHAR_STAR);
                
            bool shouldNotRender = e->flags & ENTITY_SHOULD_NOT_RENDER;
            addVar(&fileContents, &shouldNotRender, "shouldNotRender", VAR_BOOL);


            if((e->flags & ENTITY_SHOULD_SAVE_ANIMATION) && !easyAnimation_isControllerEmpty(&e->animationController)) {
                   
                char *animationName = e->animationController.parent.next->animation->name;
                addVar(&fileContents, animationName, "animationName", VAR_CHAR_STAR);  
            }

            if(e->model) {
                addVar(&fileContents, e->model->name, "model", VAR_CHAR_STAR);  
            }
	        
	        
	        // addVar(&fileContents, &card->level, "rotation", VAR_INT);
	        
	        sprintf(buffer, "}\n\n");
	        addElementInifinteAllocWithCount_(&fileContents, buffer, strlen(buffer));
	        

	        // char entityFileName[512];
	        // sprintf(entityFileName, "entity_%d.txt", e->T.id);
	        	
	        char *fullEntityFileName = getFullNameForEntityFile(e->T.id, fullSceneFolderPath);

	        ///////////////////////************ Write the file to disk *************////////////////////
	        game_file_handle handle = platformBeginFileWrite(fullEntityFileName);
	        if(!handle.HasErrors) {
	        	platformWriteFile(&handle, fileContents.memory, fileContents.count*fileContents.sizeOfMember, 0);
	        } else {
				easyConsole_addToStream(DEBUG_globalEasyConsole, "Can't save entity file. Handle has Errors.");	        	
	        }

	        platformEndFile(handle);	
	        

            ////// empty array out
	        fileContents.count = 0;

	    }
	}

    releaseInfiniteAlloc(&aiNodeData);

    ////////save the tile map
    char buffer[32];
    sprintf(buffer, "{\n\n");
    addElementInifinteAllocWithCount_(&fileContents, buffer, strlen(buffer));
                    
    addVar(&fileContents, MyEntity_EntityTypeStrings[(int)ENTITY_TILE_MAP], "type", VAR_CHAR_STAR);

    for(int i = 0; i < gameState->tileSheet.tileCount; ++i) {
        WorldTile *t = gameState->tileSheet.tiles + i;

        addVar(&fileContents, MyTiles_TileTypeStrings[(int)t->type], "tileType", VAR_CHAR_STAR);

        addVar(&fileContents, MyTile_TileRotationTypeStrings[(int)t->rotation], "tileRotationType", VAR_CHAR_STAR);

        addVar(&fileContents, &t->yAxis, "tileYAxis", VAR_FLOAT);
        

        V3 pos = v3(t->x, t->y, t->z);
        addVar(&fileContents, &pos, "tilePos", VAR_V3);

    }

    sprintf(buffer, "}\n\n");
    addElementInifinteAllocWithCount_(&fileContents, buffer, strlen(buffer));
                

    char *fullEntityFileName = concatInArena(fullSceneFolderPath, "tile_map.txt", &globalPerFrameArena);

    ///////////////////////************ Write the file to disk *************////////////////////
    game_file_handle handle = platformBeginFileWrite(fullEntityFileName);
    if(!handle.HasErrors) {
        platformWriteFile(&handle, fileContents.memory, fileContents.count*fileContents.sizeOfMember, 0);
    } else {
        easyConsole_addToStream(DEBUG_globalEasyConsole, "Can't save entity file. Handle has Errors.");             
    }

    platformEndFile(handle);    


    ////////////////////////`   


    ///////////////////////************* Clean up the memory ************////////////////////   
    releaseInfiniteAlloc(&fileContents);
}

static bool gameScene_doesSceneExist(char *sceneName_) {
	DEBUG_TIME_BLOCK()

	char *sceneName = sceneName_;
	if(sceneName_[easyString_getSizeInBytes_utf8(sceneName) - 1] != '/' || sceneName_[easyString_getSizeInBytes_utf8(sceneName) - 1] != '\\') {
		sceneName = concatInArena(sceneName_, "/", &globalPerFrameArena);
	}

	char *allScenesFolderName = getAllScenesFolderName();
	char *fullSceneFolderPath = getFullSceneFolderPath(sceneName, allScenesFolderName);

	if(!platformDoesDirectoryExist(allScenesFolderName)) {
		//Create the scene folder  
		platformCreateDirectory(allScenesFolderName);	
		easyConsole_addToStream(DEBUG_globalEasyConsole, "Scene folder created");
	}

	//Check that this level exists
	bool result = platformDoesDirectoryExist(fullSceneFolderPath);

	return result;
}

static void gameScene_loadScene(GameState *gameState, EntityManager *manager, char *sceneName_, GameWeatherState *gameWeather) {
	DEBUG_TIME_BLOCK()

	gameState->currentSceneName = sceneName_;

    gameState->currentTerrainEntity = 0;
    gameState->tileSheet.tileCount = 0;

	char *sceneName = sceneName_;
	if(sceneName_[easyString_getSizeInBytes_utf8(sceneName) - 1] != '/' || sceneName_[easyString_getSizeInBytes_utf8(sceneName) - 1] != '\\') {
		sceneName = concatInArena(sceneName_, "/", &globalPerFrameArena);
	}

	easyConsole_addToStream(DEBUG_globalEasyConsole, "Loading Scene");

	char *allScenesFolderName = getAllScenesFolderName();
	char *fullSceneFolderPath = getFullSceneFolderPath(sceneName, allScenesFolderName);


	if(!platformDoesDirectoryExist(allScenesFolderName)) {
		//Create the scene folder  
		platformCreateDirectory(allScenesFolderName);	
		easyConsole_addToStream(DEBUG_globalEasyConsole, "Scene folder created");
	}

    //NOTE: update background sounds
    if(gameState->currentBackgroundSound) {
        easySound_endSound(gameState->currentBackgroundSound);
        gameState->currentBackgroundSound = 0;
    }

    if(easyString_stringsMatch_nullTerminated("castle", gameState->currentSceneName)) {
        gameState->currentBackgroundSound = playGameSound(&globalLongTermArena, easyAudio_findSound("Castle.wav"), 0, AUDIO_BACKGROUND);
        EasySound_LoopSound(gameState->currentBackgroundSound);

        gameWeather->timeOfDay = 0;
        gameWeather->timeOfDaySpeed = 0;

    } else {
        gameState->currentBackgroundSound = playGameSound(&globalLongTermArena, easyAudio_findSound("dark_forest.wav"), 0, AUDIO_BACKGROUND);
        EasySound_LoopSound(gameState->currentBackgroundSound);
        
        gameWeather->timeOfDay = 0.5f;
        gameWeather->timeOfDaySpeed = DEFAULT_DAY_NIGHT_SPEED;
    }

    //////////////////////////////////////////////////////////////

	//Check that this level exists
	if(platformDoesDirectoryExist(fullSceneFolderPath)) {
        //clear per load level arena 
        releaseMemoryMark(&perSceneArenaMark);
        perSceneArenaMark = takeMemoryMark(&globalPerSceneArena);

        Entity *wizard = 0;

        int maxId = 0;
		char *fileType[] = {"txt"};
        FileNameOfType files = getDirectoryFilesOfType(fullSceneFolderPath, fileType, arrayCount(fileType));
        int splatCount = files.count;
        for(int i = 0; i < files.count; ++i) {
            char *fullName = files.names[i];
            char *shortName = getFileLastPortionWithArena(fullName, &globalPerFrameArena);
            if(shortName[0] != '.') { //don't load hidden file 
    			bool parsing = true;
        		FileContents contents = getFileContentsNullTerminate(fullName);
        		EasyTokenizer tokenizer = lexBeginParsing((char *)contents.memory, EASY_LEX_OPTION_EAT_WHITE_SPACE);

        		V3 position = v3(0, 0, 0);
        		Quaternion rotation = identityQuaternion();
        		V3 scale = v3(1, 1, 1);
        		V4 color = v4(1, 1, 1, 1);
        		EntityType entType = ENTITY_NULL;
        		float layer = 0;
        		
        		bool foundId = false;
        		int id = 0;

                EntityEnemyType enemyType = ENEMY_SKELETON;


        		bool colliderSet = false;
        		bool colliderSet2 = false;
        		V2 colliderScale = v2(1, 1);
        		V2 colliderScale2 = v2(1, 1);

                V3 colliderOffset = v3(0, 0, 0);
                V3 colliderOffset2 = v3(0, 0, 0);

        		Texture *splatTexture = 0;

        		SubEntityType subtype = ENTITY_SUB_TYPE_NONE;

        		char *audioFile = 0;

                Animation *animation = 0;

                V3 lightColor = v3(1, 0.5f, 0);
                float lightIntensity = 2.0f;

                bool shouldNotRender = false;

                EntityTriggerType triggerType = ENTITY_TRIGGER_NULL;

                ChestType chestType = CHEST_TYPE_HEALTH_POTION;

                EntityLocationSoundType locationSoundType = ENTITY_SOUND_NULL;


                EasyModel *model = 0;

                EntityType entityToCreate;
                float rateOfCreation;
                float enemyMoveSpeed = 1;

                EasyAiController *aiController = 0;

                char *sceneToLoad = 0;

                bool renderFirstPass = false;

        		// s32 teleporterIds[256];
        		// Entity *teleportEnts[256];
        		// u32 idCount = 0;
        		// bool wasTeleporter = false;

                DialogInfoType dialogType = ENTITY_DIALOG_NULL;

        		while(parsing) {
            		EasyToken token = lexGetNextToken(&tokenizer);
            		InfiniteAlloc *data = 0;
            		switch(token.type) {
                		case TOKEN_NULL_TERMINATOR: {

                    		parsing = false;
                		} break;
                		case TOKEN_WORD: {
                			// if(stringsMatchNullN("tag", token.at, token.size)) {
                			// 	if(gameState->currentRoomTagCount < arrayCount(gameState->currentRoomTags)) {
                			// 			char *tagString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                			// 			char *newString = easyString_copyToHeap(tagString, strlen(tagString));

                			// 			gameState->currentRoomTags[gameState->currentRoomTagCount++] = newString;

                			//     		////////////////////////////////////////////////////////////////////
                			//     		releaseInfiniteAlloc(data);	
                			// 	} else {
                			// 		easyConsole_addToStream(DEBUG_globalEasyConsole, "Tags full");
                			// 	}
    		           	    	
                			// }
                    		if(stringsMatchNullN("position", token.at, token.size)) {
                        		position = buildV3FromDataObjects(&tokenizer);
                    		}
                    		if(stringsMatchNullN("id", token.at, token.size)) {
                        		id = getIntFromDataObjects(&tokenizer);
                        		foundId = true;
                    		}
    						if(stringsMatchNullN("layer", token.at, token.size)) {
    				    		layer = getFloatFromDataObjects(&tokenizer);
    						}
    						if(stringsMatchNullN("subtype", token.at, token.size)) {
    				    		subtype = (SubEntityType)getIntFromDataObjects(&tokenizer);
    						}


                            //These are arrays
                            if(stringsMatchNullN("aiNodes", token.at, token.size)) {
                                if(!aiController) {
                                    aiController = easyAi_initController(&globalPerSceneArena);
                                }

                                InfiniteAlloc *dataObjs = getDataObjects(&tokenizer);
                                int indexAt = 0;
                                while(indexAt < dataObjs->count) {

                                    V3 pAt = makeV3FromDataObjects_notClearObjects(&tokenizer, indexAt);
                                    indexAt += 3;

                                    bool nodeType = (bool)getIntFromDataObjects_notClearObjects(&tokenizer, indexAt);
                                    indexAt += 1;

                                    easyAi_pushNode(aiController, pAt, aiController->boardHash, nodeType); 
                                    

                                    assert((dataObjs->count - indexAt) % 4 == 0);
                                }

                                dataObjs->count = 0;
                                
                            }

                            //These are arrays
                            if(stringsMatchNullN("searchBouys", token.at, token.size)) {
                                if(!aiController) {
                                    aiController = easyAi_initController(&globalPerSceneArena);
                                }

                                InfiniteAlloc *dataObjs = getDataObjects(&tokenizer);
                                int indexAt = 0;
                                while(indexAt < dataObjs->count) {

                                    V3 pAt = makeV3FromDataObjects_notClearObjects(&tokenizer, indexAt);
                                    indexAt += 3;

                                    easyAi_pushSearchBouy(aiController, pAt);  
                                    
                                    assert((dataObjs->count - indexAt) % 3 == 0);
                                }

                                dataObjs->count = 0;
                                
                            }


                            

                            if(stringsMatchNullN("levelToLoad", token.at, token.size)) {
                                char *loadString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                sceneToLoad = easyString_copyToHeap(loadString);
                            }


                            if(stringsMatchNullN("shouldNotRender", token.at, token.size)) {
                                shouldNotRender = getBoolFromDataObjects(&tokenizer);
                            }


                            if(stringsMatchNullN("rateOfCreation", token.at, token.size)) {
                                rateOfCreation = getFloatFromDataObjects(&tokenizer);
                            }

                            if(stringsMatchNullN("enemyMoveSpeed", token.at, token.size)) {
                                enemyMoveSpeed = getFloatFromDataObjects(&tokenizer);
                            }

                            if(stringsMatchNullN("typeToCreate", token.at, token.size)) {
                                char *typeToCreate = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);


                                entityToCreate = (EntityType)findEnumValue(typeToCreate, MyEntity_EntityTypeStrings, arrayCount(MyEntity_EntityTypeStrings));
                            }


                            if(stringsMatchNullN("renderFirstPass", token.at, token.size)) {
                                renderFirstPass = getIntFromDataObjects(&tokenizer);
                            }


                            
                    		// if(stringsMatchNullN("teleporterId", token.at, token.size)) {
                    		// 	assert(idCount < arrayCount(teleporterIds));
                      //   		teleporterIds[idCount++] = getIntFromDataObjects(&tokenizer);
                      //   		wasTeleporter = true;
                    		// }
    		                
                    		if(stringsMatchNullN("scale", token.at, token.size)) {
                        		scale = buildV3FromDataObjects(&tokenizer);
                    		}
            				if(stringsMatchNullN("colliderScale", token.at, token.size)) {
            					colliderSet = true;
            		    		colliderScale = buildV2FromDataObjects(&tokenizer);
            				}
    						if(stringsMatchNullN("colliderScale2", token.at, token.size)) {
    							colliderSet2 = true;
    				    		colliderScale2 = buildV2FromDataObjects(&tokenizer);
    						}
                            if(stringsMatchNullN("colliderOffset", token.at, token.size)) {
                                colliderOffset = buildV3FromDataObjects(&tokenizer);
                            }
                            if(stringsMatchNullN("colliderOffset2", token.at, token.size)) {
                                colliderOffset2 = buildV3FromDataObjects(&tokenizer);
                            }   

                            if(stringsMatchNullN("enemyType", token.at, token.size)) {
                                char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                enemyType = (EntityEnemyType)findEnumValue(typeString, MyEntity_EnemyTypeStrings, arrayCount(MyEntity_EnemyTypeStrings));

                            }

                    		if(stringsMatchNullN("type", token.at, token.size)) {
                    			char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                        		entType = (EntityType)findEnumValue(typeString, MyEntity_EntityTypeStrings, arrayCount(MyEntity_EntityTypeStrings));

                    		}

                            if(stringsMatchNullN("triggerType", token.at, token.size)) {
                                char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                triggerType = (EntityTriggerType)findEnumValue(typeString, MyEntity_TriggerTypeStrings, arrayCount(MyEntity_TriggerTypeStrings));

                            }

                            if(stringsMatchNullN("chestType", token.at, token.size)) {
                                char *chestTypeStr = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                chestType = (ChestType)findEnumValue(chestTypeStr, MyEntity_ChestTypeStrings, arrayCount(MyEntity_ChestTypeStrings));

                            }

                            if(stringsMatchNullN("locationSoundType", token.at, token.size)) {
                                char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                locationSoundType = (EntityLocationSoundType)findEnumValue(typeString, MyEntity_LocationSoundTypeStrings, arrayCount(MyEntity_LocationSoundTypeStrings));

                            }

                            

                            if(stringsMatchNullN("dialogType", token.at, token.size)) {
                                char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                dialogType = (DialogInfoType)findEnumValue(typeString, MyDialog_DialogTypeStrings, arrayCount(MyDialog_DialogTypeStrings));

                                if(dialogType == ENTITY_DIALOG_WELCOME_SIGN) {
                                    int j = 0;
                                }

                            }

                            if(stringsMatchNullN("model", token.at, token.size)) {
                                char *modelString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                model = findModelAsset_Safe(modelString);

                                if(!model) {
                                    //Default to a pink texture so it doesnt crash 
                                    //COuld change to a more bogus model 
                                    splatTexture = &globalPinkTexture;
                                }

                            }

                            
                    		if(stringsMatchNullN("rotation", token.at, token.size)) {
                        		V4 rot = buildV4FromDataObjects(&tokenizer);
                        		rotation.E[0] = rot.x;
                        		rotation.E[1] = rot.y;
                        		rotation.E[2] = rot.z;
                        		rotation.E[3] = rot.w;
                    		}
                    		if(stringsMatchNullN("color", token.at, token.size)) {
                        		color = buildV4FromDataObjects(&tokenizer);
                    		}
                    		if(stringsMatchNullN("audioFileName", token.at, token.size)) {
                        		char *name = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);
                        		 audioFile = easyString_copyToHeap(name);

                    		}

                            if(stringsMatchNullN("lightColor", token.at, token.size)) {
                                lightColor = buildV3FromDataObjects(&tokenizer);
                            }

                            if(stringsMatchNullN("lightIntensity", token.at, token.size)) {
                                lightIntensity = getFloatFromDataObjects(&tokenizer);
                            }


                            if(stringsMatchNullN("animationName", token.at, token.size)) {
                                char *name = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);
                                animation = gameState_findSplatAnimation(gameState, name); 

                            }

            				if(stringsMatchNullN("spriteName", token.at, token.size)) {
            		    		char *name = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);
            		    		if(easyString_stringsMatch_nullTerminated(name, "white texture") ) {
            		    			splatTexture = &globalWhiteTexture;
            		    		} else {
            		    			splatTexture = gameState_findSplatTexture(gameState, name);	
            		    		}
            		    		

            				}
                		} break;
                		case TOKEN_CLOSE_BRACKET: {


                		} break;
                		case TOKEN_OPEN_BRACKET: {
    		                //do nothing
                		} break;
                		default: {

                		}
            		}
        		}


                

                if(entType == ENTITY_TILE_MAP) {
                    parsing = true;
                    tokenizer = lexBeginParsing((char *)contents.memory, EASY_LEX_OPTION_EAT_WHITE_SPACE);
                    WorldTileType tileType; 
                    WorldTileRotation rotationType = WORLD_TILE_ROTATION_FLAT;
                    float tileYAxis = 0;

                    while(parsing) {
                        EasyToken token = lexGetNextToken(&tokenizer);
                        InfiniteAlloc *data = 0;
                        bool ranOutOfTileRooms = false;
                        

                        switch(token.type) {
                            case TOKEN_NULL_TERMINATOR: {

                                parsing = false;
                            } break;
                            case TOKEN_WORD: {
                                if(stringsMatchNullN("tilePos", token.at, token.size) && !ranOutOfTileRooms) {
                                    // V2 p = buildV2FromDataObjects(&tokenizer);
                                    V3 p = buildV3FromDataObjects(&tokenizer);

                                    ranOutOfTileRooms = !addWorldTile(gameState, p.x, p.y, p.z, tileYAxis, tileType, rotationType);
                                    if(ranOutOfTileRooms) { easyFlashText_addText(&globalFlashTextManager, "Tile Array Full. Make bigger!"); }
                                }

                                if(stringsMatchNullN("tileType", token.at, token.size)) {
                                    char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);
                                    tileType = (WorldTileType)findEnumValue(typeString, MyTiles_TileTypeStrings, arrayCount(MyTiles_TileTypeStrings));
                                }

                                if(stringsMatchNullN("tileRotationType", token.at, token.size)) {
                                    char *tileRotationTypeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);
                                    rotationType = (WorldTileRotation)findEnumValue(tileRotationTypeString, MyTile_TileRotationTypeStrings, arrayCount(MyTile_TileRotationTypeStrings));
                                }

                                if(stringsMatchNullN("tileYAxis", token.at, token.size)) {
                                    tileYAxis = getFloatFromDataObjects(&tokenizer);
                                }

                            } break;
                            default: {
                            }
                        }
                    }
                } else {    

        			//Make the entity
        			Entity *newEntity = initEntityOfType(gameState, manager, position, splatTexture, entType, subtype, colliderSet, triggerType, audioFile, animation);

                    newEntity->chestType = chestType;

                    newEntity->T.pos = position;

                    releaseInfiniteAlloc(&tokenizer.typesArray);

        			assert(newEntity);

        			//NOTE(ollie): Set the id
        			if(foundId) {
            			newEntity->T.id = id;
            			if(id > maxId) {
            				maxId = id;
            			}
        			}
        			foundId = false;

                    if(sceneToLoad) {
                        newEntity->levelToLoad = sceneToLoad;
                    }

        			if(newEntity->collider) {
        				newEntity->collider->dim2f = colliderScale;	
                        newEntity->collider->offset = colliderOffset;
        			}


                    if(entType == ENTITY_WIZARD) {
                        wizard = newEntity;
                    }


        			if(newEntity->collider1) {
        				newEntity->collider1->dim2f = colliderScale2;	
                        newEntity->collider1->offset = colliderOffset2;
        			}

                    if(newEntity->type == ENTITY_ENEMY) {
                        newEntity->enemyMoveSpeed = enemyMoveSpeed;
                    }

                    newEntity->rateOfCreation = rateOfCreation;
                    newEntity->typeToCreate = entityToCreate;

                    newEntity->subEntityType = subtype;

        			newEntity->layer = layer;
                    newEntity->renderFirstPass = renderFirstPass;

                    newEntity->aiController = aiController;
                    
                    newEntity->dialogType = dialogType;
                    
                    if(newEntity->type == ENTITY_TERRAIN) {
                        gameState->currentTerrainEntity = newEntity;
                        newEntity->T.Q = identityQuaternion();
            		//   
            		} else {
                         newEntity->T.Q = rotation;
                        // newEntity->T.Q = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);//rotation;
                    }

                    if(animation) {
                        easyAnimation_addAnimationToController(&newEntity->animationController, &gameState->animationFreeList, animation, EASY_ANIMATION_PERIOD);  
                    }
                   
                    newEntity->locationSoundType = locationSoundType;
                    newEntity->T.scale = scale;
            		newEntity->colorTint = color;

                    newEntity->enemyType = enemyType;

                    if(model) {
                        newEntity->model = model;
                    }


                    if(audioFile) {
                        newEntity->audioFile = audioFile;
                    }

                    u32 shouldNotRenderFlag = (shouldNotRender) ? ENTITY_SHOULD_NOT_RENDER : 0;
                    newEntity->flags |= shouldNotRenderFlag;

                    newEntity->lightColor = lightColor;
                    newEntity->lightIntensity = lightIntensity;

                }
                
                /////////////////////////////////
        		//reset the values
        		position = v3(0, 0, 0);
        		rotation = identityQuaternion();
        		scale = v3(1, 1, 1);
        		color = v4(1, 1, 1, 1);
        		entType = ENTITY_NULL;

                enemyMoveSpeed = 1;
                lightColor = v3(0, 0, 0);
                lightIntensity = 0;

                enemyType = ENEMY_SKELETON; 


        		colliderSet = false;
        		colliderSet2 = false;
                shouldNotRender = false;

        		splatTexture = 0;
                animation = 0; 
        		audioFile = 0;
                aiController = 0;
                chestType = CHEST_TYPE_HEALTH_POTION;

        		subtype = ENTITY_SUB_TYPE_NONE;
        		// if(wasTeleporter) {
        		// 	teleportEnts[idCount - 1] = newEntity;
        		// }

        		// wasTeleporter = false;



        		//////////////////////////////////////////////////////////////

        		easyFile_endFileContents(&contents);
        		
        	}
        	//Free the filename from the 'find all file types' function
        	free(fullName);
        }

        //Link any connected entities up
        // for(int i = 0; i < idCount; ++i) {
        // 	Entity * e = MyEntity_findEntityById_static(entityManager, teleporterIds[i]);
        // 	assert(e->type == ENTITY_TELEPORTER);
        // 	teleportEnts[i]->teleporterPartner = e;
        // }
        GLOBAL_transformID_static = maxId + 1;


        if(wizard) {
            //we make sure the wizard has an offset for the collider1 which it the hurt box when she uses her sword
            wizard->collider1->offset.y = 0.5f;
            wizard->collider1->dim2f = v2(1, 1.4f);    
        }
        
    }
	

}	