static void initCrafting(GameState *state) {

	Game_Crafting *shop = pushStruct(&globalLongTermArena, Game_Crafting);

	for(int i = 0; i < arrayCount(shop->animationItemTimers); ++i) {
		shop->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
		shop->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
	}

    shop->usingCauldron = false;

	state->crafting = shop;

    shop->uiLevel = 0;


}

// static void addItemToShop(Game_Shop *shop, EntityType type,int count, bool isDisposable, float cost) {
// 	assert(shop->itemCount < MAX_SHOP_ITEM_COUNT);

// 	ItemInfo newItem = {};

// 	newItem.type = type;
// 	newItem.count = count;
// 	newItem.isDisposable = isDisposable;
// 	newItem.cost = cost;
// 	newItem.maxCount = count;

// 	shop->items[shop->itemCount++] = newItem;
// }

static void enterGameCrafting(GameState *gameState) {
    Game_Crafting *shop = gameState->crafting;
	shop->inventoryBreathSelector = 0;

	if(gameState->gameIsPaused) { gameState->gameIsPaused = false; }

	shop->itemIndex = 0;
	shop->animationItemTimers[0].target = UI_ITEM_PICKER_MAX_SIZE;

	//Stop all the other animation timers
	for(int i = 1; i < arrayCount(shop->animationItemTimers); ++i) {
	    shop->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
	    shop->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
	}

	gameState->gameModeType = GAME_MODE_CRAFTING;
	gameState->gameIsPaused = true;

    shop->usingCauldron = false;

    

}

static void updateCrafting(Game_Crafting *shop, float dt) {
	shop->timeSinceLastRefill += dt;

	if(shop->timeSinceLastRefill > 60*3) { //3 minutes
		for(int i = 0; i < shop->itemCount; ++i) {
			shop->items[i].count = shop->items[i].maxCount;
		}
	}
}

