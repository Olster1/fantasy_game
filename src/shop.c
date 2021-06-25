static Game_Shop *initGameShop_() {

	Game_Shop *shop = pushStruct(&globalLongTermArena, Game_Shop);

	for(int i = 0; i < arrayCount(shop->animationItemTimers); ++i) {
		shop->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
		shop->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
	}

	return shop;
}

static void addItemToShop(Game_Shop *shop, EntityType type,int count, bool isDisposable, float cost) {
	assert(shop->itemCount < MAX_SHOP_ITEM_COUNT);

	ItemInfo newItem = {};

	newItem.type = type;
	newItem.count = count;
	newItem.isDisposable = isDisposable;
	newItem.cost = cost;
	newItem.maxCount = count;

	shop->items[shop->itemCount++] = newItem;
}


static void initAllShopsWithItems(GameState *gameState) {
	//NOTE: The town shop
	{
		gameState->townShop = initGameShop_();

		addItemToShop(gameState->townShop, ENTITY_BOMB, 3, true, 10);
		addItemToShop(gameState->townShop, ENTITY_HEALTH_POTION_1, 5, true, 8);
	}
}


static void enterGameShop(Game_Shop *shop, GameState *gameState) {
	shop->inventoryBreathSelector = 0;

	if(gameState->gameIsPaused) { gameState->gameIsPaused = false; }

	shop->itemIndex = 0;
	shop->animationItemTimers[0].target = UI_ITEM_PICKER_MAX_SIZE;

	//Stop all the other animation timers
	for(int i = 1; i < arrayCount(shop->animationItemTimers); ++i) {
	    shop->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
	    shop->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
	}

	gameState->gameModeType = GAME_MODE_SHOP;
	gameState->current_shop = shop;
	gameState->gameIsPaused = true;
}

static void updateShop(Game_Shop *shop, float dt) {
	shop->timeSinceLastRefill += dt;

	if(shop->timeSinceLastRefill > 60*3) { //3 minutes
		for(int i = 0; i < shop->itemCount; ++i) {
			shop->items[i].count = shop->items[i].maxCount;
		}
	}
}

static void updateShop(Game_Shop *shop, GameState *gameState, Entity *player, EasyFont_Font *gameFont, OSAppInfo *appInfo) {

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

    
    if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_RIGHT) && shop->itemIndex  < (shop->itemCount - 1)) {
        shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MIN_SIZE;

        shop->inventoryBreathSelector = 0;


        shop->itemIndex++;
        easyConsole_pushInt(DEBUG_globalEasyConsole, shop->itemIndex);
        playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
        if(shop->itemIndex >= arrayCount(shop->items)) {
            shop->itemIndex = arrayCount(shop->items) - 1;

        }

        shop->animationItemTimers[shop->itemIndex].target = UI_ITEM_PICKER_MAX_SIZE;
    }

    /////////////////// DRAW THE SHOP ///////////////////////////////////

    EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);
  
    gameState->gameIsPaused = true;

    setViewTransform(globalRenderGroup, mat4());

    float fuaxWidth = 1920.0f;
    float fuaxHeight = appInfo->aspectRatio_yOverX*fuaxWidth;
    
    setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

    V2 size = getBounds("SHOP", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 2, gameState->fuaxResolution, 1);
    outputTextNoBacking(gameFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 100, 0.1f, gameState->fuaxResolution, "SHOP", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 2, true, 1);
    

    //DRAW BACKGROUND

    EasyTransform T;
    easyTransform_initTransform_withScale(&T, v3(0, 0, 0.4f), v3(1200, 1200, 1), EASY_TRANSFORM_NO_ID); 

    setModelTransform(globalRenderGroup, easyTransform_getTransform(&T));
   
    renderSetShader(globalRenderGroup, &textureProgram);
    renderDrawSprite(globalRenderGroup, findTextureAsset("quill.png"), COLOR_WHITE);

    /////////////////////////// DRAW THE ITEMS ///////////////////////////////////


    Texture *t = findTextureAsset("inventory_spot.png");
    Texture *hover_t = findTextureAsset("targeting2.png");
   
    float spacing = 300;
    float circleSize = 150;

    float xAt = -0.5f*spacing*shop->itemCount;
    float yAt = 0;

    for(int i = 1; i < shop->itemCount + 1; ++i) {   

        Texture *t_todraw = t;

        ItemInfo *itemI = &shop->items[i - 1];

        // The Item the player is hovering on
        if(shop->itemIndex == (i - 1)) {
            shop->inventoryBreathSelector += appInfo->dt;

            if(shop->inventoryBreathSelector > 1.0f) {
                shop->inventoryBreathSelector -= 1.0f;
            }

            float s = smoothStep00(1.3f*circleSize, shop->inventoryBreathSelector, 1.6f*circleSize);

            Matrix4 T_matrix = Matrix4_translate(Matrix4_scale(mat4(), v3(s, s, 0)), v3(0.1f*gameState->fuaxResolution.x + xAt, yAt, 0.3f));
                                
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

            //UPDATE PLAYER EQUIPING ITEM
            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_SPACE)) {
                
                //NOTE: BUY the item
                if(itemI->cost <= gameState->playerSaveProgress.playerInfo.moneyCount && itemI->count > 0) {

    	            //NOTE: "Buy" sound
	                playGameSound(&globalLongTermArena, gameState->saveSuccessSound, 0, AUDIO_BACKGROUND);

	                gameState->playerSaveProgress.playerInfo.moneyCount -= itemI->cost;
	                itemI->count--;

	                //NOTE: Add to the player inventory
	                addItemToPlayer(gameState, itemI->type, 1, itemI->isDisposable);

	
                } else {
                	//Notify user they don't have enough money
                	//NOTE: "Can't Buy" sound
	                playGameSound(&globalLongTermArena, gameState->exitUiSound, 0, AUDIO_BACKGROUND);
                }
            }

            //////////////////////
        }

        /////////////////////////////////////////////////

        ///////////////// DRAW THE ITEM COUNT AND ITEM  ////////////////////////////
        float x = 0.1f*fuaxWidth + xAt;
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
    
    easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

    ////////////////////////////////////////////////////////////////////////////


    drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
}