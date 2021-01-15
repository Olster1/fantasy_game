#include "defines.h"
#include "easy_headers.h"


#include "gameState.h"
#include "gameScene.c"
#include "entity.c"



int main(int argc, char *args[]) {
    
    EASY_ENGINE_ENTRY_SETUP()
    
    V2 screenDim = v2(DEFINES_WINDOW_SIZE_X, DEFINES_WINDOW_SIZE_Y); //init in create app function
    V2 resolution = v2(DEFINES_RESOLUTION_X, DEFINES_RESOLUTION_Y);
    
    OSAppInfo *appInfo = easyOS_createApp("Fantasy Game", &screenDim, false);
    
    if(appInfo->valid) {
        
        easyOS_setupApp(appInfo, &resolution, RESOURCE_PATH_EXTENSION);

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

            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_R)) {
                gameState->isLookingAtItems = !gameState->isLookingAtItems;
            }


            if(gameState->isLookingAtItems) {

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

                        Entity *e1 = initEntity(manager, &gameState->firePitAnimation, e->position, v2(1, 1), v2(1, 1), gameState, e->type, 0, 0, COLOR_WHITE, layer, true);
                        e1->rb->dP = e->dP;
                        e1->lifeSpanLeft = 3.0f;
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
            renderClearDepthBuffer(mainFrameBuffer.bufferId);

            FrameBuffer *endBuffer = &mainFrameBuffer;
            if(gameState->isLookingAtItems) {
                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;

            }
            
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
        }
        easyOS_endProgram(appInfo);
    }
    return(0);
}