static void updateCrafting(Game_Crafting *shop, GameState *gameState, Entity *player, EasyFont_Font *gameFont, OSAppInfo *appInfo) {

    //NOTE: Get the craftable inventory count
    int inventoryCount = 0;

    ItemInfo *infos[arrayCount(gameState->itemSpots)];

    //////// NOTE: Get the inventory count first /////////////////////////
    for(int i = 1; i < arrayCount(gameState->itemSpots) + 1; ++i) {   

        if(gameState->itemSpots[i].isCraftable) {
            infos[inventoryCount++] = &gameState->itemSpots[i];
        }
    }
    /////////////


    if(shop->uiLevel == 0) {
    	if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT) && shop->itemIndex  != 0) {
            shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MIN_SIZE;


            shop->itemIndex--;
            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);

            easyConsole_pushInt(DEBUG_globalEasyConsole, shop->itemIndex);
            shop->inventoryBreathSelector = 0;

            if(shop->itemIndex < 0) {
                shop->itemIndex = 0;
            }

            shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MAX_SIZE;
        }

        
        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_RIGHT) && shop->itemIndex  < (inventoryCount - 1)) {
            shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MIN_SIZE;

            shop->inventoryBreathSelector = 0;


            shop->itemIndex++;
            easyConsole_pushInt(DEBUG_globalEasyConsole, shop->itemIndex);
            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
            if(shop->itemIndex >= inventoryCount) {
                shop->itemIndex = inventoryCount - 1;

            }

            shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MAX_SIZE;
        }

        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_UP)) {
            shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MIN_SIZE;

            shop->inventoryBreathSelector = 0;

            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);

            shop->uiLevel = 1;
        }
    } else {
        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_DOWN)) {
            shop->inventoryBreathSelector = 0;

            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);

            shop->uiLevel = 0;
        }
    } 





    /////////////////// DRAW THE SHOP ///////////////////////////////////

    EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);
  
    gameState->gameIsPaused = true;

    setViewTransform(globalRenderGroup, mat4());

    float fuaxWidth = 1920.0f;
    float fuaxHeight = appInfo->aspectRatio_yOverX*fuaxWidth;
    
    setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

    V2 size = getBounds("ALCHEMY", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 2, gameState->fuaxResolution, 1);
    // outputTextNoBacking(gameFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 100, 0.1f, gameState->fuaxResolution, "ALCHEMY", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 2, true, 1);
    

    //DRAW BACKGROUND

    EasyTransform T;
    easyTransform_initTransform_withScale(&T, v3(0, 0, 0.4f), v3(fuaxWidth, fuaxHeight, 1), EASY_TRANSFORM_NO_ID); 

    setModelTransform(globalRenderGroup, easyTransform_getTransform(&T));
   
    renderSetShader(globalRenderGroup, &textureProgram);
    renderDrawSprite(globalRenderGroup, findTextureAsset("quilll_cropped.png"), COLOR_WHITE);

    /////////////////////////// DRAW THE CAULDRON ///////////////////////////////////

    {
        Texture *cauldronTexture = 0;

        if(gameState->cauldronAnimationController.currentLoopCount > 3  && gameState->cauldronAnimationController.finishedAnimationLastUpdate) {
            easyAnimation_emptyAnimationContoller(&gameState->cauldronAnimationController, &gameState->animationFreeList);
            easyParticles_pauseSystem(&gameState->steamParticleSystem);
            gameState->crafting->usingCauldron = false;
        }

        if(!easyAnimation_isControllerEmpty(&gameState->cauldronAnimationController)) {
            char *animationFileName = easyAnimation_updateAnimation(&gameState->cauldronAnimationController, &gameState->animationFreeList, appInfo->dt);
            cauldronTexture = findTextureAsset(animationFileName);
        } else {
            cauldronTexture = findTextureAsset("cauldron_long.png"); 
        }
        

        // Texture *cauldronTexture = findTextureAsset("cauldron.png");
        renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
        float cauldronScale = 300;
        Matrix4 T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(cauldronScale, cauldronTexture->aspectRatio_h_over_w*cauldronScale, 0)), v3(0, 200, 0.3f));
                            
        setModelTransform(globalRenderGroup, T_matrix);
        renderDrawSprite(globalRenderGroup, cauldronTexture, COLOR_WHITE);

        Texture *brewTexture = findTextureAsset("brew_writing.png"); 

        if(gameState->crafting->uiLevel == 1) {
            brewTexture =  findTextureAsset("brew_writing_lite.png");            
        }

        T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(cauldronScale, brewTexture->aspectRatio_h_over_w*cauldronScale, 0)), v3(0, 300, 0.2f));
                            
        setModelTransform(globalRenderGroup, T_matrix);
        renderDrawSprite(globalRenderGroup, brewTexture, COLOR_WHITE);

        {
            // Texture *hover_t = findTextureAsset("targeting2.png");
                
            // float circleSize = 150;
    
            // float s = smoothStep00(1.3f*circleSize, shop->inventoryBreathSelector, 1.6f*circleSize);

            // Matrix4 T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(1.5f*s, s, 0)), v3(0, 300, 0.1f));
                                
            // setModelTransform(globalRenderGroup, T_matrix);
            // renderDrawSprite(globalRenderGroup, hover_t, COLOR_WHITE);
        }
        


        renderSetShader(globalRenderGroup, &textureProgram);
        V3 worldP = v3_plus(v3(0, 100, 0), v3(0, 0, 0.2f));
        drawAndUpdateParticleSystem(globalRenderGroup, &gameState->steamParticleSystem, worldP, appInfo->dt, v3(0, 0, 0), COLOR_WHITE, true);
        ///////////////////////////////////////////

    }

    /////////////////////////// DRAW THE ITEMS ///////////////////////////////////


    Texture *t = findTextureAsset("inventory_spot.png");
    Texture *hover_t = findTextureAsset("targeting2.png");
   
    float spacing = 300;
    float circleSize = 150;
    
    float xAt = -0.5f*spacing*(inventoryCount - 1);
    if(inventoryCount == 1) {
        xAt = 0;
    }
    float yAt = -200;

    for(int i = 1; i < inventoryCount + 1; ++i) {   


        Texture *t_todraw = t;

        ItemInfo *itemI = infos[i - 1];


        if(!itemI->isCraftable) {
            continue;
        }

        // The Item the player is hovering on
        if(shop->itemIndex == (i - 1) && shop->uiLevel == 0) {
            shop->inventoryBreathSelector += appInfo->dt;

            if(shop->inventoryBreathSelector > 1.0f) {
                shop->inventoryBreathSelector -= 1.0f;
            }

            float s = smoothStep00(1.3f*circleSize, shop->inventoryBreathSelector, 1.6f*circleSize);

            Matrix4 T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(s, s, 0)), v3(xAt, yAt, 0.3f));
                                
            setModelTransform(globalRenderGroup, T_matrix);
            renderDrawSprite(globalRenderGroup, hover_t, COLOR_WHITE);


            if(itemI->type != ENTITY_NULL) {
                char *s = getInventoryString(itemI->type);
                float descripX = 0.5f*gameState->fuaxResolution.x;
                V2 size = getBounds(s, rect2fMinMax(0, 0, 0.9f*gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 1.4f, gameState->fuaxResolution, 1);
                descripX -= 0.5f*size.x;
                outputTextNoBacking(gameFont, descripX, gameState->fuaxResolution.y - 200, 0.1f, gameState->fuaxResolution, s, rect2fMinMax(descripX, 0, 0.9f*gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(0, 0, 0, 1), 1.4f, true, 1);
            }
            

            // t_todraw = findTextureAsset("inventory_spot1.png");

            //ADD ITEM TO CAULDRON
            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_SPACE) && !gameState->crafting->usingCauldron && gameState->crafting->uiLevel == 0) {
                CraftRecipe *recipe = &gameState->crafting->currentRecipe;    
                if(recipe->typeCount < arrayCount(recipe->types)) {
                    
                    int index = -1;

                    bool same = false;

                    for(int i = 0; i < recipe->typeCount ; ++i) {
                        if(recipe->types[i] == itemI->type) {
                            index = i;
                            same = true;
                            break;
                        }
                    }

                    if(index < 0) {
                        for(int i = 0; i < arrayCount(recipe->types) ; ++i) {
                            if(recipe->types[i] == ENTITY_NULL) {
                                index = i;
                                break;
                            }
                        }
                    }

                    recipe->types[index] = itemI->type;
                    recipe->count[index] = recipe->count[index] + 1;

                    if(!same) {
                        recipe->typeCount++;
                    }
                        

                    playGameSound(&globalLongTermArena, easyAudio_findSound("ui_soft.wav"), 0, AUDIO_BACKGROUND);

                    itemI->count--;
                } else {
                    //NOTE: Show error message
                }
            }

            
        }

        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_SPACE) && !gameState->crafting->usingCauldron && gameState->crafting->uiLevel == 1) {
            playGameSound(&globalLongTermArena, easyAudio_findSound("cauldron.wav"), 0, AUDIO_BACKGROUND);
            easyAnimation_addAnimationToController(&gameState->cauldronAnimationController, &gameState->animationFreeList, gameState_findSplatAnimation(gameState, "cauldron_bubbling_9.png"), 0.25f*GLOBAL_DEFINE_defaultMetersPerLetter_floatingText);
            easyParticles_playSystem(&gameState->steamParticleSystem);
            gameState->crafting->usingCauldron = true;
        }

        //////////////////////

        /////////////////////////////////////////////////

        ///////////////// DRAW THE ITEM COUNT AND ITEM  ////////////////////////////
        float x = xAt;
        float y = yAt;
        Matrix4 T_matrix;
        // Matrix4 T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(circleSize, circleSize, 0)), v3(x, y, 0.4f));
        
        // setModelTransform(globalRenderGroup, T_matrix);
        // renderDrawSprite(globalRenderGroup, t_todraw, COLOR_WHITE);

       
        if(itemI->type != ENTITY_NULL) {

            renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
            Texture *inventoryTexture = getInvetoryTexture(itemI->type);


            T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(circleSize/inventoryTexture->aspectRatio_h_over_w, circleSize, 0)), v3(x, y, 0.2f));
            setModelTransform(globalRenderGroup, T_matrix);
            renderDrawSprite(globalRenderGroup, inventoryTexture, COLOR_WHITE);
            renderSetShader(globalRenderGroup, &glossProgram);

            if(itemI->isDisposable) {
                float offset = 50;
                char *str = easy_createString_printf(&globalPerFrameArena, "%d", itemI->count);
                outputTextNoBacking(gameFont, x + 0.5f*fuaxWidth + offset, 0.5f*fuaxHeight - y + offset, 0.1f, gameState->fuaxResolution, str, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_BLACK, 1.3f, true, 1);
            }
            
            
        }
        //////////////////////////////////////////////////////////////////////////

        xAt += spacing;

    
    }


    /////////////////////////// DRAW THE recipe ///////////////////////////////////

    {
        CraftRecipe *recipe = &gameState->crafting->currentRecipe;    

        float xAt = -0.45f*fuaxWidth;
        float yAt = 200;
        for(int i = 0; i < recipe->typeCount; ++i) {

            Texture *inventoryTexture = getInvetoryTexture(recipe->types[i]);

            renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
            float scale = 100;
            Matrix4 T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(scale, inventoryTexture->aspectRatio_h_over_w*scale, 0)), v3(xAt, yAt, 0.3f));
                                
            setModelTransform(globalRenderGroup, T_matrix);
            renderDrawSprite(globalRenderGroup, inventoryTexture, COLOR_WHITE);

            float offset = 50;
            char *str = easy_createString_printf(&globalPerFrameArena, "%d", recipe->count[i]);
            outputTextNoBacking(gameFont, xAt + 0.5f*fuaxWidth, yAt + 0.5f*fuaxHeight, 0.1f, gameState->fuaxResolution, str, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_BLACK, 1.3f, true, 1);

            ///////////////////////////////////////////

            xAt += 0.75f*scale;

            if(recipe->typeCount > 1 && i != (recipe->typeCount - 1)) {
                float offset = 50;
                outputTextNoBacking(gameFont, xAt + 0.5f*fuaxWidth, fuaxHeight - (yAt + 0.5f*fuaxHeight), 0.1f, gameState->fuaxResolution, "+", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_BLACK, 1.3f, true, 1);

            }

            xAt += 0.75f*scale;
        }


            
    }
    
    easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

    ////////////////////////////////////////////////////////////////////////////


    drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
}