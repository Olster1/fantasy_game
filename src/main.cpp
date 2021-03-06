#include "defines.h"
#include "easy_headers.h"

static bool DEBUG_DRAW_SCENERY_TEXTURES = true;
static bool DEBUG_GRAVITY = false;

#include "gameState.h"
#include "gameScene.c"
#include "editor.h"
#include "entity.c"
#include "saveload.c"


#define MSF_GIF_IMPL
#include "msf_gif.h"

// #define STB_IMAGE_RESIZE_IMPLEMENTATION
// #include "stb_resize.h"

#define GIF_MODE 0


static Texture *getInvetoryTexture(EntityType type) {
    Texture *t = 0;
    switch(type) {
        case ENTITY_HEALTH_POTION_1: {
            t = findTextureAsset("blue_jar.png");
            // assert(false);
        } break;
        default: {

        }
    }
    return t;
}

static bool DEBUG_DRAW_COLLISION_BOUNDS = false; 


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
        int gif_width = resolution.x, gif_height = resolution.y, centisecondsPerFrame = 5, bitDepth = 16;
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

            easyAnimation_initAnimation(&gameState->wizardIdle, "wizardIdle");
            easyAnimation_pushFrame(&gameState->wizardIdle, "player.png");
            // loadAndAddImagesStripToAssets(&gameState->wizardIdle, "img/fantasy_sprites/wizard/Idle.png", 231);

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
        Texture *t_square = findTextureAsset("square_img.png");
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



        #define LOAD_SCENE_FROM_FILE 1
        #if LOAD_SCENE_FROM_FILE
                gameScene_loadScene(gameState, manager, "custom");
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


        EasySound_LoopSound(playGameSound(&globalLongTermArena, easyAudio_findSound("GrandQuest.wav"), 0, AUDIO_BACKGROUND));

        while(appInfo->running) {

            if(refreshTweakFile(tweakerFileName, tweaker)) {
                gameState->jumpPower = getIntFromTweakData(tweaker, "jumpPower");
                gameState->walkPower = getIntFromTweakData(tweaker, "walkPower");
                gameState->gravityScale = (float)getFloatFromTweakData(tweaker, "gravityScale");
                gameState->cameraSnapDistance = (float)getFloatFromTweakData(tweaker, "cameraSnapDistance");

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
            V3 worldP = easyTransform_getWorldPos(&manager->player->T);
            {//update x position
                float distance = absVal(worldP.x - camera.hidden_pos.x);

                if(distance > gameState->cameraSnapDistance) {
                    float newPosX = lerp(camera.hidden_pos.x, clamp01(appInfo->dt*2.f), manager->player->T.pos.x);
                    camera.hidden_pos.x = newPosX;
                }
            }

            {//update y position
                float distance = absVal(worldP.y - camera.hidden_pos.y);

                if(distance > gameState->cameraSnapDistance) {
                    float newPosY = lerp(camera.hidden_pos.y, clamp01(appInfo->dt*2.f), manager->player->T.pos.y);
                    camera.hidden_pos.y = newPosY;
                }
            }
            ////////////////

            easy3d_updateCamera(&camera, &appInfo->keyStates, 1, 1000.0f, appInfo->dt, camMove);


            easy_setEyePosition(globalRenderGroup, camera.pos);
            
            // update camera first
            Matrix4 viewMatrix = easy3d_getWorldToView(&camera);
            Matrix4 perspectiveMatrix = projectionMatrixFOV(camera.zoom, resolution.x/resolution.y);
            Matrix4 perspectiveMatrix_inventory = projectionMatrixFOV(60.0f, resolution.x/resolution.y);

            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_I)) {
                gameState->isLookingAtItems = !gameState->isLookingAtItems;
                gameState->lookingAt_animTimer.current = UI_ITEM_RADIUS_MIN;
                gameState->lookingAt_animTimer.target = UI_ITEM_RADIUS_MAX;

                gameState->indexInItems = 0;
                gameState->animationItemTimers[0].target = UI_ITEM_PICKER_MAX_SIZE;

                //Stop all the other animation timers
                for(int i = 1; i < arrayCount(gameState->animationItemTimers); ++i) {
                    gameState->animationItemTimers[i].target = UI_ITEM_PICKER_MIN_SIZE;
                    gameState->animationItemTimers[i].current = UI_ITEM_PICKER_MIN_SIZE;
                }
                //Open  close menu sound
                playGameSound(&globalLongTermArena, gameState->openMenuSound, 0, AUDIO_BACKGROUND);
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
                        gameState->indexInItems = arrayCount(manager->player->itemSpots) - 1;
                    }

                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                }

                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT)) {
                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                    gameState->indexInItems++;
                    playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
                    if(gameState->indexInItems >= arrayCount(manager->player->itemSpots)) {
                        gameState->indexInItems = 0;
                    }

                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                }
            } else 
            {
                EasyPhysics_UpdateWorld(&gameState->physicsWorld, appInfo->dt);    
            }



            RenderProgram *mainShader = &glossProgram;
            renderSetShader(globalRenderGroup, mainShader);
            setViewTransform(globalRenderGroup, viewMatrix);
            setProjectionTransform(globalRenderGroup, perspectiveMatrix);


            AppKeyStates gameKeyStates = appInfo->keyStates;
            AppKeyStates consoleKeyStates = appInfo->keyStates;
            if(appInfo->console.isInFocus) {
                gameKeyStates = {};
            } else if(easyConsole_isOpen(&appInfo->console)) {
                consoleKeyStates = {};
                consoleKeyStates.gameButtons[BUTTON_ESCAPE] = gameKeyStates.gameButtons[BUTTON_ESCAPE];
            }

             Entity *insideEntity = 0;
            //DEBUG
            // if(true)
            {
                

                
               
                for(int i = 0; i < manager->entities.count; ++i) {
                    Entity *e = (Entity *)getElement(&manager->entities, i);
                    if(e) {

                        if(gameState->isEditorOpen) {
                            V3 mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, getEntityZLayerPos(e) - camera.pos.z, easy3d_getViewToWorld(&camera));

                            //Grab Entity
                            
                            V2 entP = easyTransform_getWorldPos(&e->T).xy;
                            V2 entScale = easyTransform_getWorldScale(&e->T).xy;
                            Rect2f entBounds = rect2fCenterDim(entP.x, entP.y, entScale.x, entScale.y);
                            if(inBounds(mouseP_inWorldP.xy, entBounds, BOUNDS_RECT)) {
                                insideEntity = e;

                                if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                                    editorState->entitySelected = e;
                                    editorState->entityIndex = i;
                                    editorState->grabOffset = v2_minus(entP, mouseP_inWorldP.xy);

                                }    
                            }

                        }


                        if(e->collider) {   
                            e->T.pos.z = getEntityZLayerPos(e);
                            V3 prevScale = e->T.scale;

                            e->T.scale.x *= e->collider->dim2f.x;
                            e->T.scale.y *= e->collider->dim2f.y;

                            Matrix4 T = easyTransform_getTransform(&e->T);

                            V2 dim = e->collider->dim2f;

                            V4 color = COLOR_RED;

                            if(e->collider->isTrigger) {
                                color = COLOR_YELLOW;
                            } 
                            
                        

                            setModelTransform(globalRenderGroup, T);
                            if(DEBUG_DRAW_COLLISION_BOUNDS) {
                                renderDrawQuad(globalRenderGroup, color);    
                            }
                            
                            e->T.pos.z = 0;

                            e->T.scale = prevScale;
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
           

            for(int i = 0; i < manager->entities.count; ++i) {
                Entity *e = (Entity *)getElement(&manager->entities, i);
                if(e) {
                    updateEntity(manager, e, gameState, appInfo->dt, &gameKeyStates, &appInfo->console, &camera, manager->player, gameState->isLookingAtItems);        
                
                    if(e->isDead) {
                        ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteForFrame);
                        int *indexToAdd = (int *)arrayInfo.elm;
                        *indexToAdd = i;
                    }
                }
                
            }

                       
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
                        } 

                        Entity *e1 = initEntity(manager, &gameState->firePitAnimation, e->position, size, v2(0.9f, 0.9f), gameState, e->type, inverse_weight, t, COLOR_WHITE, layer, true);
                        e1->rb->dP = e->dP;
                        
                        e1->lifeSpanLeft = lifeSpan;
                        e1->rb->reboundFactor = reboundFactor;
                    }
                }
            }

            easyArray_clear(&manager->entitiesToAddForFrame);

            { //NOTE: Remove entities that need deleting
                for(int i = 0; i < manager->entitiesToDeleteForFrame.count; ++i) {
                    int *indexToDelete = (int *)getElement(&manager->entitiesToDeleteForFrame, i);
                    if(indexToDelete) {
                        Entity *e = (Entity *)getElement(&manager->entities, *indexToDelete);

                        EasyPhysics_removeCollider(&gameState->physicsWorld, e->collider);
                        EasyPhysics_removeRigidBody(&gameState->physicsWorld, e->rb);

                        removeElement_ordered(&manager->entities, *indexToDelete);


                    }
                }

                easyArray_clear(&manager->entitiesToDeleteForFrame);
            }   


            if(!appInfo->console.isInFocus && wasPressed(appInfo->keyStates.gameButtons, BUTTON_F5)) {
                gameState->isEditorOpen = !gameState->isEditorOpen;
            }

                
            if(gameState->isEditorOpen){
                easyEditor_startDockedWindow(appInfo->editor, "Editor Window", EASY_EDITOR_DOCK_TOP_RIGHT);
                // easyEditor_startWindow(appInfo->editor, "Editor Window", 300, 300);
                
                
                // easyEditor_pushFloat1(appInfo->editor, "first float: ", &v.x);
                // easyEditor_pushSlider(appInfo->editor, "value slider: ", &v.x, -10, 10);

                editorState->createMode = (EditorCreateMode)easyEditor_pushList(appInfo->editor, "Editor Mode: ", EditorCreateModesStrings, arrayCount(EditorCreateModesStrings));

                int splatIndexOn = 0;
                if(editorState->createMode == EDITOR_CREATE_SCENERY || editorState->createMode == EDITOR_CREATE_SCENERY_RIGID_BODY || editorState->createMode == EDITOR_CREATE_ONE_WAY_PLATFORM) {
                    splatIndexOn = easyEditor_pushList(appInfo->editor, "Splat Names: ", (char **)gameState->splatList.memory, gameState->splatList.count);    
                }



                

                V3 mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, -camera.pos.z, easy3d_getViewToWorld(&camera));

                bool pressed = wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE);

                Texture *splatTexture = ((Texture **)(gameState->splatTextures.memory))[splatIndexOn];

                if(!appInfo->editor->isHovering) {
                    switch(editorState->createMode) {
                        case EDITOR_CREATE_SELECT_MODE: {
                            //do nothing
                        } break;
                        case EDITOR_CREATE_TERRAIN: {
                            if(pressed) {
                                editorState->entitySelected = initTerrain(gameState, manager, mouseP_inWorldP);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                            }
                        } break;
                        case EDITOR_CREATE_SCENERY: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, mouseP_inWorldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;
                            }

                        } break;
                        case EDITOR_CREATE_ONE_WAY_PLATFORM: {
                            if(pressed) {
                                initOneWayPlatform(gameState, manager, mouseP_inWorldP, splatTexture);
                                editorState->entityIndex = manager->lastEntityIndex;
                            }
                        } break;
                        case EDITOR_CREATE_SCENERY_RIGID_BODY: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, mouseP_inWorldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, true);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;   
                        case EDITOR_CREATE_SKELETON: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, &gameState->skeltonIdle, mouseP_inWorldP, v2(2.5f, 2.5f), v2(0.25f, 0.15f), gameState, ENTITY_SKELETON, gameState->inverse_weight, 0, COLOR_WHITE, 1, true);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_CHECKPOINT: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, mouseP_inWorldP, v2(1, 1), v2(1, 1), gameState, ENITY_CHECKPOINT, 0, &globalWhiteTexture, COLOR_BLUE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_TORCH: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, &gameState->torchAnimation, mouseP_inWorldP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;

                            }
                        } break;
                        case EDITOR_CREATE_AUDIO_CHECKPOINT: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, mouseP_inWorldP, v2(1, 1), v2(1, 1), gameState, ENITY_AUDIO_CHECKPOINT, 0, &globalWhiteTexture, COLOR_BLUE, -1, false);
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
                }

                if(easyEditor_pushButton(appInfo->editor, (DEBUG_DRAW_SCENERY_TEXTURES) ? "Scenery Off" : "Scenery On")) {
                    DEBUG_DRAW_SCENERY_TEXTURES = !DEBUG_DRAW_SCENERY_TEXTURES;
                }

                if(easyEditor_pushButton(appInfo->editor, "Zero Pos Player")) {
                    manager->player->T.pos.xy = v2(0, 0);
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
                    //treat the z position as zero
                    entP.z = 0;

                    V2 pos = v2_scale(0.5f, easyTransform_getWorldScale(&e->T).xy);
                    Matrix4 rotation = easyTransform_getWorldRotation(&e->T);


                    float entityZ = getEntityZLayerPos(e) - 0.1f;

                    mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, entityZ - camera.pos.z, easy3d_getViewToWorld(&camera));

                    V3 rightTopCorner = v3(pos.x, pos.y, entityZ);
                    V3 leftTopCorner = v3(-pos.x, pos.y, entityZ);
                    V3 rightBottomCorner = v3(pos.x, -pos.y, entityZ);
                    V3 leftBottomCorner = v3(-pos.x, -pos.y, entityZ);

                    float handleSize = 0.3f;

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

                    V3 aP = v3_plus(transformPositionV3(rightTopCorner, rotation), entP);
                    Rect2f aB = rect2fCenterDim(aP.x, aP.y, handleScale*handleSize, handleScale*handleSize);
                    if(inBounds(mouseP_inWorldP.xy, aB, BOUNDS_RECT)) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_TOP_RIGHT;
                        }    
                    }

                    V3 bP = v3_plus(transformPositionV3(leftTopCorner, rotation), entP);
                    aB = rect2fCenterDim(bP.x, bP.y, handleScale*handleSize, handleScale*handleSize);
                    if(inBounds(mouseP_inWorldP.xy, aB, BOUNDS_RECT)) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_TOP_LEFT;
                        }    
                    }

                    V3 cP = v3_plus(transformPositionV3(rightBottomCorner, rotation), entP);
                    aB = rect2fCenterDim(cP.x, cP.y, handleScale*handleSize, handleScale*handleSize);
                    if(inBounds(mouseP_inWorldP.xy, aB, BOUNDS_RECT)) {
                        if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                            editorState->gizmoSelect = EDITOR_GIZMO_BOTTOM_RIGHT;
                        }    
                    }

                    V3 dP = v3_plus(transformPositionV3(leftBottomCorner, rotation), entP);
                    aB = rect2fCenterDim(dP.x, dP.y, handleScale*handleSize, handleScale*handleSize);
                    if(inBounds(mouseP_inWorldP.xy, aB, BOUNDS_RECT)) {
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
                            V3 diff = v3_minus(mouseP_inWorldP, dP);

                            V2 prevS = e->T.scale.xy;

                            e->T.scale.xy = diff.xy;
                            e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2_minus(e->T.scale.xy, prevS)));
                            
                        } break;
                        case EDITOR_GIZMO_TOP_LEFT: {

                            V3 diff = v3_minus(mouseP_inWorldP, cP);

                            V2 prevS = e->T.scale.xy;

                            V2 positiveSize = absVal_v2(diff.xy);

                            e->T.scale.xy = positiveSize;
                            e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2(prevS.x - e->T.scale.x, e->T.scale.y - prevS.y)));

                        } break;
                        case EDITOR_GIZMO_BOTTOM_LEFT: {
                            V3 diff = v3_minus(mouseP_inWorldP, aP);

                            V2 prevS = e->T.scale.xy;

                            V2 positiveSize = absVal_v2(diff.xy);

                            e->T.scale.xy = positiveSize;
                            e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2(prevS.x - e->T.scale.x, prevS.y - e->T.scale.y)));

                        } break;
                        case EDITOR_GIZMO_BOTTOM_RIGHT: {

                            V3 diff = v3_minus(mouseP_inWorldP, bP);

                            V2 prevS = e->T.scale.xy;

                            V2 positiveSize = absVal_v2(diff.xy);

                            e->T.scale.xy = positiveSize;
                            e->T.pos.xy = v2_plus(e->T.pos.xy, v2_scale(0.5f, v2(e->T.scale.x - prevS.x, prevS.y - e->T.scale.y)));

                        } break;
                    }

                    if(wasReleased(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                        editorState->gizmoSelect = EDITOR_GIZMO_NONE;   
                    }

                    //////////



                    easyEditor_startDockedWindow(appInfo->editor, "Entity Window", EASY_EDITOR_DOCK_TOP_LEFT);
                    

                    easyEditor_pushButton(appInfo->editor, MyEntity_EntityTypeStrings[(int)e->type]);
                    easyEditor_pushInt1(appInfo->editor, "Subtype: ", &(int)e->subEntityType);
                    
                    easyEditor_pushFloat3(appInfo->editor, "Position: ", &e->T.pos.x, &e->T.pos.y, &e->T.pos.z);
                    easyEditor_pushFloat3(appInfo->editor, "Scale: ", &e->T.scale.x, &e->T.scale.y, &e->T.scale.z);

                    ////////////////////////////////////////////////////////////////////
                    //NOTE(ollie): Rotation with euler angles
                    
                    V3 tempEulerAngles = easyMath_QuaternionToEulerAngles(e->T.Q);
                    
                    
                    //NOTE(ollie): Swap to degrees 
                    // tempEulerAngles.x = easyMath_radiansToDegrees(tempEulerAngles.x);
                    // tempEulerAngles.y = easyMath_radiansToDegrees(tempEulerAngles.y);
                    tempEulerAngles.z = easyMath_radiansToDegrees(tempEulerAngles.z);


                    easyEditor_pushSlider(appInfo->editor, "Rotation: ", &tempEulerAngles.z, 0, 360.0f);
                    
                    //NOTE(ollie): Convert back to radians
                    // tempEulerAngles.x = easyMath_degreesToRadians(tempEulerAngles.x);
                    // tempEulerAngles.y = easyMath_degreesToRadians(tempEulerAngles.y);
                    tempEulerAngles.z = easyMath_degreesToRadians(tempEulerAngles.z);
                    
                    e->T.Q = eulerAnglesToQuaternion(tempEulerAngles.y, tempEulerAngles.x, tempEulerAngles.z);
                    
                    ////////////////////////////////////////////////////////////////////

                    easyEditor_pushColor(appInfo->editor, "Color: ", &e->colorTint);
                    easyEditor_pushInt1(appInfo->editor, "MaxHealth: ", &e->maxHealth);

                    easyEditor_pushSlider(appInfo->editor, "Layer: ", &e->layer, -5, 5.0f);


                    if(e->collider) {
                        easyEditor_pushFloat2(appInfo->editor, "Collider: ", &e->collider->dim2f.x, &e->collider->dim2f.y);    
                    }

                    if(e->collider1) {
                        easyEditor_pushFloat2(appInfo->editor, "Collider 2: ", &e->collider1->dim2f.x, &e->collider1->dim2f.y);
                    }
                    

                    // easyEditor_pushInt1(appInfo->editor, "Health: ", &e->health);
                    // easyEditor_pushInt1(appInfo->editor, "Max Health: ", &e->maxHealth);


                    if(e->type != ENTITY_WIZARD && easyEditor_pushButton(appInfo->editor, "Delete Entity")) {
                        ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteForFrame);
                        int *indexToAdd = (int *)arrayInfo.elm;
                        *indexToAdd = editorState->entityIndex;
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

                            }
                        }

                        if(newEntity) {
                            if(newEntity->collider) {
                                newEntity->collider->dim2f = e->collider->dim2f; 
                            }


                            if(newEntity->collider1) {
                                newEntity->collider1->dim2f = e->collider1->dim2f;   
                            }

                            newEntity->layer = e->layer;
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
                        V3 mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, getEntityZLayerPos(e) - camera.pos.z, easy3d_getViewToWorld(&camera));
                        e->T.pos.xy = v2_plus(mouseP_inWorldP.xy, editorState->grabOffset);
                    }


                }

                
            }


            ////////////////////////////////////////////////////////////////////

            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));



            
            //NOTE(ollie): Make sure the transition is on top
            // renderClearDepthBuffer(mainFrameBuffer.bufferId);

            FrameBuffer *endBuffer = &mainFrameBuffer;
            if(gameState->isLookingAtItems) {

                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;



                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);

                setViewTransform(globalRenderGroup, cameraLookAt_straight);

                V2 size = getBounds("INVENTORY", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), globalDebugFont, 2, gameState->fuaxResolution, 1);
                outputTextNoBacking(globalDebugFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 100, 0.1f, gameState->fuaxResolution, "INVENTORY", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(0, 0, 0, 1), 2, true, 1);
                
                float fuaxWidth = 1920.0f;
                
                setProjectionTransform(globalRenderGroup, perspectiveMatrix_inventory);//OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

                //Update animation timer
                gameState->lookingAt_animTimer.current = lerp(gameState->lookingAt_animTimer.current, 20.0f*clamp01(appInfo->dt), gameState->lookingAt_animTimer.target);
                ////


                Matrix4 T_ = Matrix4_scale(mat4(), v3(2, 2, 0));
                Matrix4 T_1 = Matrix4_scale(mat4(), v3(0.8f, 0.8f, 0));

                
                float angle = PI32/2.0f;
                float angleUpdate = 2*PI32 / arrayCount(manager->player->itemSpots);
                float radius = gameState->lookingAt_animTimer.current*3.5f;
                for(int i = 0; i < arrayCount(manager->player->itemSpots); ++i) {

                    V2 pos = v2(radius*cos(angle), radius*sin(angle));

                    gameState->animationItemTimers[i].current = lerp(gameState->animationItemTimers[i].current, 8*clamp01(appInfo->dt), gameState->animationItemTimers[i].target);                    

                    float growthSize = gameState->animationItemTimers[i].current;

                    if(manager->player->itemSpots[i] != ENTITY_NULL) {
                        Texture *t = getInvetoryTexture(manager->player->itemSpots[i]);
                        
                        Matrix4 T = Matrix4_translate(Matrix4_scale(T_1, v3(growthSize, growthSize, 0)), v3(pos.x, pos.y, 0.4f));
                        
                        setModelTransform(globalRenderGroup, T);
                        renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);
                    }


                    Matrix4 T = Matrix4_translate(Matrix4_scale(T_, v3(growthSize, growthSize, 0)), v3(pos.x, pos.y, 0.5f));
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, t_square, lightBrownColor);


                    if(i == gameState->indexInItems) {
                        Matrix4 T = Matrix4_translate(T_, v3(pos.x, pos.y, 0.6f));
                        setModelTransform(globalRenderGroup, T);
                        // renderDrawSprite(globalRenderGroup, particleImage, COLOR_GOLD);

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_Z)) {
                            if(manager->player->itemSpots[i] != ENTITY_NULL) {
                                //NOTE: Equip item sound
                                playGameSound(&globalLongTermArena, gameState->equipItemSound, 0, AUDIO_BACKGROUND);

                                EntityType tempType = gameState->playerHolding[0];

                                gameState->playerHolding[0] = manager->player->itemSpots[i];
                                manager->player->itemSpots[i] = tempType;

                                gameState->animationItemTimersHUD[0] = 0.0f;

                                //NOTE: This was animating the transfer of the item to the HUD spot, but didn't seen necessary atm?
                                // assert(gameState->itemAnimationCount < arrayCount(gameState->item_animations)); 
                                // gameState->item_animations[gameState->itemAnimationCount++] = items_initItemAnimation(v2_scale(0.5f, gameState->fuaxResolution), itemPosition1.xy, gameState->playerHolding[0]);

                                // assert(gameState->itemAnimationCount < arrayCount(gameState->item_animations)); 
                                // gameState->item_animations[gameState->itemAnimationCount++] = items_initItemAnimation(itemPosition1.xy, v2_scale(0.5f, gameState->fuaxResolution), player->itemSpots[i]);
                            }
                        }
                    }

                    angle += angleUpdate; 
                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

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
                

                setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(T_1, v3(canVal0, canVal0, 0)), itemPosition1));
                renderDrawSprite(globalRenderGroup, t_square, lightBrownColor);

                outputTextNoBacking(globalDebugFont, itemPosition1.x + 50, gameState->fuaxResolution.y - itemPosition1.y + 40, 0.1f, gameState->fuaxResolution, "Z", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(0, 0, 0, 1), 1, true, 1);
                
                if(gameState->playerHolding[0] != ENTITY_NULL) {

                    setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(item_T, v3(canVal0, canVal0, 0)), v3(itemPosition1.x, itemPosition1.y, 0.1f)));

                    Texture *t = getInvetoryTexture(gameState->playerHolding[0]);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);
                }

                setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(T_1, v3(canVal1, canVal1, 0)), itemPosition2));
                renderDrawSprite(globalRenderGroup, t_square, lightBrownColor);

                outputTextNoBacking(globalDebugFont, itemPosition2.x + 50, gameState->fuaxResolution.y - itemPosition2.y + 40, 0.1f, gameState->fuaxResolution, "X", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(0, 0, 0, 1), 1, true, 1);

                if(gameState->playerHolding[1] != ENTITY_NULL) {
                    setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(item_T, v3(canVal1, canVal1, 0)), v3(itemPosition2.x, itemPosition2.y, 0.1f)));

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
