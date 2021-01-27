#include "defines.h"
#include "easy_headers.h"


#include "gameState.h"
#include "gameScene.c"
#include "entity.c"

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

int main(int argc, char *args[]) {
    
    EASY_ENGINE_ENTRY_SETUP()
    
    V2 screenDim = v2(DEFINES_WINDOW_SIZE_X, DEFINES_WINDOW_SIZE_Y); //init in create app function
    V2 resolution = v2(DEFINES_RESOLUTION_X, DEFINES_RESOLUTION_Y);

    #if GIF_MODE
    resolution.x = 480;
    resolution.y = 320;
    #endif
    
    OSAppInfo *appInfo = easyOS_createApp("Fantasy Game", &screenDim, false);

    
    if(appInfo->valid) {
        
        easyOS_setupApp(appInfo, &resolution, RESOURCE_PATH_EXTENSION);

        //Gif Recording stuff    
        int gif_width = 480, gif_height = appInfo->aspectRatio_yOverX*gif_width, centisecondsPerFrame = 5, bitDepth = 16;
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
        
        EasyCamera camera;
        easy3d_initCamera(&camera, v3(0, 0, -10));
        
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
            loadAndAddImagesStripToAssets(&gameState->wizardRun, "img/fantasy_sprites/wizard/Run.png", 231);

            easyAnimation_initAnimation(&gameState->wizardAttack, "wizardAttack");
            loadAndAddImagesStripToAssets(&gameState->wizardAttack, "img/fantasy_sprites/wizard/Attack1.png", 231);

            easyAnimation_initAnimation(&gameState->wizardAttack2, "wizardAttack2");
            loadAndAddImagesStripToAssets(&gameState->wizardAttack2, "img/fantasy_sprites/wizard/Attack2.png", 231);

            easyAnimation_initAnimation(&gameState->wizardDeath, "wizardDeath");
            loadAndAddImagesStripToAssets(&gameState->wizardDeath, "img/fantasy_sprites/wizard/Death.png", 231);

            easyAnimation_initAnimation(&gameState->wizardHit, "wizardHit");
            loadAndAddImagesStripToAssets(&gameState->wizardHit, "img/fantasy_sprites/wizard/Hit.png", 231);

            easyAnimation_initAnimation(&gameState->wizardIdle, "wizardIdle");
            loadAndAddImagesStripToAssets(&gameState->wizardIdle, "img/fantasy_sprites/wizard/Idle.png", 231);

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


        EasyPhysics_beginWorld(&gameState->physicsWorld);

        float inverse_weight = 1 / 10.0f;

        EntityManager *manager = pushStruct(&globalLongTermArena, EntityManager);
        initEntityManager(manager);
        //Init player first so it's in slot 0 which is special since we want to update the player position before other entities
        Entity *player = initEntity(manager, &gameState->wizardIdle, v3(0, 0, 0), v2(2.4f, 2.0f), v2(0.5f, 0.5f), gameState, ENTITY_WIZARD, inverse_weight, 0, COLOR_WHITE, 0, true);


        // initEntity(manager, &gameState->firePitAnimation, v3(0, 1, 0), v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, 3);
        initEntity(manager, &gameState->torchAnimation, v3(0, -3, 0), v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, 0, COLOR_WHITE, -1, false);
        initEntity(manager, &gameState->skeltonIdle, v3(-3, 0, 0), v2(2.5f, 2.5f), v2(0.5f, 0.4f), gameState, ENTITY_SKELETON, inverse_weight, 0, COLOR_WHITE, 1, true);

        
        initEntity(manager, 0, v3(0, -4, 0), v2(10, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, &globalWhiteTexture, COLOR_BLACK, 2, true);
        initEntity(manager, 0, v3(3, -3.5f, 0), v2(3, 2.5f), v2(1, 1), gameState, ENTITY_SCENERY, 0, &globalWhiteTexture, COLOR_BLACK, 2, true);

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
        while(appInfo->running) {

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
            V3 worldP = easyTransform_getWorldPos(&player->T);

            float distance = absVal(worldP.x - camera.hidden_pos.x);

            if(distance > 2.0f) {
                float newPos = lerp(camera.hidden_pos.x, clamp01(appInfo->dt*1.f), player->T.pos.x);

                camera.hidden_pos.x = newPos;
            }
            ////////////////

            easy3d_updateCamera(&camera, &appInfo->keyStates, 1, 1000.0f, appInfo->dt, camMove);


            easy_setEyePosition(globalRenderGroup, camera.pos);
            
            // update camera first
            Matrix4 viewMatrix = easy3d_getWorldToView(&camera);
            Matrix4 perspectiveMatrix = projectionMatrixFOV(camera.zoom, resolution.x/resolution.y);

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
                // playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
            }

            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_F1)) {
                if(recording) {
                    easyFlashText_addText(&globalFlashTextManager, "STOP RECORDING");

                    MsfGifResult result = msf_gif_end(&gifState);
                    char str[1028];
                    sprintf(str, "%s%s", appInfo->saveFolderLocation, "MyGif.gif");

                    FILE * fp = fopen(str, "wb");
                    fwrite(result.data, result.dataSize, 1, fp);
                    fclose(fp);
                    msf_gif_free(result);

                } else {
                    easyFlashText_addText(&globalFlashTextManager, "START RECORDING");

                    msf_gif_begin(&gifState, gif_height, gif_height);
                    
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
                        gameState->indexInItems = arrayCount(player->itemSpots) - 1;
                    }

                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                }

                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT)) {
                    gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                    gameState->indexInItems++;
                    playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
                    if(gameState->indexInItems >= arrayCount(player->itemSpots)) {
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

            //DEBUG
            if(false) {

                for(int i = 0; i < manager->entities.count; ++i) {
                    Entity *e = (Entity *)getElement(&manager->entities, i);
                    if(e && e->collider) {
                        e->T.pos.z = -0.1f*e->layer;
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
                        renderDrawQuad(globalRenderGroup, color);
                        e->T.pos.z = 0;

                        e->T.scale = prevScale;
                    }
                    
                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
                renderClearDepthBuffer(mainFrameBuffer.bufferId);
            }
            //////////////////////////////////////////

            
           

            for(int i = 0; i < manager->entities.count; ++i) {
                Entity *e = (Entity *)getElement(&manager->entities, i);
                if(e) {
                    updateEntity(manager, e, gameState, appInfo->dt, &gameKeyStates, &appInfo->console, &camera, player, gameState->isLookingAtItems);        
                
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

                        float inverse_weight = 0;

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
                
                setProjectionTransform(globalRenderGroup, perspectiveMatrix);//OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

                //Update animation timer
                gameState->lookingAt_animTimer.current = lerp(gameState->lookingAt_animTimer.current, 20.0f*clamp01(appInfo->dt), gameState->lookingAt_animTimer.target);
                ////


                Matrix4 T_ = Matrix4_scale(mat4(), v3(2, 2, 0));
                Matrix4 T_1 = Matrix4_scale(mat4(), v3(0.8f, 0.8f, 0));

                
                float angle = PI32/2.0f;
                float angleUpdate = 2*PI32 / arrayCount(player->itemSpots);
                float radius = gameState->lookingAt_animTimer.current*3.5f;
                for(int i = 0; i < arrayCount(player->itemSpots); ++i) {

                    V2 pos = v2(radius*cos(angle), radius*sin(angle));

                    gameState->animationItemTimers[i].current = lerp(gameState->animationItemTimers[i].current, 8*clamp01(appInfo->dt), gameState->animationItemTimers[i].target);                    

                    float growthSize = gameState->animationItemTimers[i].current;

                    if(player->itemSpots[i] != ENTITY_NULL) {
                        Texture *t = getInvetoryTexture(player->itemSpots[i]);
                        
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
                            if(player->itemSpots[i] != ENTITY_NULL) {
                                EntityType tempType = gameState->playerHolding[0];

                                gameState->playerHolding[0] = player->itemSpots[i];
                                player->itemSpots[i] = tempType;

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
                    easyConsole_addToStream(DEBUG_globalEasyConsole, "HAS SOMETHING");

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
                    if(stringsMatchNullN("camMove", token.at, token.size)) {
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
                    u8 *stream = (u8 *)easyPlatform_allocateMemory(resolution.x*resolution.y*4, EASY_PLATFORM_MEMORY_NONE);

                    renderReadPixels(endBuffer->bufferId, 0, 0, resolution.x, resolution.y, 
                                          (u32)GL_RGBA,
                                          (u32)GL_UNSIGNED_BYTE,
                                          stream);

                    // u8 *output_pixels = (u8 *)easyPlatform_allocateMemory(gif_width*gif_height*4, EASY_PLATFORM_MEMORY_NONE);

                    // stbir_resize_uint8(stream, resolution.x, resolution.y, 0, output_pixels, gif_width, gif_height, 0, 4);
                                                   
                    msf_gif_frame(&gifState, stream, centisecondsPerFrame, bitDepth, -gif_width * 4); //frame 1  

                    easyPlatform_freeMemory(stream); 
                    // easyPlatform_freeMemory(output_pixels); 
                } 
            }
            
        }
        easyOS_endProgram(appInfo);
    }
    return(0);
}
