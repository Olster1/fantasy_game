#include "defines.h"
#include "easy_headers.h"

//****** TODO *********/////
/*
1. collision boxes should be at the bottom of the sprites, and not in the middle. Really big sprites would put the box back further.  

Gameplay:

The wolf should attack you
You pick up a sword which goes to your inventory - floating 3d model?
You pick up shield as well 
*/
///////////////////////////

static bool DEBUG_DRAW_SCENERY_TEXTURES = true;
static bool DEBUG_GRAVITY = false;
static bool DEBUG_DRAW_TERRAIN = true;
static bool DEBUG_LOCK_POSITION_TO_GRID = true;
static bool DEBUG_ANGLE_ENTITY_ON_CREATION = true;
static bool DEBUG_DRAW_COLLISION_BOUNDS = false; 
static bool DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS = false; 

#include "gameState.h"
#include "gameScene.c"
#include "editor.h"
#include "enemy_ai.c"
#include "entity.c"
#include "saveload.c"


#define MSF_GIF_IMPL
#include "msf_gif.h"

// #define STB_IMAGE_RESIZE_IMPLEMENTATION
// #include "stb_resize.h"

#define GIF_MODE 0


static void drawTriggerCollider_DEBUG(Entity *e, EasyCollider *collider) {
        V3 prevScale = e->T.scale;

        e->T.scale.x *= collider->dim2f.x;
        e->T.scale.y *= collider->dim2f.y;

        Quaternion Q = e->T.Q;

        V3 prevPos = e->T.pos;

        e->T.pos.x += collider->offset.x;
        e->T.pos.y += collider->offset.y; 
        e->T.pos.z = 0;

        // easyConsole_pushFloat(DEBUG_globalEasyConsole, e->collider1->offset.y);
        

        e->T.Q = identityQuaternion();

        Matrix4 T = easyTransform_getTransform(&e->T);

        e->T.Q = Q;

        V4 color = COLOR_RED;

        assert(collider->isTrigger);
        color = COLOR_YELLOW;
    

        setModelTransform(globalRenderGroup, T);
        renderDrawQuad(globalRenderGroup, color);    
        
        e->T.scale = prevScale;
        e->T.pos = prevPos;
}

static Texture *getInvetoryTexture(EntityType type) {
    Texture *t = 0;
    switch(type) {
        case ENTITY_HEALTH_POTION_1: {
            t = findTextureAsset("blue_jar.png");
            // assert(false);
        } break;
        case ENTITY_SWORD: {
            t = findTextureAsset("inventory_sword.png");
        } break;
        case ENTITY_SHEILD: {
            t = findTextureAsset("inventory_shield.png");
        } break;
        default: {

        }
    }
    return t;
}

static V3 roundToGridBoard(V3 in) {
    float xMod = (in.x < 0) ? -1 : 1;
    float yMod = (in.y < 0) ? -1 : 1;
    V3 result = v3((int)(in.x + xMod*0.5f), (int)(in.y + yMod*0.5f), (int)(in.z));
    return result;
}



