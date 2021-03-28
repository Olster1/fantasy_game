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

static void gameScene_saveScene(EntityManager *manager, char *sceneName_) {
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
	

	for(int i = 0; i < manager->entities.count; ++i) {
	    Entity *e = (Entity *)getElement(&manager->entities, i);
	    if(e && (e->flags & ENTITY_SHOULD_SAVE)) {
	        
	    	InfiniteAlloc fileContents = initInfinteAlloc(char);

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

	        if(e->sprite) {
	        	addVar(&fileContents, e->sprite->name, "spriteName", VAR_CHAR_STAR);
	        }
			
			if(e->collider) {
				addVar(&fileContents, &e->collider->dim2f, "colliderScale", VAR_V2);
                addVar(&fileContents, &e->collider->offset, "colliderOffset", VAR_V3);	
			}	        


	        if(e->collider1) {
	        	addVar(&fileContents, &e->collider1->dim2f, "colliderScale2", VAR_V2);
                addVar(&fileContents, &e->collider1->offset, "colliderOffset2", VAR_V3);	
	        }	

	        if(e->audioFile) {
	        	addVar(&fileContents, e->audioFile, "audioFileName", VAR_CHAR_STAR);	
	        } 


            addVar(&fileContents, &e->renderFirstPass, "renderFirstPass", VAR_INT);    


            addVar(&fileContents, MyDialog_DialogTypeStrings[(int)e->dialogType], "dialogType", VAR_CHAR_STAR);

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
	        
	        
	        ///////////////////////************* Clean up the memory ************////////////////////	
	        releaseInfiniteAlloc(&fileContents);

	    }
	}
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

static void gameScene_loadScene(GameState *gameState, EntityManager *manager, char *sceneName_) {
	DEBUG_TIME_BLOCK()

	gameState->currentSceneName = sceneName_;

    gameState->currentTerrainEntity = 0;

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

	//Check that this level exists
	if(platformDoesDirectoryExist(fullSceneFolderPath)) {

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


        		bool colliderSet = false;
        		bool colliderSet2 = false;
        		V2 colliderScale = v2(1, 1);
        		V2 colliderScale2 = v2(1, 1);

                V3 colliderOffset = v3(0, 0, 0);
                V3 colliderOffset2 = v3(0, 0, 0);

        		Texture *splatTexture = 0;

        		SubEntityType subtype = ENTITY_SUB_TYPE_NONE;

        		char *audioFile = 0;


                EasyModel *model = 0;

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


                    		if(stringsMatchNullN("type", token.at, token.size)) {
                    			char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                        		entType = (EntityType)findEnumValue(typeString, MyEntity_EntityTypeStrings, arrayCount(MyEntity_EntityTypeStrings));

                    		}

                            if(stringsMatchNullN("dialogType", token.at, token.size)) {
                                char *typeString = getStringFromDataObjects_lifeSpanOfFrame(&tokenizer);

                                dialogType = (DialogInfoType)findEnumValue(typeString, MyDialog_DialogTypeStrings, arrayCount(MyDialog_DialogTypeStrings));

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

                releaseInfiniteAlloc(&tokenizer.typesArray);

    			//Make the entity
    			Entity *newEntity = 0;

    			switch(entType) {
    				case ENTITY_SCENERY: {
    					if(subtype & ENTITY_SUB_TYPE_TORCH) {
    						newEntity = initTorch(gameState, manager, position);
    					} else if(subtype & ENTITY_SUB_TYPE_ONE_WAY_UP_PLATFORM) {
                            newEntity = initOneWayPlatform(gameState, manager, position, splatTexture);
                        } else {
    						if(colliderSet) {
    							newEntity = initScenery_withRigidBody(gameState, manager, position, splatTexture);
    						} else {
    							newEntity = initScenery_noRigidBody(gameState, manager, position, splatTexture);	
    						}	
    					}
    					
    				} break;
    				case ENTITY_WIZARD: {
    					newEntity = initWizard(gameState, manager, position);
    					manager->player = newEntity;

    				} break;
    				case ENTITY_PLAYER_PROJECTILE: {
    					assert(false);
    				} break;
    				case ENTITY_SKELETON: {
    					newEntity = initSkeleton(gameState, manager, position);
    				} break;
                    case ENTITY_SWORD: {
                        newEntity = initSword(gameState, manager, position);
                    } break;
                    case ENTITY_CHEST: {
                        newEntity = initChest(gameState, manager, position);
                    } break;
                    case ENTITY_HORSE: {
                        newEntity = initHorse(gameState, manager, position);
                    } break;
                    case ENTITY_BLOCK_TO_PUSH: {
                        newEntity = initPushRock(gameState, manager, position);
                    } break;
                    case ENTITY_SHEILD: {
                        newEntity = initSheild(gameState, manager, position);
                    } break;
                    case ENTITY_SIGN: {
                        newEntity = initSign(gameState, manager, position, splatTexture);
                    } break;
                    case ENTITY_LAMP_POST: {
                        newEntity = initLampPost(gameState, manager, position, splatTexture);
                    } break;
                    case ENTITY_HOUSE: {
                        newEntity = initHouse(gameState, manager, position, splatTexture);
                    } break;
                    case ENTITY_WEREWOLF: {
                        newEntity = initWerewolf(gameState, manager, position);
                    } break;
    				case ENTITY_HEALTH_POTION_1: {
    					assert(false);
    				} break;
    				case ENITY_AUDIO_CHECKPOINT: {
    					newEntity = initAudioCheckPoint(gameState, manager, position);
    					newEntity->audioFile = audioFile;
    				} break;
    				case ENITY_CHECKPOINT: {
    					newEntity = initCheckPoint(gameState, manager, position);
    				} break;
                    case ENTITY_TERRAIN: {
                        newEntity = initTerrain(gameState, manager, position);
                    } break;
    				default: {

    				}
    			}


    			assert(newEntity);

    			//NOTE(ollie): Set the id
    			if(foundId) {
        			newEntity->T.id = id;
        			if(id > maxId) {
        				maxId = id;
        			}
    			}
    			foundId = false;



    			if(newEntity->collider) {
    				newEntity->collider->dim2f = colliderScale;	
                    newEntity->collider->offset = colliderOffset;
    			}


    			if(newEntity->collider1) {
    				newEntity->collider1->dim2f = colliderScale2;	
                    newEntity->collider1->offset = colliderOffset2;
    			}

    			newEntity->layer = layer;
                newEntity->renderFirstPass = renderFirstPass;
                
                newEntity->dialogType = dialogType;
                
                if(newEntity->type == ENTITY_TERRAIN) {
                    gameState->currentTerrainEntity = newEntity;
                    newEntity->T.Q = identityQuaternion();
        		//   
        		} else {
                     newEntity->T.Q = rotation;
                    // newEntity->T.Q = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);//rotation;
                }
               
                
                newEntity->T.scale = scale;
        		newEntity->colorTint = color;

                if(model) {
                    newEntity->model = model;
                }


                if(audioFile) {
                    newEntity->audioFile = audioFile;
                }

                
        		//reset the values
        		position = v3(0, 0, 0);
        		rotation = identityQuaternion();
        		scale = v3(1, 1, 1);
        		color = v4(1, 1, 1, 1);
        		entType = ENTITY_NULL;

        		colliderSet = false;
        		colliderSet2 = false;

        		splatTexture = 0;

        		audioFile = 0;

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
    }
	

}	