int main(int argc, char *args[]) {
    
    EASY_ENGINE_ENTRY_SETUP()
    
    V2 screenDim = v2(DEFINES_WINDOW_SIZE_X, DEFINES_WINDOW_SIZE_Y); //init in create app function
    V2 resolution = v2(DEFINES_RESOLUTION_X, DEFINES_RESOLUTION_Y);

    #if GIF_MODE
    resolution.x *= 0.5f;
    resolution.y *= 0.5f;
    #endif
    
    OSAppInfo *appInfo = easyOS_createApp("Fantasy Game", &screenDim, false);

    
    if(appInfo->valid) {
        
        easyOS_setupApp(appInfo, &resolution, RESOURCE_PATH_EXTENSION);

        //Gif Recording stuff    
        int gif_width = resolution.x, gif_height = resolution.y, centisecondsPerFrame = 10, bitDepth = 16;
        MsfGifState gifState = {};
        /////

        FrameBuffer mainFrameBuffer;
        FrameBuffer toneMappedBuffer;
        FrameBuffer bloomFrameBuffer;
        FrameBuffer cachedFrameBuffer;

        {
            DEBUG_TIME_BLOCK_NAMED("Create frame buffers");
            
            //******** CREATE THE FRAME BUFFERS ********///
            
            mainFrameBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_DEPTH | FRAMEBUFFER_STENCIL | FRAMEBUFFER_HDR, 2);
            toneMappedBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_DEPTH | FRAMEBUFFER_STENCIL, 1);
            
            bloomFrameBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_HDR, 1);
            
            cachedFrameBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_HDR, 1);
            //////////////////////////////////////////////////
        
        }

        loadAndAddImagesToAssets("img/engine_icons/");
        loadAndAddImagesToAssets("img/temp_images/");
        loadAndAddImagesToAssets("img/inventory/");


        //Setup the gravity if it's on or not. GAMEPLAY: Could be an interesting gameplay feature: magentic rooms
        if(DEBUG_GRAVITY) {
            global_easyPhysics_gravityModifier = 1;
        } else {
            global_easyPhysics_gravityModifier = 0;
        }
        ///
        
        EasyCamera camera;
        easy3d_initCamera(&camera, v3(0, 0, -10));

        camera.orientation = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);
        
        EasyTransform sunTransform;
        easyTransform_initTransform(&sunTransform, v3(0, -10, 0), EASY_TRANSFORM_TRANSIENT_ID);

        sunTransform.Q = eulerAnglesToQuaternion(0, -0.5f*PI32, 0);
        
        EasyLight *light = easy_makeLight(&sunTransform, EASY_LIGHT_DIRECTIONAL, 1.0f, v3(1, 1, 1));
        easy_addLight(globalRenderGroup, light);
        
        GameState *gameState = initGameState((resolution.y / resolution.x));

        easyAnimation_initAnimation(&gameState->firePitAnimation, "firepit_idle");
        loadAndAddImagesStripToAssets(&gameState->firePitAnimation, "img/fantasy_sprites/firePlace.png", 64);

        easyAnimation_initAnimation(&gameState->torchAnimation, "torch_idle");
        loadAndAddImagesStripToAssets(&gameState->torchAnimation, "img/fantasy_sprites/torch.png", 64);

        {
            //WIZARD ANIMATIONS
            easyAnimation_initAnimation(&gameState->wizardRun, "wizardRun");
            // loadAndAddImagesStripToAssets(&gameState->wizardRun, "img/fantasy_sprites/wizard/Run.png", 231);
            easyAnimation_pushFrame(&gameState->wizardRun, "player.png");
            easyAnimation_pushFrame(&gameState->wizardRun, "player.png");

            easyAnimation_initAnimation(&gameState->wizardIdle, "wizardIdle");
            easyAnimation_pushFrame(&gameState->wizardIdle, "player.png");
            // loadAndAddImagesStripToAssets(&gameState->wizardIdle, "img/fantasy_sprites/wizard/Idle.png", 231);


            easyAnimation_initAnimation(&gameState->wizardBottom, "wizardBottom");
            easyAnimation_pushFrame(&gameState->wizardBottom, "player_down.png");

            easyAnimation_initAnimation(&gameState->wizardLeft, "wizardLeft");
            easyAnimation_pushFrame(&gameState->wizardLeft, "player_left.png");

            easyAnimation_initAnimation(&gameState->wizardRight, "wizardRight");
            easyAnimation_pushFrame(&gameState->wizardRight, "player_right.png");

            ////////////////////////////////////////////////////////////////////////////

            easyAnimation_initAnimation(&gameState->wizardAttack, "wizardAttack");
            loadAndAddImagesStripToAssets(&gameState->wizardAttack, "img/fantasy_sprites/wizard/Attack1.png", 231);

            easyAnimation_initAnimation(&gameState->wizardAttack2, "wizardAttack2");
            loadAndAddImagesStripToAssets(&gameState->wizardAttack2, "img/fantasy_sprites/wizard/Attack2.png", 231);

            easyAnimation_initAnimation(&gameState->wizardDeath, "wizardDeath");
            loadAndAddImagesStripToAssets(&gameState->wizardDeath, "img/fantasy_sprites/wizard/Death.png", 231);

            easyAnimation_initAnimation(&gameState->wizardHit, "wizardHit");
            loadAndAddImagesStripToAssets(&gameState->wizardHit, "img/fantasy_sprites/wizard/Hit.png", 231);


            easyAnimation_initAnimation(&gameState->wizardJump, "wizardJump");
            loadAndAddImagesStripToAssets(&gameState->wizardJump, "img/fantasy_sprites/wizard/Jump.png", 231);

            easyAnimation_initAnimation(&gameState->wizardFall, "wizardFall");
            loadAndAddImagesStripToAssets(&gameState->wizardFall, "img/fantasy_sprites/wizard/Fall.png", 231);

        }

        {
            easyAnimation_initAnimation(&gameState->werewolfIdle, "werewolfIdle");
            // loadAndAddImagesStripToAssets(&gameState->wizardRun, "img/fantasy_sprites/wizard/Run.png", 231);
            easyAnimation_pushFrame(&gameState->werewolfIdle, "werewolf2.png");
            
        }

        {
            easyAnimation_initAnimation(&gameState->skeletonAttack, "skeltonAttack");
            loadAndAddImagesStripToAssets(&gameState->skeletonAttack, "img/fantasy_sprites/skeleton/SAttack.png", 150);

            easyAnimation_initAnimation(&gameState->skeletonDeath, "skeltonDeath");
            loadAndAddImagesStripToAssets(&gameState->skeletonDeath, "img/fantasy_sprites/skeleton/SDeath.png", 150);

            easyAnimation_initAnimation(&gameState->skeltonIdle, "skeltonIdle");
            loadAndAddImagesStripToAssets(&gameState->skeltonIdle, "img/fantasy_sprites/skeleton/SIdle.png", 150);

            easyAnimation_initAnimation(&gameState->skeltonShield, "skeltonShield");
            loadAndAddImagesStripToAssets(&gameState->skeltonShield, "img/fantasy_sprites/skeleton/SShield.png", 150);

            easyAnimation_initAnimation(&gameState->skeltonHit, "skeltonHit");
            loadAndAddImagesStripToAssets(&gameState->skeltonHit, "img/fantasy_sprites/skeleton/SHit.png", 150);

            easyAnimation_initAnimation(&gameState->skeltonWalk, "skeltonWalk");
            loadAndAddImagesStripToAssets(&gameState->skeltonWalk, "img/fantasy_sprites/skeleton/SWalk.png", 150);

        }


        // char *tempStr = easy_createString_printf(&globalPerFrameArena, "%d %d %s", 1, 2, "HELLO");

        EntityManager *manager = pushStruct(&globalLongTermArena, EntityManager);
        initEntityManager(manager);



        int canCameraMove = 0;//EASY_CAMERA_MOVE;
        int canCamRotate = 0;

        ///////////************************/////////////////

        Matrix4 cameraLookAt_straight = easy3d_lookAt(v3(0, 0, -10), v3(0, 0, 0), v3(0, 1, 0));


        V4 lightBrownColor = hexARGBTo01Color(0xFFF5DEB3);
        Texture *t_square = findTextureAsset("inventory_equipped.png");
        Texture *particleImage = findTextureAsset("light_03.png");

        V3 itemPosition1 = v3(100, 100, 0.2f);
        V3 itemPosition2 = v3(250, 100, 0.2f);

        bool recording = false;
        float timeSinceLastFrame = 0.0f;

        //Editor state
        EditorState *editorState = initEditorState(&globalLongTermArena);

        //

        //NOTE: Load Splat files for editor
        
        
        char *imgFileTypes[] = {"jpg", "jpeg", "png", "bmp", "PNG"};
        char *folderName = concatInArena(globalExeBasePath, "img/environment_splats/", &globalPerFrameArena);
        FileNameOfType splatFileNames = getDirectoryFilesOfType(folderName, imgFileTypes, arrayCount(imgFileTypes));
        int splatCount = splatFileNames.count;
        for(int i = 0; i < splatFileNames.count; ++i) {
            char *fullName = splatFileNames.names[i];
            char *shortName = getFileLastPortion(fullName);
            if(shortName[0] != '.') { //don't load hidden file 
                addElementInfinteAlloc_notPointer(&gameState->splatList, shortName);

                Texture texOnStack = loadImage(fullName, TEXTURE_FILTER_LINEAR, true, true);
                Texture *tex = (Texture *)calloc(sizeof(Texture), 1);
                memcpy(tex, &texOnStack, sizeof(Texture));
                addElementInfinteAlloc_notPointer(&gameState->splatTextures, tex);
                
                
                easyConsole_addToStream(DEBUG_globalEasyConsole, shortName);
            }
            free(fullName);
            // free(shortName);
        }

        ////////////////////////


                char *modelDirs[] = { "models/robertModels/"};
                
                EasyAssetLoader_AssetArray allModelsForEditor = {};
                allModelsForEditor.count = 0;
                allModelsForEditor.assetType = ASSET_MODEL;

                
                for(int dirIndex = 0; dirIndex < arrayCount(modelDirs); ++dirIndex) {
                    char *dir = modelDirs[dirIndex];

                    //NOTE(ollie): Load materials first
                    char *fileTypes[] = { "mtl" };
                    easyAssetLoader_loadAndAddAssets(0, dir, fileTypes, arrayCount(fileTypes), ASSET_MATERIAL, EASY_ASSET_LOADER_FLAGS_NULL);
                    
                    //NOTE(ollie): Then load models
                    fileTypes[0] = "obj";
                    easyAssetLoader_loadAndAddAssets(&allModelsForEditor, dir, fileTypes, arrayCount(fileTypes), ASSET_MODEL, EASY_ASSET_LOADER_FLAGS_NULL);

                }

                char **modelsLoadedNames = pushArray(&globalLongTermArena, allModelsForEditor.count, char *);

                for(int i = 0; i < allModelsForEditor.count; ++i) {
                    char *nameOfModel = allModelsForEditor.array[i].model->name;

                    modelsLoadedNames[i] = nameOfModel;
                }



        #define LOAD_SCENE_FROM_FILE 1
        #if LOAD_SCENE_FROM_FILE
                gameScene_loadScene(gameState, manager, "swamp");
        #else
                //Init player first so it's in slot 0 which is special since we want to update the player position before other entities
                manager->player = initEntity(manager, &gameState->wizardIdle, v3(0, 0, 0), v2(2.4f, 2.0f), v2(0.2f, 0.25f), gameState, ENTITY_WIZARD, gameState->inverse_weight, 0, COLOR_WHITE, 0, true);

                // initEntity(manager, &gameState->firePitAnimation, v3(0, 1, 0), v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, 3);
                Entity *torch = initEntity(manager, &gameState->torchAnimation, v3(0, -3, 0), v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, -1, false);
                torch->subEntityType |= ENTITY_SUB_TYPE_TORCH;

                initEntity(manager, &gameState->skeltonIdle, v3(-3, 0, 0), v2(2.5f, 2.5f), v2(0.25f, 0.15f), gameState, ENTITY_SKELETON, gameState->inverse_weight, 0, COLOR_WHITE, 1, true);

                
                initEntity(manager, 0, v3(0, -4, 0), v2(10, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, &globalWhiteTexture, COLOR_BLACK, 2, true);
                initEntity(manager, 0, v3(3, -3.5f, 0), v2(3, 2.5f), v2(1, 1), gameState, ENTITY_SCENERY, 0, &globalWhiteTexture, COLOR_BLACK, 2, true);
        #endif


        gameState->cameraSnapDistance = 1.5f;
        gameState->jumpPower = 55000;
        Tweaker *tweaker = pushStruct(&globalLongTermArena, Tweaker);
        tweaker->varCount = 0;

        char *tweakerFileName = concatInArena(globalExeBasePath, "tweaker_file.txt", &globalLongTermArena); 

        EasyTransform treeT;
        easyTransform_initTransform_withScale(&treeT, v3(0, 0, 0), v3(0.1, 0.15, 0.1), EASY_TRANSFORM_STATIC_ID);

        treeT.Q = eulerAnglesToQuaternion(-0.5f*PI32, -0.5f*PI32, 0.5f*PI32);

        EasySound_LoopSound(playGameSound(&globalLongTermArena, easyAudio_findSound("dark_forest.wav"), 0, AUDIO_BACKGROUND));

        EasyTransform outlineTransform;
        easyTransform_initTransform_withScale(&outlineTransform, v3(0, 0, 0), v3(1, 1, 1), EASY_TRANSFORM_TRANSIENT_ID); 

        Texture *outlineSprite = findTextureAsset("outline.png");

        Texture *fadeBlackTexture = findTextureAsset("fade_black.png");

        EasyFont_Font *gameFont = easyFont_loadFontAtlas(concatInArena(globalExeBasePath, "fontAtlas_BebasNeue-Regular", &globalPerFrameArena), &globalLongTermArena);

        while(appInfo->running) {

            if(refreshTweakFile(tweakerFileName, tweaker)) {
                gameState->jumpPower = getIntFromTweakData(tweaker, "jumpPower");
                gameState->walkPower = getIntFromTweakData(tweaker, "walkPower");
                gameState->gravityScale = (float)getFloatFromTweakData(tweaker, "gravityScale");
                gameState->cameraSnapDistance = (float)getFloatFromTweakData(tweaker, "cameraSnapDistance");

                gameState->werewolf_attackSpeed = (float)getFloatFromTweakData(tweaker, "werewolf_attackSpeed");
                gameState->werewolf_restSpeed = (float)getFloatFromTweakData(tweaker, "werewolf_restSpeed");

                gameState->werewolf_knockback_distance = (float)getFloatFromTweakData(tweaker, "werewolf_knockback_distance");
                gameState->player_knockback_distance = (float)getFloatFromTweakData(tweaker, "player_knockback_distance");


                for(int i = 0; i < manager->entities.count; ++i) {
                    Entity *e = (Entity *)getElement(&manager->entities, i);
                    if(e && e->rb && e->rb->gravityFactor > 0) {
                        e->rb->gravityFactor = gameState->gravityScale;
                    }
                }
            }

            easyOS_processKeyStates(&appInfo->keyStates, resolution, &screenDim, &appInfo->running, !appInfo->hasBlackBars);
            easyOS_beginFrame(resolution, appInfo);
            
            beginRenderGroupForFrame(globalRenderGroup);
            
            clearBufferAndBind(appInfo->frameBackBufferId, COLOR_BLACK, FRAMEBUFFER_COLOR, 0);
            clearBufferAndBind(mainFrameBuffer.bufferId, COLOR_WHITE, mainFrameBuffer.flags, globalRenderGroup);
            
            renderEnableDepthTest(globalRenderGroup);
            renderEnableCulling(globalRenderGroup);
            setBlendFuncType(globalRenderGroup, BLEND_FUNC_STANDARD_PREMULTIPLED_ALPHA);
            renderSetViewport(globalRenderGroup, 0, 0, resolution.x, resolution.y);

            ////////////////////////////////////////////////////////////////////
            
            EasyCamera_MoveType camMove = (EasyCamera_MoveType)(EASY_CAMERA_ZOOM);

            camMove = (EasyCamera_MoveType)((int)camMove | canCameraMove | canCamRotate);
    
            //FOLLOW PLAYER  
            if(!canCameraMove) {
                V3 worldP = easyTransform_getWorldPos(&manager->player->T);
                {//update x position
                    float distance = absVal(worldP.x - camera.hidden_pos.x);

                    if(distance > gameState->cameraSnapDistance) {
                        float newPosX = lerp(camera.hidden_pos.x, clamp01(appInfo->dt*10.f), worldP.x);
                        camera.hidden_pos.x = newPosX;
                    }
                }

                {//update y position
                    float distance = absVal(worldP.y - 10 - camera.hidden_pos.y);

                    if(distance > gameState->cameraSnapDistance) {
                        float newPosY = lerp(camera.hidden_pos.y, clamp01(appInfo->dt*10.f), worldP.y - 10);
                        camera.hidden_pos.y = newPosY;
                    }
                }
            }
            ////////////////

            easy3d_updateCamera(&camera, &appInfo->keyStates, 1, 1000.0f, appInfo->dt, camMove);


            easy_setEyePosition(globalRenderGroup, camera.pos);
            
            // update camera first
            Matrix4 viewMatrix = easy3d_getWorldToView(&camera);
            Matrix4 perspectiveMatrix = projectionMatrixFOV(camera.zoom, resolution.x/resolution.y);
            Matrix4 perspectiveMatrix_inventory = projectionMatrixFOV(60.0f, resolution.x/resolution.y);

            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_1) && !gameState->isEditorOpen) {
                gameState->gameModeType = GAME_MODE_PAUSE_MENU;
                //Open  close menu sound
                playGameSound(&globalLongTermArena, gameState->openMenuSound, 0, AUDIO_BACKGROUND);
            }



            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_I)) {
                gameState->isLookingAtItems = !gameState->isLookingAtItems;
                // gameState->lookingAt_animTimer.current = UI_ITEM_RADIUS_MIN;
                // gameState->lookingAt_animTimer.target = UI_ITEM_RADIUS_MAX;

                if(gameState->gameIsPaused) { gameState->gameIsPaused = false; }

                gameState->indexInItems = 0;
                gameState->animationItemTimers[0].target = UI_ITEM_PICKER_MAX_SIZE;

                //Stop all the other animation timers
                for(int i = 1; i < arrayCount(gameState->animationItemTimers); ++i) {
                    gameState->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
                    gameState->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
                }

                if(gameState->isLookingAtItems) {
                    //Open  close menu sound
                    playGameSound(&globalLongTermArena, easyAudio_findSound("mapopen1.wav"), 0, AUDIO_BACKGROUND)->volume = 3.0f;

                } else {
                    playGameSound(&globalLongTermArena, easyAudio_findSound("mapclose.wav"), 0, AUDIO_BACKGROUND)->volume = 3.0f;
                }
               
                
            }

            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_F1)) {
                if(recording) {
                    easyFlashText_addText(&globalFlashTextManager, "STOP RECORDING");

                    MsfGifResult result = msf_gif_end(&gifState);
                    char str[1028];
                    sprintf(str, "%s%s", appInfo->saveFolderLocation, "/MyGif1.gif");

                    FILE * fp = fopen(str, "wb");
                    fwrite(result.data, result.dataSize, 1, fp);
                    fclose(fp);
                    msf_gif_free(result);

                } else {
                    easyFlashText_addText(&globalFlashTextManager, "START RECORDING");

                    msf_gif_begin(&gifState, gif_width, gif_height);
                    
                }
                recording = !recording;
                timeSinceLastFrame = 0;
            }





            if(gameState->isLookingAtItems) {
                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_RIGHT)) {
                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                    gameState->indexInItems--;
                    playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);


                    if(gameState->indexInItems < 0) {
                        gameState->indexInItems = arrayCount(gameState->itemSpots) - 1;
                    }

                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                }

                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT)) {
                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                    gameState->indexInItems++;
                    playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
                    if(gameState->indexInItems >= arrayCount(gameState->itemSpots)) {
                        gameState->indexInItems = 0;
                    }

                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                }
            } else 
            {
                EasyPhysics_UpdateWorld(&gameState->physicsWorld, appInfo->dt);    
            }

            setViewTransform(globalRenderGroup, viewMatrix);
            setProjectionTransform(globalRenderGroup, perspectiveMatrix);


            renderSetShader(globalRenderGroup, &phongProgram);
            // setModelTransform(globalRenderGroup, easyTransform_getTransform(&treeT));

            // renderModel(globalRenderGroup, findModelAsset_Safe("tree.obj"), v4(1, 1, 1, 1));

            RenderProgram *mainShader = &glossProgram;
            renderSetShader(globalRenderGroup, mainShader);

            AppKeyStates gameKeyStates = appInfo->keyStates;
            AppKeyStates consoleKeyStates = appInfo->keyStates;
            if(appInfo->console.isInFocus || canCameraMove) {
                gameKeyStates = {};
            } else if(easyConsole_isOpen(&appInfo->console)) {
                consoleKeyStates = {};
                consoleKeyStates.gameButtons[BUTTON_ESCAPE] = gameKeyStates.gameButtons[BUTTON_ESCAPE];
            }

           
           V3 mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, -camera.pos.z, easy3d_getViewToWorld(&camera));
           EasyRay mouseP_worldRay = {};
           mouseP_worldRay.origin = camera.pos;
           mouseP_worldRay.direction = v3_minus(mouseP_inWorldP, camera.pos);


            /////////////////

             Entity *insideEntity = 0;
            //DEBUG
            // if(true)
            {
                

                //Check if user grabs the entity to edit
                for(int i = 0; i < manager->entities.count; ++i) {
                    Entity *e = (Entity *)getElement(&manager->entities, i);
                    if(e) {

                        if(gameState->isEditorOpen) {

                            Matrix4 rotationT  = easyTransform_getWorldRotation(&e->T);

                            V3 position = easyTransform_getWorldPos(&e->T);

                            V3 scale = easyTransform_getWorldScale(&e->T);

                            Rect3f bounds = rect3fCenterDimV3(v3(0, 0, 0), scale);

                            EasyPhysics_RayCastAABB3f_Info castInfo = EasyPhysics_CastRayAgainstAABB3f(rotationT, position, v3(1, 1, 1), bounds, mouseP_worldRay.direction, mouseP_worldRay.origin);
                            if(castInfo.didHit && e->type != ENTITY_TERRAIN) {
                                insideEntity = e;

                                if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                                    editorState->entitySelected = e;
                                    editorState->entityIndex = i;

                                    editorState->grabOffset = v3_scale(-1, castInfo.hitP);
                                    editorState->grabOffset.z = 0;
                                    // easyConsole_pushV3(DEBUG_globalEasyConsole, editorState->grabOffset);

                                }    
                            }

                        }


                        //Draw the DEBUG collider info 
                        if(e->collider && DEBUG_DRAW_COLLISION_BOUNDS) {   
                            V3 prevScale = e->T.scale;

                            e->T.scale.x *= e->collider->dim2f.x;
                            e->T.scale.y *= e->collider->dim2f.y;

                            e->T.pos.x += e->collider->offset.x;
                            e->T.pos.y += e->collider->offset.y; 

                            Quaternion Q = e->T.Q;

                            float prevZ = e->T.pos.z;

                            e->T.pos.z = 0;
                            e->T.Q = identityQuaternion();

                            Matrix4 T = easyTransform_getTransform(&e->T);

                            e->T.Q = Q;

                            V2 dim = e->collider->dim2f;

                            V4 color = COLOR_RED;

                            if(e->collider->isTrigger) {
                                color = COLOR_YELLOW;
                            } 
                        

                            setModelTransform(globalRenderGroup, T);
                            
                            renderDrawQuad(globalRenderGroup, color);    
                            
                            
                            e->T.scale = prevScale;
                            e->T.pos.z = prevZ;
                        }

                        if(e->collider1 && DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS) {   
                            drawTriggerCollider_DEBUG(e, e->collider1);
                        }
                        if(e->collider2 && DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS) {   
                            drawTriggerCollider_DEBUG(e, e->collider2);
                        }
                        if(e->collider3 && DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS) {   
                            drawTriggerCollider_DEBUG(e, e->collider3);
                        }
                        if(e->collider4 && DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS) {   
                            drawTriggerCollider_DEBUG(e, e->collider4);
                        }
                    }
                    
                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
                renderClearDepthBuffer(mainFrameBuffer.bufferId);

                // if(wasReleased(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                //     editorState->entitySelected = 0;
                // }

                
            }
            //////////////////////////////////////////
           //Draw the square to where the mouse is pointing
           {
               V3 hitP;
               float tAt;

               EasyPlane floor = {};

               floor.origin = v3(0, 0, 0);
               floor.normal = v3(0, 0, -1);

               if(easyMath_castRayAgainstPlane(mouseP_worldRay, floor, &hitP, &tAt)) {

                   
                   hitP = roundToGridBoard(hitP);

                   outlineTransform.pos = hitP;

                   // easyConsole_pushFloat(DEBUG_globalEasyConsole, outlineTransform.pos.y);

                   Matrix4 outlineT = easyTransform_getTransform(&outlineTransform);
                   setModelTransform(globalRenderGroup, outlineT);
                   renderDrawSprite(globalRenderGroup, outlineSprite, COLOR_WHITE);

                   drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
               }
           }



            //DRAW THE TERRATIN FIRST
            if(gameState->currentTerrainEntity && DEBUG_DRAW_TERRAIN) {
                Entity *terrainEntity = (Entity *)(gameState->currentTerrainEntity);
                setModelTransform(globalRenderGroup, easyTransform_getTransform(&terrainEntity->T));
                renderDrawTerrain2d(globalRenderGroup, v4(1, 1, 1, 1), &gameState->terrainPacket);
                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
            }
            ////////////////////////////

            for(int i = 0; i < manager->entities.count; ++i) {
                Entity *e = (Entity *)getElement(&manager->entities, i);
                if(e) {
                    if(e->model) {
                        Quaternion Q = e->T.Q;
                        V3 rotation = v3(0, 0, 0);

                        if(e->type == ENTITY_SHEILD) {
                            rotation.z = e->rotation;
                            // rotation.y = 0.5f*PI32;

                            Quaternion Q1 = eulerAnglesToQuaternion(rotation.y, rotation.x, rotation.z);
                            e->T.Q = quaternion_mult(Q1, Q);

                        } else {
                            rotation.z = e->rotation;
                            Quaternion Q1 = eulerAnglesToQuaternion(rotation.y, rotation.x, rotation.z);

                            e->T.Q = quaternion_mult(Q, Q1);
                        }
                        

                        Matrix4 T = easyTransform_getTransform(&e->T);

                        e->T.Q = Q;
                        setModelTransform(globalRenderGroup, T);
                        renderSetShader(globalRenderGroup, &phongProgram);
                        renderModel(globalRenderGroup, e->model, e->colorTint);
                    }    
                }
                
            }

            //Render Player holding inventory item
            {
                for(int i = 0; i < arrayCount(gameState->playerHolding); ++i) {
                    EntityType type = gameState->playerHolding[i];

                    if(type != ENTITY_NULL) {

                        Entity *p = manager->player;
                        EasyTransform T = p->T;

                        V3 rotation = v3(0, 0, 0);

                        Matrix4 offsetMatrix = mat4();

                        T.scale = v3(2, 2, 2);

                        EasyModel *model = 0;

                        V3 offset = v3(0, 0, 0);

                        if(i == 0) { //left hand
                            offset.x = -0.5f;
                        } else { //right hand
                            offset.x = 0.5f;
                        }

                        offset.y += 0.5f;

                        offset.z = -1;

                        switch(type) {
                            case ENTITY_SWORD: {
                                model = findModelAsset_Safe("sword.obj"); 
                                offset.y += 1.0f;
                                T.scale = v3(5, 5, 5);
                                rotation.y = 0.5*PI32;

                                // rotation.x = -0.5f*PI32;

                                if(gameState->swordSwingTimer >= 0.0f) {

                                    gameState->swordSwingTimer += appInfo->dt;

                                    float sideFactor = -1;

                                    if(i == 1) {
                                        sideFactor = 1;
                                    }

                                    float swordLifeLeft = (float)gameState->swordSwingTimer / (float)0.3f;
                                    float lerpT = smoothStep00(0, swordLifeLeft, sideFactor*0.25f*PI32);
                                    // easyConsole_pushFloat(DEBUG_globalEasyConsole, swordLifeLeft);

                                    rotation.x = lerpT;

                                    

                                    offsetMatrix = Matrix4_translate(offsetMatrix, v3(0, 0.15f, 0));
                                    offset.y -= 0.15f;

                                    if(swordLifeLeft >= 1.0f) {
                                        gameState->swordSwingTimer = -1.0f;
                                    }
                                }
                                

                            } break;
                            case ENTITY_SHEILD: {
                                model = findModelAsset_Safe("shield.obj"); 
                                offset.y += 0.5f;
                                offset.z = -0.5f;

                                float sideFactor = 1;
                                 if(i == 1) { //right
                                    sideFactor = -1;
                                }

                                if(p->shieldInUse) {
                                    offset.z -= 0.5f;
                                    offset.x += sideFactor*0.5f;                                    
                                }

                                rotation.x = 0.5f*PI32;
                            } break;
                            default: {

                            }
                        }

                        Quaternion Q1 = eulerAnglesToQuaternion(rotation.y, rotation.x, rotation.z);

                        T.Q = Q1;

                        Matrix4 Tm = Mat4Mult(easyTransform_getTransform(&T), offsetMatrix);

                        Tm = Mat4Mult(Matrix4_translate(mat4(), offset), Tm);

                        if(model) {
                            setModelTransform(globalRenderGroup, Tm);
                            renderSetShader(globalRenderGroup, &phongProgram);
                            renderModel(globalRenderGroup, model, COLOR_WHITE);
                        }
                    }

                }
            }
            


            ////

            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            

            for(int i = 0; i < manager->entities.count; ++i) {
                Entity *e = (Entity *)getElement(&manager->entities, i);
                if(e) {
                    updateEntity(manager, e, gameState, appInfo->dt, &gameKeyStates, &appInfo->console, &camera, manager->player, gameState->gameIsPaused, EasyCamera_getZAxis(&camera));        
                
                    if(e->isDead) {
                        ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteForFrame);
                        int *indexToAdd = (int *)arrayInfo.elm;
                        *indexToAdd = i;
                    }
                }
                
            }


            { //NOTE: Update the damage numbers
                for(int i = 0; i < manager->damageNumbers.count; ++i) {
                    Entity_DamageNumber *num = (Entity_DamageNumber *)getElement(&manager->damageNumbers, i);

                    if(num) {
                        num->aliveTimer += appInfo->dt;

                        float canVal = num->aliveTimer / 1.0f;

                        char *at = num->str;

                        float xAt = 0; 

                        float fontSize_to_worldSize = 1.3f / (float)gameFont->fontHeight;

                        num->pos = v3_plus(num->pos, v3_scale(appInfo->dt, v3(0, 0, -3)));  

                        while(*at) {

                            unsigned int unicodePoint = easyUnicode_utf8_codepoint_To_Utf32_codepoint((char **)&at, true);
                            
                            // easyConsole_addToStream(DEBUG_globalEasyConsole, "Update Number");

                            EasyFont_Glyph *g = easyFont_findGlyph(gameFont, unicodePoint); 
                            assert(g->codepoint == unicodePoint);

                            if(g->hasTexture) {

                                V3 position = v3_plus(num->pos, v3(xAt, -fontSize_to_worldSize*g->yOffset, -1.0f)); //num->pos;//

                                // easyConsole_pushV3(DEBUG_globalEasyConsole, position);

                                gameState->tempTransform.pos = position;

                                float width = g->texture.width*fontSize_to_worldSize;
                                float height = g->texture.height*fontSize_to_worldSize;

                                gameState->tempTransform.scale.xy = v2(width, height);

                                V4 color = COLOR_WHITE;

                                color.w = 1.0f - canVal;

                                setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
                                renderdrawGlyph(globalRenderGroup, &g->texture, color);
                            
                            }

                            xAt += (g->texture.width + g->xOffset)*fontSize_to_worldSize;
                        }
                        

                        // num->pos;
                        // num->str;



                        if(canVal >= 1.0f) {
                            //free the string
                            easyPlatform_freeMemory(num->str);

                            //remove the damaage number of the list
                            removeElement_ordered(&manager->damageNumbers, i);
                        }
                    }
                }
            }

            renderSetShader(globalRenderGroup, mainShader); 

            { //NOTE: Add entities that need adding
                for(int i = 0; i < manager->entitiesToAddForFrame.count; ++i) {
                    EntityToAdd *e = (EntityToAdd *)getElement(&manager->entitiesToAddForFrame, i);
                    if(e) {

                        float layer = -0.5f;    
                        float lifeSpan = 3.0f;

                        float inverse_weight = 1.0f / 10.0f;

                        V2 size = v2(1, 1);
                        Texture *t = 0;
                        float reboundFactor = 1.0f;
                        if(e->type == ENTITY_HEALTH_POTION_1) {
                            t = findTextureAsset("blue_jar.png");
                            assert(t);
                            lifeSpan = -1.0f;
                            inverse_weight = 1.0f / 20.0f; 
                            size = v2(0.4f, 0.4f);
                            reboundFactor = 0.98f;

                            easyConsole_addToStream(DEBUG_globalEasyConsole, "Creating potion");
                        }



                        Entity *e1 = initEntity(manager, &gameState->firePitAnimation, e->position, size, v2(0.9f, 0.9f), gameState, e->type, inverse_weight, t, COLOR_WHITE, layer, true);
                        e1->rb->dP = e->dP;

                        

                        //NOTE: This isn't a thing anymore
                        if(e->subType == ENTITY_SUB_TYPE_SWORD) {
                            //sword life span
                            lifeSpan = 0.3f;
                        } 
                            
                        e1->maxLifeSpan = lifeSpan;
                        e1->lifeSpanLeft = lifeSpan;
                        e1->rb->reboundFactor = reboundFactor;
                        // easyConsole_pushFloat(DEBUG_globalEasyConsole, lifeSpan);


                        e1->subEntityType = e->subType;

                        e1->T.parent = e->parentT;
                    }
                }
            }

            easyArray_clear(&manager->entitiesToAddForFrame);

            { //NOTE: Remove entities that need deleting
                for(int i = 0; i < manager->entitiesToDeleteForFrame.count; ++i) {
                    int *indexToDelete = (int *)getElement(&manager->entitiesToDeleteForFrame, i);
                    if(indexToDelete) {
                        Entity *e = (Entity *)getElement(&manager->entities, *indexToDelete);

                        e->isDeleted = true;

                        if(e->collider) {
                            EasyPhysics_removeCollider(&gameState->physicsWorld, e->collider);
                        }

                        if(e->rb) {
                            EasyPhysics_removeRigidBody(&gameState->physicsWorld, e->rb);
                        }


                        removeElement_ordered(&manager->entities, *indexToDelete);
                    }
                }

                easyArray_clear(&manager->entitiesToDeleteForFrame);
            }   



            if(!appInfo->console.isInFocus && wasPressed(appInfo->keyStates.gameButtons, BUTTON_F5)) {
                gameState->isEditorOpen = !gameState->isEditorOpen;
            }


            //DRAW THE PLAYER HUD
            {

                EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);


                float fuaxWidth = 1920.0f;
                float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

                setViewTransform(globalRenderGroup, mat4());
                setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen_BottomLeft(fuaxWidth, fuaxHeight));

                Entity *p = manager->player;

                float staminaPercent = p->stamina / p->maxStamina;

                float maxBarPixels = 300;
                float barHeight = 60;

                float x = 0.1*fuaxWidth;  
                float y = fuaxHeight - (0.1f*fuaxHeight);

                //Stamina points backing
                Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(maxBarPixels, barHeight, 0)), v3(x, y, 0.4f));
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, &globalWhiteTexture, COLOR_GREY);

                float barWidth = staminaPercent*maxBarPixels;
                float xOffset = 0.5f*(maxBarPixels - barWidth);

                //Stamina points percent bar
                T = Matrix4_translate(Matrix4_scale(mat4(), v3(barWidth, barHeight, 0)), v3(x - xOffset, y, 0.3f));
                
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, &globalWhiteTexture, COLOR_GREEN);



                //////////////////////////// HEALTH BAR //////////////////////////////////////////


                float healthPercent = (float)p->health / (float)p->maxHealth;

                assert(healthPercent <= 1.0f);

                y -= 1.5f*barHeight;

                //health points backing
                T = Matrix4_translate(Matrix4_scale(mat4(), v3(maxBarPixels, barHeight, 0)), v3(x, y, 0.4f));
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, &globalWhiteTexture, COLOR_GREY);

                barWidth = healthPercent*maxBarPixels;
                xOffset = 0.5f*(maxBarPixels - barWidth);

                assert(barWidth <= maxBarPixels);

                //hea;th points percent bar
                T = Matrix4_translate(Matrix4_scale(mat4(), v3(barWidth, barHeight, 0)), v3(x - xOffset, y, 0.3f));

                // easyConsole_pushFloat(DEBUG_globalEasyConsole, barWidth);
                
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, &globalWhiteTexture, COLOR_RED);

                ///////////////////////////////////////////////////////////////////////

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

                renderClearDepthBuffer(mainFrameBuffer.bufferId);

                easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);
            }
            /////////////////////////////


                
            if(gameState->isEditorOpen){
                easyEditor_startDockedWindow(appInfo->editor, "Editor Window", EASY_EDITOR_DOCK_TOP_RIGHT);
                // easyEditor_startWindow(appInfo->editor, "Editor Window", 300, 300);
                
                
                // easyEditor_pushFloat1(appInfo->editor, "first float: ", &v.x);
                // easyEditor_pushSlider(appInfo->editor, "value slider: ", &v.x, -10, 10);

                editorState->createMode = (EditorCreateMode)easyEditor_pushList(appInfo->editor, "Editor Mode: ", EditorCreateModesStrings, arrayCount(EditorCreateModesStrings));

                int splatIndexOn = 0;
                if(editorState->createMode == EDITOR_CREATE_SCENERY || editorState->createMode == EDITOR_CREATE_SCENERY_RIGID_BODY || editorState->createMode == EDITOR_CREATE_ONE_WAY_PLATFORM) {
                    int splatIndexOn = easyEditor_pushList(appInfo->editor, "Sprites: ", (char **)gameState->splatList.memory, gameState->splatList.count); 

                }   

                EasyModel *modelSelected = 0;

                if(editorState->createMode == EDITOR_CREATE_3D_MODEL) {
                    int modelIndex = easyEditor_pushList(appInfo->editor, "Models: ", modelsLoadedNames, allModelsForEditor.count); 

                    modelSelected = allModelsForEditor.array[modelIndex].model;   

                }
                

                bool pressed = wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE);

                Texture *splatTexture = ((Texture **)(gameState->splatTextures.memory))[splatIndexOn];

                if(!appInfo->editor->isHovering) {

                    EasyPlane floor = {};

                    floor.origin = v3(0, 0, 0);
                    floor.normal = v3(0, 0, -1);

                    V3 hitP;
                    float tAt;

                    if(easyMath_castRayAgainstPlane(mouseP_worldRay, floor, &hitP, &tAt)) {
                        //NOTE: Lock entities to integer coordinates - grid base gameplay
                        if(DEBUG_LOCK_POSITION_TO_GRID) {
                            hitP = roundToGridBoard(hitP);
                        }
                    }

                    editorState->grabOffset = v3(0, 0, 0);
                    switch(editorState->createMode) {
                        case EDITOR_CREATE_SELECT_MODE: {
                            //do nothing
                        } break;
                        case EDITOR_CREATE_BLOCK_TO_PUSH: {
                            if(pressed) {
                                editorState->entitySelected = initPushRock(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                            }
                        } break;
                        case EDITOR_CREATE_3D_MODEL: {
                            if(pressed) {
                                editorState->entitySelected = init3dModel(gameState, manager, hitP, modelSelected);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                            }
                        } break;
                        case EDITOR_CREATE_TERRAIN: {
                            if(pressed) {
                                editorState->entitySelected = initTerrain(gameState, manager, hitP);
                                gameState->currentTerrainEntity = editorState->entitySelected; 
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                            }
                        } break;
                        case EDITOR_CREATE_SCENERY: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, hitP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;
                            }

                        } break;
                        case EDITOR_CREATE_ONE_WAY_PLATFORM: {
                            if(pressed) {
                                initOneWayPlatform(gameState, manager, hitP, splatTexture);
                                editorState->entityIndex = manager->lastEntityIndex;
                            }
                        } break;
                        case EDITOR_CREATE_SCENERY_RIGID_BODY: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, hitP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, true);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;   
                        case EDITOR_CREATE_SKELETON: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, &gameState->skeltonIdle, hitP, v2(2.5f, 2.5f), v2(0.25f, 0.15f), gameState, ENTITY_SKELETON, gameState->inverse_weight, 0, COLOR_WHITE, 1, true);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_WEREWOLF: {
                            if(pressed) {
                                editorState->entitySelected = initWerewolf(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_SWORD: {
                            if(pressed) {
                                editorState->entitySelected = initSword(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_SHEILD: {
                            if(pressed) {
                                editorState->entitySelected = initSheild(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_SIGN: {
                            if(pressed) {
                                editorState->entitySelected = initSign(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_CHECKPOINT: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, hitP, v2(1, 1), v2(1, 1), gameState, ENITY_CHECKPOINT, 0, &globalWhiteTexture, COLOR_BLUE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_TORCH: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, &gameState->torchAnimation, hitP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_AUDIO_CHECKPOINT: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, hitP, v2(1, 1), v2(1, 1), gameState, ENITY_AUDIO_CHECKPOINT, 0, &globalWhiteTexture, COLOR_BLUE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                    }
                }
                



                // if(easyEditor_pushButton(appInfo->editor, "Save Level")) {
                    
                //     // gameScene_saveScene(manager, "scene1/");

                //     easyFlashText_addText(&globalFlashTextManager, "SAVED");
                // }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_DRAW_COLLISION_BOUNDS) ? "Bounds Off" : "Bounds On")) {
                    DEBUG_DRAW_COLLISION_BOUNDS = !DEBUG_DRAW_COLLISION_BOUNDS;
                    // manager->player->collider1->offset.y = 0;
                }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS) ? "Bounds Triggers Off" : "Bounds Triggers On")) {
                    DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS = !DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS;
                }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_DRAW_TERRAIN) ? "Terrain Off" : "Terrain On")) {
                    DEBUG_DRAW_TERRAIN = !DEBUG_DRAW_TERRAIN;
                }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_LOCK_POSITION_TO_GRID) ? "Lock Pos Off" : "Lock Pos On")) {
                    DEBUG_LOCK_POSITION_TO_GRID = !DEBUG_LOCK_POSITION_TO_GRID;
                }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_ANGLE_ENTITY_ON_CREATION) ? "Angle Entity Off" : "Angle Entity On")) {
                    DEBUG_ANGLE_ENTITY_ON_CREATION = !DEBUG_ANGLE_ENTITY_ON_CREATION;
                }
                
                
                if(easyEditor_pushButton(appInfo->editor, (DEBUG_DRAW_SCENERY_TEXTURES) ? "Scenery Off" : "Scenery On")) {
                    DEBUG_DRAW_SCENERY_TEXTURES = !DEBUG_DRAW_SCENERY_TEXTURES;
                }

                if(easyEditor_pushButton(appInfo->editor, "Zero Pos Player")) {
                    manager->player->T.pos.xy = v2(0, 0);
                }

                if(easyEditor_pushButton(appInfo->editor, "Undo")) {
                    if(editorState->entitiesDeletedBuffer.count > 0) {
                        EditorUndoState *u = (EditorUndoState *)getElement(&editorState->entitiesDeletedBuffer, editorState->entitiesDeletedBuffer.count - 1);
                        if(u) {



                            /////////// Remove the element at the end
                            removeElement_ordered(&manager->entities, editorState->entitiesDeletedBuffer.count - 1);
                        }
                    }
                }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_GRAVITY) ? "Gravity Off" : "Gravity On")) {
                    DEBUG_GRAVITY = !DEBUG_GRAVITY;
                    if(DEBUG_GRAVITY) {
                        global_easyPhysics_gravityModifier = 1;
                    } else {
                        global_easyPhysics_gravityModifier = 0;
                    }
                }

                easyEditor_endWindow(appInfo->editor); //might not actuall need this



                if(editorState->entitySelected) {


                    Entity *e = (Entity *)editorState->entitySelected;
                                
                    //Draw the Gizmo
                    V3 entP = easyTransform_getWorldPos(&e->T);

                    V3 pos = v3_scale(0.5f, easyTransform_getWorldScale(&e->T));
                    Matrix4 rotation = easyTransform_getWorldRotation(&e->T);


                    mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, pos.z - camera.pos.z, easy3d_getViewToWorld(&camera));

                    V3 rightTopCorner = v3(pos.x, pos.y, pos.z);
                    V3 leftTopCorner = v3(-pos.x, pos.y, pos.z);
                    V3 rightBottomCorner = v3(pos.x, -pos.y, pos.z);
                    V3 leftBottomCorner = v3(-pos.x, -pos.y, pos.z);

                    float handleSize = 0.1f;

                    Matrix4 a = Mat4Mult(Matrix4_translate(mat4(), entP), Mat4Mult(rotation, Matrix4_translate(Matrix4_scale(mat4(), v3(handleSize, handleSize, 0)), rightTopCorner)));
                    Matrix4 b = Mat4Mult(Matrix4_translate(mat4(), entP), Mat4Mult(rotation, Matrix4_translate(Matrix4_scale(mat4(), v3(handleSize, handleSize, 0)), leftTopCorner)));
                    Matrix4 c = Mat4Mult(Matrix4_translate(mat4(), entP), Mat4Mult(rotation, Matrix4_translate(Matrix4_scale(mat4(), v3(handleSize, handleSize, 0)), rightBottomCorner)));
                    Matrix4 d = Mat4Mult(Matrix4_translate(mat4(), entP), Mat4Mult(rotation, Matrix4_translate(Matrix4_scale(mat4(), v3(handleSize, handleSize, 0)), leftBottomCorner)));

                    setModelTransform(globalRenderGroup, a);
                    renderDrawQuad(globalRenderGroup, (editorState->gizmoSelect == EDITOR_GIZMO_TOP_RIGHT) ? COLOR_BLUE : COLOR_RED);

                    setModelTransform(globalRenderGroup, b);
                    renderDrawQuad(globalRenderGroup, (editorState->gizmoSelect == EDITOR_GIZMO_TOP_LEFT) ? COLOR_BLUE : COLOR_RED);

                    setModelTransform(globalRenderGroup, c);
                    renderDrawQuad(globalRenderGroup, (editorState->gizmoSelect == EDITOR_GIZMO_BOTTOM_RIGHT) ? COLOR_BLUE : COLOR_RED);

                    setModelTransform(globalRenderGroup, d);
                    renderDrawQuad(globalRenderGroup, (editorState->gizmoSelect == EDITOR_GIZMO_BOTTOM_LEFT) ? COLOR_BLUE : COLOR_RED);

                    float handleScale = 1.0f;

                    float handleDim = handleScale*handleSize;
                    Rect3f handleBounds = rect3fCenterDimV3(v3(0, 0, 0), v3(handleDim, handleDim, handleDim));

                    

                    V3 aP = v3_plus(transformPositionV3(rightTopCorner, rotation), entP);
                    
                    if(EasyPhysics_CastRayAgainstAABB3f(rotation, aP, v3(1, 1, 1), handleBounds, mouseP_worldRay.direction, mouseP_worldRay.origin).didHit) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_TOP_RIGHT;
                        }    
                    }

                    V3 bP = v3_plus(transformPositionV3(leftTopCorner, rotation), entP);
                    if(EasyPhysics_CastRayAgainstAABB3f(rotation, bP, v3(1, 1, 1), handleBounds, mouseP_worldRay.direction, mouseP_worldRay.origin).didHit) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_TOP_LEFT;
                        }    
                    }

                    V3 cP = v3_plus(transformPositionV3(rightBottomCorner, rotation), entP);
                    if(EasyPhysics_CastRayAgainstAABB3f(rotation, cP, v3(1, 1, 1), handleBounds, mouseP_worldRay.direction, mouseP_worldRay.origin).didHit) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_BOTTOM_RIGHT;
                        }    
                    }

                    V3 dP = v3_plus(transformPositionV3(leftBottomCorner, rotation), entP);
                    if(EasyPhysics_CastRayAgainstAABB3f(rotation, dP, v3(1, 1, 1), handleBounds, mouseP_worldRay.direction, mouseP_worldRay.origin).didHit) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_BOTTOM_LEFT;
                        }    
                    }


                    /////////////////
                    //Update the Gizmo
                    switch(editorState->gizmoSelect) {
                        case EDITOR_GIZMO_NONE: {

                        } break;
                        case EDITOR_GIZMO_TOP_RIGHT: {
                            // V3 diff = v3_minus(mouseP_inWorldP, dP);

                            // V2 prevS = e->T.scale.xy;

                            // e->T.scale.xy = diff.xy;
                            // e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2_minus(e->T.scale.xy, prevS)));
                            
                        } break;
                        case EDITOR_GIZMO_TOP_LEFT: {

                            // V3 diff = v3_minus(mouseP_inWorldP, cP);

                            // V2 prevS = e->T.scale.xy;

                            // V2 positiveSize = absVal_v2(diff.xy);

                            // e->T.scale.xy = positiveSize;
                            // e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2(prevS.x - e->T.scale.x, e->T.scale.y - prevS.y)));

                        } break;
                        case EDITOR_GIZMO_BOTTOM_LEFT: {
                            // V3 diff = v3_minus(mouseP_inWorldP, aP);

                            // V2 prevS = e->T.scale.xy;

                            // V2 positiveSize = absVal_v2(diff.xy);

                            // e->T.scale.xy = positiveSize;
                            // e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2(prevS.x - e->T.scale.x, prevS.y - e->T.scale.y)));

                        } break;
                        case EDITOR_GIZMO_BOTTOM_RIGHT: {

                            // V3 diff = v3_minus(mouseP_inWorldP, bP);

                            // V2 prevS = e->T.scale.xy;

                            // V2 positiveSize = absVal_v2(diff.xy);

                            // e->T.scale.xy = positiveSize;
                            // e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2(e->T.scale.x - prevS.x, prevS.y - e->T.scale.y)));

                        } break;
                    }

                    if(wasReleased(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                        editorState->gizmoSelect = EDITOR_GIZMO_NONE;   
                    }

                    //////////



                    easyEditor_startDockedWindow(appInfo->editor, "Entity Window", EASY_EDITOR_DOCK_TOP_LEFT);
                    
                    easyEditor_pushInt1(appInfo->editor, "Id: ", &(int)e->T.id);
                    easyEditor_pushInt1(appInfo->editor, "Flags: ", &(int)e->flags);

                    easyEditor_pushButton(appInfo->editor, MyEntity_EntityTypeStrings[(int)e->type]);
                    easyEditor_pushInt1(appInfo->editor, "Subtype: ", &(int)e->subEntityType);
                    
                    easyEditor_pushFloat3(appInfo->editor, "Position: ", &e->T.pos.x, &e->T.pos.y, &e->T.pos.z);
                    easyEditor_pushFloat3(appInfo->editor, "Scale: ", &e->T.scale.x, &e->T.scale.y, &e->T.scale.z);

                    ////////////////////////////////////////////////////////////////////
                    //NOTE(ollie): Rotation with euler angles
                        
                    //NOTE: The rotation of a _single_ quaternion formed from euler angles, the shape keeps their original axis _tied_ to the shape. 
                    //      So as it rotates on Y and turns the X and z axis stays with the shape. This isn't the case when we concat rotations togeether.   


                    V3 tempEulerAngles = easyMath_QuaternionToEulerAngles(e->T.Q);
                    
                    
                    // NOTE(ollie): Swap to degrees 
                    tempEulerAngles.x = easyMath_radiansToDegrees(tempEulerAngles.x);
                    tempEulerAngles.y = easyMath_radiansToDegrees(tempEulerAngles.y);
                    tempEulerAngles.z = easyMath_radiansToDegrees(tempEulerAngles.z);


                    easyEditor_pushFloat3(appInfo->editor, "Rotation: ", &tempEulerAngles.x, &tempEulerAngles.y, &tempEulerAngles.z);
                    
                    // NOTE(ollie): Convert back to radians
                    tempEulerAngles.x = easyMath_degreesToRadians(tempEulerAngles.x);
                    tempEulerAngles.y = easyMath_degreesToRadians(tempEulerAngles.y);
                    tempEulerAngles.z = easyMath_degreesToRadians(tempEulerAngles.z);

                    e->T.Q = eulerAnglesToQuaternion(tempEulerAngles.y, tempEulerAngles.x, tempEulerAngles.z);

                    // tempEulerAngles = easyMath_QuaternionToEulerAngles(e->T.Q);
                        

                    // float tempValue = easyMath_radiansToDegrees(e->rotation);

                    // easyEditor_pushFloat1(appInfo->editor, "Rotation: ", &tempValue);

                    // e->rotation = easyMath_degreesToRadians(tempValue);

                    ////////////////////////////////////////////////////////////////////

                    easyEditor_pushColor(appInfo->editor, "Color: ", &e->colorTint);
                    easyEditor_pushInt1(appInfo->editor, "MaxHealth: ", &e->maxHealth);
                    easyEditor_pushFloat1(appInfo->editor, "MaxStamina: ", &e->maxStamina);


                    if(e->collider) {
                        easyEditor_pushFloat2(appInfo->editor, "Collider: ", &e->collider->dim2f.x, &e->collider->dim2f.y);    
                    }

                    if(e->collider1) {
                        easyEditor_pushFloat2(appInfo->editor, "Collider 2: ", &e->collider1->dim2f.x, &e->collider1->dim2f.y);
                    }

                    if(e->type == ENTITY_SIGN) {
                        e->message = easyEditor_pushTextBox(appInfo->editor, "Message: ", e->message);
                    }
                    

                    // easyEditor_pushInt1(appInfo->editor, "Health: ", &e->health);
                    // easyEditor_pushInt1(appInfo->editor, "Max Health: ", &e->maxHealth);

                    //wasPressed(appInfo->keyStates.gameButtons, BUTTON_DELETE) || wasPressed(appInfo->keyStates.gameButtons, BUTTON_BACKSPACE)
                    if(e->type != ENTITY_WIZARD && (easyEditor_pushButton(appInfo->editor, "Delete Entity"))) {
                        ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteForFrame);
                        int *indexToAdd = (int *)arrayInfo.elm;
                        *indexToAdd = editorState->entityIndex;
                        if(editorState->entitySelected == gameState->currentTerrainEntity) {
                            gameState->currentTerrainEntity = 0;
                        }
                        editorState->entitySelected = 0;
                    }

                    if(easyEditor_pushButton(appInfo->editor, "Let Go")) {
                        editorState->entitySelected = 0;
                    }

                    if(easyEditor_pushButton(appInfo->editor, "Duplicate")) {
                        Entity *e  = (Entity *)editorState->entitySelected;
                        Entity *newEntity = 0;
                        V3 position = v3_plus(e->T.pos, v3(1, 1, 0));
                        Texture *splatTexture = e->sprite;

                        bool colliderSet = e->collider;

                        switch(e->type) {
                            case ENTITY_SCENERY: {
                                if(e->model) {
                                    newEntity = init3dModel(gameState, manager, position, e->model);
                                } else {
                                    if(e->subEntityType & ENTITY_SUB_TYPE_TORCH) {
                                        newEntity = initTorch(gameState, manager, position);
                                    } else if(e->subEntityType & ENTITY_SUB_TYPE_ONE_WAY_UP_PLATFORM) {
                                        newEntity = initOneWayPlatform(gameState, manager, position, splatTexture);
                                    } else {
                                        if(colliderSet) {
                                            newEntity = initScenery_withRigidBody(gameState, manager, position, splatTexture);
                                        } else {
                                            newEntity = initScenery_noRigidBody(gameState, manager, position, splatTexture);    
                                        }   
                                    }
                                }
                                
                            } break;
                            case ENTITY_PLAYER_PROJECTILE: {
                                // assert(false);
                                easyFlashText_addText(&globalFlashTextManager, "Can't copy");
                            } break;
                            case ENTITY_SKELETON: {
                                newEntity = initSkeleton(gameState, manager, position);
                            } break;
                            case ENTITY_HEALTH_POTION_1: {
                                // assert(false);
                                easyFlashText_addText(&globalFlashTextManager, "Can't copy");
                            } break;
                            case ENITY_AUDIO_CHECKPOINT: {
                                newEntity = initAudioCheckPoint(gameState, manager, position);
                                newEntity->audioFile = e->audioFile;
                            } break;
                            case ENITY_CHECKPOINT: {
                                newEntity = initCheckPoint(gameState, manager, position);
                            } break;
                            default: {
                                easyFlashText_addText(&globalFlashTextManager, "Can't duplicate yet, please add in editor");
                            }
                        }

                        if(newEntity) {
                            if(newEntity->collider) {
                                newEntity->collider->dim2f = e->collider->dim2f; 
                            }


                            if(newEntity->collider1) {
                                newEntity->collider1->dim2f = e->collider1->dim2f;   
                            }

                            newEntity->maxHealth = e->maxHealth;
                            newEntity->T.Q = e->T.Q;
                            newEntity->T.scale = e->T.scale;
                            newEntity->colorTint = e->colorTint;
                        }

                    }

                    easyEditor_endWindow(appInfo->editor); //might not actuall need this

                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && !appInfo->editor->isHovering && !insideEntity && editorState->gizmoSelect == EDITOR_GIZMO_NONE) {
                        editorState->entitySelected = 0;
                    }

                    //Update the entity moving
                    if(editorState->entitySelected && isDown(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && !appInfo->editor->isHovering && editorState->gizmoSelect == EDITOR_GIZMO_NONE) {
                        Entity *e = ((Entity *)editorState->entitySelected);
                            
                        EasyPlane floor = {};

                        floor.origin = v3(0, 0, 0);
                        floor.normal = v3(0, 0, -1);

                        V3 hitP;
                        float tAt;

                        if(easyMath_castRayAgainstPlane(mouseP_worldRay, floor, &hitP, &tAt)) {
                            
                            hitP.z = 0;

                            e->T.pos.xy = v3_plus(hitP, editorState->grabOffset).xy;

                            //NOTE: Lock entities to integer coordinates - grid base gameplay
                            if(DEBUG_LOCK_POSITION_TO_GRID) {
                                e->T.pos.xy = roundToGridBoard(e->T.pos).xy;
                            }
                        }
                    }


                }

                
            }


            ////////////////////////////////////////////////////////////////////

            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            

            
            //NOTE(ollie): Make sure the transition is on top
            renderClearDepthBuffer(mainFrameBuffer.bufferId);

            FrameBuffer *endBuffer = &mainFrameBuffer;

            ////////////////// Draw the Pause menu //////////////////////
            if(gameState->gameModeType == GAME_MODE_PAUSE_MENU) {
                //Make sure game is paused
                gameState->gameIsPaused = true;

                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;

                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);

                EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);


                float fuaxWidth = 1920.0f;
                float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

                setViewTransform(globalRenderGroup, mat4());
                setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxHeight));


                char *pauseMenuItems[] = { "Continue", "Settings", "Exit" };

                float yT = fuaxHeight / (float)(arrayCount(pauseMenuItems) + 1);

                float fonty = yT;


                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_DOWN)) {
                    gameState->currentMenuIndex++;

                   if(gameState->currentMenuIndex >= arrayCount(pauseMenuItems)) {
                       gameState->currentMenuIndex = arrayCount(pauseMenuItems) - 1;
                   }
                }

                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_UP)) {
                    gameState->currentMenuIndex--;

                    if(gameState->currentMenuIndex <= 0) {
                        gameState->currentMenuIndex = 0;
                    }
                }

                for(int i = 0; i < arrayCount(pauseMenuItems); ++i) {
                    char *title = pauseMenuItems[i];

                    V4 color = COLOR_WHITE;
                    if(gameState->currentMenuIndex == i) {
                        color = COLOR_GOLD;

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_ENTER)) {
                            if(gameState->currentMenuIndex == 0) {
                                gameState->gameModeType = GAME_MODE_PLAY;
                                gameState->gameIsPaused = false;
                            }
                        }
                    }

                    //Draw the text
                    V2 size = getBounds(title, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), globalDebugFont, 2, gameState->fuaxResolution, 1);
                     
                    float fontx = -0.5f*size.x + 0.5f*fuaxWidth; 
                    

                    

                    outputTextNoBacking(globalDebugFont, fontx, fonty, 0.1f, gameState->fuaxResolution, title, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), color, 2, true, 1);
                    /////////////////////////

                    fonty += yT;
                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

                renderClearDepthBuffer(mainFrameBuffer.bufferId);

                easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);  

                fonty += yT;       
            }

            /////////////// Draw the text ///////////////////

            if(gameState->gameModeType == GAME_MODE_READING_TEXT) {

                //Make sure game is paused
                gameState->gameIsPaused = true;
                //DRAW THE PLAYER HUD

                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;

                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);


               EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);


               float fuaxWidth = 1920.0f;
               float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

               setViewTransform(globalRenderGroup, mat4());
               setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxHeight));

               float textBg_height = 0.4f*fuaxHeight;

               float textureY = -0.5f*fuaxHeight + 0.5f*textBg_height;

               //Stamina points backing
               Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth, textBg_height, 0)), v3(0, textureY, 0.4f));
               setModelTransform(globalRenderGroup, T);
               renderDrawSprite(globalRenderGroup, fadeBlackTexture, COLOR_WHITE);

               

               V2 size = getBounds(gameState->currentTalkText, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), globalDebugFont, 2, gameState->fuaxResolution, 1);
                

               float fontx = -0.5f*size.x + 0.5f*fuaxWidth; 
               float fonty = fuaxHeight - 0.3f*textBg_height;

               outputTextNoBacking(globalDebugFont, fontx, fonty, 0.1f, gameState->fuaxResolution, gameState->currentTalkText, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 2, true, 1);


               drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

               renderClearDepthBuffer(mainFrameBuffer.bufferId);

               easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

            }
            ///////////////////////////

            
            if(gameState->isLookingAtItems) {

                gameState->gameIsPaused = true;
                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;

                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);

                setViewTransform(globalRenderGroup, mat4());

                V2 size = getBounds("ITEMS", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), globalDebugFont, 2, gameState->fuaxResolution, 1);
                outputTextNoBacking(globalDebugFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 100, 0.1f, gameState->fuaxResolution, "ITEMS", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 2, true, 1);
                
                float fuaxWidth = 1920.0f;
                float fuaxHeight = appInfo->aspectRatio_yOverX*fuaxWidth;
                
                setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

                //Update animation timer
                // gameState->lookingAt_animTimer.current = lerp(gameState->lookingAt_animTimer.current, 20.0f*clamp01(appInfo->dt), gameState->lookingAt_animTimer.target);
                ////


                //DRAW BACKGROUND
                Texture *t = findTextureAsset("inventory_backing.png");
                                        
                Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth, fuaxHeight, 0)), v3(0, 0, 0.4f));
                
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);
                //////////////////////////////////////////////////////////////
                t = findTextureAsset("inventory_spot.png");
                Texture *hover_t = findTextureAsset("targeting2.png");
                float xAt = 0;
                float yAt = 0;

                float spacing = 180;
                float circleSize = 150;

                for(int i = 1; i < 13; ++i) {   

                    Texture *t_todraw = t;

                    if(gameState->indexInItems == (i - 1)) {
                        T = Matrix4_translate(Matrix4_scale(mat4(), v3(1.3f*circleSize, 1.3f*circleSize, 0)), v3(0.1f*fuaxWidth + xAt, -0.15f*fuaxHeight + yAt, 0.3f));
                                            
                        setModelTransform(globalRenderGroup, T);
                        renderDrawSprite(globalRenderGroup, hover_t, COLOR_WHITE);

                        t_todraw = findTextureAsset("inventory_spot1.png");
                    }


                    T = Matrix4_translate(Matrix4_scale(mat4(), v3(circleSize, circleSize, 0)), v3(0.1f*fuaxWidth + xAt, -0.15f*fuaxHeight + yAt, 0.4f));
                    
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, t_todraw, COLOR_WHITE);

                    

                    xAt += spacing;

                    if((i % 4) == 0) {
                        yAt += spacing;
                        xAt = 0;
                    }
    
                }
                
                /////////////////////////////////////////////////

                //LEFT SIDE OF INVENTORY
                t = findTextureAsset("inventory_marker.png");
                xAt = 0;
                yAt = 0;

                
                for(int i = 1; i < 9; ++i) {

                    float xspacing = 120;
                    float yspacing = 150;
                    circleSize = 20;

                    if(i == 7) { t = findTextureAsset("inventory_sword.png"); circleSize = 100; }
                    if(i == 8) { t = findTextureAsset("inventory_shield.png"); circleSize = 100; }

                    T = Matrix4_translate(Matrix4_scale(mat4(), v3(circleSize, circleSize, 0)), v3(-0.4f*fuaxWidth + xAt, -0.2f*fuaxHeight + yAt, 0.4f));
                    
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);

                    xAt += xspacing;

                    if((i % 2) == 0) {
                        yAt += yspacing;
                        xAt = 0;
                    }
                
                }

                ////////////////////////////////////////////////////////////////////////////
                
                

                //// Code from Drawing the inventory as a circle //////////////////////// 

                // Matrix4 T_ = Matrix4_scale(mat4(), v3(2, 2, 0));
                // Matrix4 T_1 = Matrix4_scale(mat4(), v3(0.8f, 0.8f, 0));

                
                // float angle = PI32/2.0f;
                // float angleUpdate = 2*PI32 / arrayCount(manager->player->itemSpots);
                // float radius = gameState->lookingAt_animTimer.current*3.5f;
                // for(int i = 0; i < arrayCount(manager->player->itemSpots); ++i) {

                //     V2 pos = v2(radius*cos(angle), radius*sin(angle));

                //     gameState->animationItemTimers[i].current = lerp(gameState->animationItemTimers[i].current, 8*clamp01(appInfo->dt), gameState->animationItemTimers[i].target);                    

                //     float growthSize = gameState->animationItemTimers[i].current;

                //     if(manager->player->itemSpots[i] != ENTITY_NULL) {
                //         Texture *t = getInvetoryTexture(manager->player->itemSpots[i]);
                        
                //         Matrix4 T = Matrix4_translate(Matrix4_scale(T_1, v3(growthSize, growthSize, 0)), v3(pos.x, pos.y, 0.4f));
                        
                //         setModelTransform(globalRenderGroup, T);
                //         renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);
                //     }


                //     Matrix4 T = Matrix4_translate(Matrix4_scale(T_, v3(growthSize, growthSize, 0)), v3(pos.x, pos.y, 0.5f));
                //     setModelTransform(globalRenderGroup, T);
                //     renderDrawSprite(globalRenderGroup, t_square, lightBrownColor);


                //     if(i == gameState->indexInItems) {
                //         Matrix4 T = Matrix4_translate(T_, v3(pos.x, pos.y, 0.6f));
                //         setModelTransform(globalRenderGroup, T);
                //         // renderDrawSprite(globalRenderGroup, particleImage, COLOR_GOLD);

                //         if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_Z)) {
                //             if(manager->player->itemSpots[i] != ENTITY_NULL) {
                //                 //NOTE: Equip item sound
                //                 playGameSound(&globalLongTermArena, gameState->equipItemSound, 0, AUDIO_BACKGROUND);

                //                 EntityType tempType = gameState->playerHolding[0];

                //                 gameState->playerHolding[0] = manager->player->itemSpots[i];


                //                 manager->player->itemSpots[i] = tempType;

                //                 gameState->animationItemTimersHUD[0] = 0.0f;

                //                 //NOTE: This was animating the transfer of the item to the HUD spot, but didn't seen necessary atm?
                //                 // assert(gameState->itemAnimationCount < arrayCount(gameState->item_animations)); 
                //                 // gameState->item_animations[gameState->itemAnimationCount++] = items_initItemAnimation(v2_scale(0.5f, gameState->fuaxResolution), itemPosition1.xy, gameState->playerHolding[0]);

                //                 // assert(gameState->itemAnimationCount < arrayCount(gameState->item_animations)); 
                //                 // gameState->item_animations[gameState->itemAnimationCount++] = items_initItemAnimation(itemPosition1.xy, v2_scale(0.5f, gameState->fuaxResolution), player->itemSpots[i]);
                //             }
                //         }

                //         if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_X)) {
                //             if(manager->player->itemSpots[i] != ENTITY_NULL) {
                //                 //NOTE: Equip item sound
                //                 playGameSound(&globalLongTermArena, gameState->equipItemSound, 0, AUDIO_BACKGROUND);

                //                 EntityType tempType = gameState->playerHolding[1];

                //                 gameState->playerHolding[1] = manager->player->itemSpots[i];


                //                 manager->player->itemSpots[i] = tempType;

                //                 gameState->animationItemTimersHUD[1] = 0.0f;

                //             }
                //         }
                    // }

                    // angle += angleUpdate; 
                // }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            } else if(gameState->gameModeType == GAME_MODE_GAME_OVER) {
                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;
                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);

                setViewTransform(globalRenderGroup, cameraLookAt_straight);


                V2 size = getBounds("GAME OVER", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), globalDebugFont, 2, gameState->fuaxResolution, 1);
                outputTextNoBacking(globalDebugFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 0.5*gameState->fuaxResolution.y, 0.1f, gameState->fuaxResolution, "GAME OVER", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 2, true, 1);
 
                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_ENTER)) {
                    gameState->gameModeType = GAME_MODE_PLAY;
                    manager->player->health = manager->player->maxHealth;
                }
            }

             //NOTE: Draw what the player is holding
            {

                Matrix4 T_1 = Matrix4_scale(mat4(), v3(100, 100, 0));
                Matrix4 item_T = Matrix4_scale(mat4(), v3(40, 40, 0));

                setViewTransform(globalRenderGroup, mat4());
                setProjectionTransform(globalRenderGroup, gameState->orthoFuaxMatrix);

                for(int i = 0; i < gameState->itemAnimationCount; ) {

                    int increment = 1;
                    ItemAnimationInfo *anim = &gameState->item_animations[i];
                    assert(anim->tAt >= 0);

                    anim->tAt += appInfo->dt;

                    float canVal = anim->tAt / 0.3f;

                    V2 P = lerpV2(anim->startP, canVal, anim->endP);

                    Texture *t = getInvetoryTexture(anim->type);
                    Matrix4 T = Matrix4_translate(item_T, v3(P.x, P.y, 0.1f));
                    
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);


                    if(canVal >= 1.0f) {
                        increment = 0;
                        anim->tAt = -1;
                        gameState->item_animations[i] = gameState->item_animations[--gameState->itemAnimationCount];
                    }

                    i += increment;
                }
                

                //Update the HUD item spots animations
                float canVal0 = 1;
                float canVal1 = 1;
                if(gameState->animationItemTimersHUD[0] >= 0.0f) { gameState->animationItemTimersHUD[0] += appInfo->dt; canVal0 = gameState->animationItemTimersHUD[0]/0.5f; if(canVal0 >= 1.0f) { gameState->animationItemTimersHUD[0] = -1.0f; } canVal0 = smoothStep00(1, canVal0, 2.5f); }
                if(gameState->animationItemTimersHUD[1] >= 0.0f) { gameState->animationItemTimersHUD[1] += appInfo->dt; canVal1 = gameState->animationItemTimersHUD[1]/0.5f; if(canVal1 >= 1.0f) { gameState->animationItemTimersHUD[1] = -1.0f; } canVal1 = smoothStep00(1, canVal1, 2.5f); }
                    
                float itemScale = 1.4f;

                float xOffset = 8;
                float yOffset = 8;
                setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(T_1, v3(canVal0, canVal0, 0)), itemPosition1));
                renderDrawSprite(globalRenderGroup, t_square, COLOR_WHITE);

                outputTextNoBacking(globalDebugFont, itemPosition1.x - 40, gameState->fuaxResolution.y - itemPosition1.y + 40, 0.1f, gameState->fuaxResolution, "Z", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_WHITE, 1, true, 1);
                
                if(gameState->playerHolding[0] != ENTITY_NULL) {

                    setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(item_T, v3(itemScale*canVal0, itemScale*canVal0, 0)), v3(itemPosition1.x + xOffset, itemPosition1.y + yOffset, 0.1f)));

                    Texture *t = getInvetoryTexture(gameState->playerHolding[0]);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);
                }

                setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(T_1, v3(canVal1, canVal1, 0)), itemPosition2));
                renderDrawSprite(globalRenderGroup, t_square, COLOR_WHITE);

                outputTextNoBacking(globalDebugFont, itemPosition2.x - 40, gameState->fuaxResolution.y - itemPosition2.y + 40, 0.1f, gameState->fuaxResolution, "X", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_WHITE, 1, true, 1);

                if(gameState->playerHolding[1] != ENTITY_NULL) {
                    setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(item_T, v3(itemScale*canVal1, itemScale*canVal1, 0)), v3(itemPosition2.x + xOffset, itemPosition2.y + yOffset, 0.1f)));

                    Texture *t = getInvetoryTexture(gameState->playerHolding[1]);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);

                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
            }
            ///////
            
            //NOTE(ollie): Update the console
            if(easyConsole_update(&appInfo->console, &consoleKeyStates, appInfo->dt, (resolution.y / resolution.x))) {
                EasyToken token = easyConsole_getNextToken(&appInfo->console);
                
                bool noMatch = true;
                if(token.type == TOKEN_WORD) {
                    bool responseYes = (token.size == 1 && stringsMatchNullN("y", token.at, token.size)) || (token.size == 3 && stringsMatchNullN("yes", token.at, token.size));
                    

                    if(appInfo->console.askingQuestion  && responseYes) {
                        appInfo->console.askingQuestion = false;
                         noMatch = false;

                        if(appInfo->console.questionId == 1) { //Save Scene Id
                            gameScene_saveScene(manager, gameState->sceneFileNameTryingToSave);
                            easyFlashText_addText(&globalFlashTextManager, "SAVED");

                            easyPlatform_freeMemory(gameState->sceneFileNameTryingToSave);
                            gameState->sceneFileNameTryingToSave = 0;
                        }

                    } 
                    else if(appInfo->console.askingQuestion && ((token.size == 1 && stringsMatchNullN("n", token.at, token.size)) || (token.size == 2 && stringsMatchNullN("no", token.at, token.size))))  {
                        appInfo->console.askingQuestion = false;
                         noMatch = false;
                    } else {
                        appInfo->console.askingQuestion = false;
                    } 

                    if(gameState->sceneFileNameTryingToSave) {
                        easyPlatform_freeMemory(gameState->sceneFileNameTryingToSave);
                    }
                            
                    if(stringsMatchNullN("save", token.at, token.size)) {
                        noMatch = false;
                        
                        token = easyConsole_getNextToken(&appInfo->console);

                        if(token.type == TOKEN_WORD) {
                            char *sceneFileName = nullTerminate(token.at, token.size);

                            if(gameScene_doesSceneExist(sceneFileName)) {
                                easyConsole_addToStream(DEBUG_globalEasyConsole, "This will override a scene. Are you sure?");
                                easyConsole_addToStream(DEBUG_globalEasyConsole, easy_createString_printf(&globalPerFrameArena, "Current scene name is %s", gameState->currentSceneName));
                                appInfo->console.askingQuestion = true;
                                appInfo->console.questionId = 1;
                                gameState->sceneFileNameTryingToSave = sceneFileName;
                            } else {
                                gameScene_saveScene(manager, sceneFileName);
                                easyFlashText_addText(&globalFlashTextManager, "SAVED");   
                            }
                            
                        } else {
                            easyConsole_addToStream(DEBUG_globalEasyConsole, "Please pass a scene name to load");
                        }
                        

                    } else if(stringsMatchNullN("load", token.at, token.size)) {
                        noMatch = false;
                        
                        token = easyConsole_getNextToken(&appInfo->console);

                        if(token.type == TOKEN_WORD) {
                            char *sceneFileName = nullTerminateArena(token.at, token.size, &globalPerFrameArena);

                            if(gameScene_doesSceneExist(sceneFileName)) {
                                entityManager_emptyEntityManager(manager, &gameState->physicsWorld);
                                editorState->entitySelected = 0;
                                //NOTE: MEMEORY LEAK HERE. We don' free the scene file name when we load a new scene
                                gameScene_loadScene(gameState, manager, nullTerminateArena(token.at, token.size, &globalLongTermArena));
                            } else {
                                easyConsole_addToStream(DEBUG_globalEasyConsole, "No scene to load");    
                            }
                        } else {
                            
                        }
                        

                    } else if(stringsMatchNullN("camMove", token.at, token.size)) {
                        noMatch = false;
                        if(canCameraMove == (int)EASY_CAMERA_MOVE) {
                            canCameraMove = 0;
                        } else {
                            canCameraMove = (int)EASY_CAMERA_MOVE;
                        }
                    } else if(stringsMatchNullN("camRotate", token.at, token.size)) {
                        noMatch = false;
                        if(canCamRotate == (int)EASY_CAMERA_ROTATE) {
                            canCamRotate = 0;
                        } else {
                            canCamRotate = (int)EASY_CAMERA_ROTATE;
                        }
                    } else if(stringsMatchNullN("help", token.at, token.size)) {
                        noMatch = false;

                        easyConsole_addToStream(&appInfo->console, "camMove - toggle camera moving\ncamRotate - toggle camera rotating");
                    } else if(stringsMatchNullN("collisionBounds", token.at, token.size)) {
                        noMatch = false;

                        DEBUG_DRAW_COLLISION_BOUNDS = !DEBUG_DRAW_COLLISION_BOUNDS;
                        char str[256];
                        sprintf(str, "Collision Bounds: %d", DEBUG_DRAW_COLLISION_BOUNDS);
                        easyConsole_addToStream(&appInfo->console, str);
                    }
                } 

                if(noMatch) {
                    easyConsole_parseDefault(&appInfo->console, token);
                }
            }

            //////////////////////////////////////////////////////////////////////////////////////////////

            easyOS_endFrame(resolution, screenDim, endBuffer->bufferId, appInfo, appInfo->hasBlackBars);
            DEBUG_TIME_BLOCK_FOR_FRAME_END(beginFrame, wasPressed(appInfo->keyStates.gameButtons, BUTTON_F4))
            DEBUG_TIME_BLOCK_FOR_FRAME_START(beginFrame, "Per frame")
            easyOS_endKeyState(&appInfo->keyStates);

            if(recording) {
                timeSinceLastFrame += appInfo->dt;

                if((timeSinceLastFrame * 100.0f) >= centisecondsPerFrame) {
                    timeSinceLastFrame = 0;
                    u8 *stream = (u8 *)easyPlatform_allocateMemory(resolution.x*resolution.y*sizeof(u32), EASY_PLATFORM_MEMORY_NONE);

                    renderReadPixels(endBuffer->bufferId, 0, 0, resolution.x, resolution.y, 
                                          (u32)GL_RGBA,
                                          (u32)GL_UNSIGNED_BYTE,
                                          stream);

                    // u8 *output_pixels = (u8 *)easyPlatform_allocateMemory(gif_width*gif_height*4, EASY_PLATFORM_MEMORY_NONE);

                    // stbir_resize_uint8(stream, resolution.x, resolution.y, 0, output_pixels, gif_width, gif_height, 0, 4);
                                                   
                    msf_gif_frame(&gifState, stream, centisecondsPerFrame, bitDepth, -gif_width * sizeof(u32)); //frame 1  

                    easyPlatform_freeMemory(stream); 
                    // easyPlatform_freeMemory(output_pixels); 
                } 
            }
            
        }
        easyOS_endProgram(appInfo);
    }
    return(0);
}
