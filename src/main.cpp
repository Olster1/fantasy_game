#include "defines.h"
#include "easy_headers.h"

//****** TODO *********/////
/*
Engine: 


Gameplay:


1. Mini game for the player in the shop to make money
2. Irish round house with cauldron in it
3. Crafting herbal potions like chemistry -- find different chemistry notes. Make coal or slime for things that don't work

2. cut grass to find potions and coins
3. When pick up item in the wild, if haven't picked it up before, breifly display above the head of the player

*/
///////////////////////////

static bool DEBUG_DRAW_SCENERY_TEXTURES = true;
static bool DEBUG_GRAVITY = false;
static bool DEBUG_DRAW_TERRAIN = true;
static bool DEBUG_LOCK_POSITION_TO_GRID = true;
static bool DEBUG_ANGLE_ENTITY_ON_CREATION = true;
static bool DEBUG_DRAW_COLLISION_BOUNDS = false; 
static bool DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS = false; 
static bool DEBUG_USE_ORTHO_MATRIX = false;
static bool DEBUG_AI_BOARD_FOR_ENTITY = true;
static bool DEBUG_CONTROL_PLAYER = true;

static int GLOBAL_WORLD_TILE_SIZE = 1;

static float global_cameraFollow_yOffset = 40;

#include "npc_dialog.h"
#include "gameState.h"
#include "editor.h"
#include "game_weather.c"


static void setCameraPosition(EasyCamera *camera, V3 position) {
    camera->pos.xy = v3_minus(position, v3(0, global_cameraFollow_yOffset, 0)).xy;
    camera->hidden_pos.xy = camera->pos.xy;
}

static Texture *getInvetoryTexture(EntityType type) {
    Texture *t = 0;
    switch(type) {
        case ENTITY_HEALTH_POTION_1: {
            t = findTextureAsset("green_potion.png");
            // assert(false);
        } break;
        case ENTITY_STAMINA_POTION_1: {
            t = findTextureAsset("yellow_potion.png");
            // assert(false);
        } break;
        case ENTITY_KEY: {
            t = findTextureAsset("keyGolden.png");
            // assert(false);
        } break;
        case ENTITY_SWORD: {
            t = findTextureAsset("inventory_sword.png");
        } break;
        case ENTITY_SHEILD: {
            t = findTextureAsset("inventory_shield.png");
        } break;
        case ENTITY_BOMB: {
            t = findTextureAsset("bomb.png");
        } break;
        case ENTITY_NETTLE: {
            t = findTextureAsset("nettle_closeup.png");
        } break;
        case ENTITY_DOCK: {
            t = findTextureAsset("dock_closeup.png");
        } break;
        case ENTITY_PLAYER_PROJECTILE: {
            t = findTextureAsset("dart.png");
        } break;
        case ENTITY_BOTTLE: {
            t = findTextureAsset("bottle.png");
        } break;
        case ENTITY_QUIVER: {
            t = findTextureAsset("quiver.png");
        } break;
        default: {
            assert(false);
        }
    }
    return t;
}

//this maps the sprite name to the tile type
static WorldTileType getTileTypeBasedOnSprite(GameState *gameState, int tileIndex) {

    WorldTileType result = WORLD_TILE_GRASS;

    Texture **ts = (Texture **)gameState->splatTextures_tiles.memory;

    char *name = ts[tileIndex]->name;


    if(false) {
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_tile")) {
        result = WORLD_TILE_GRASS;
    } else if(easyString_stringsMatch_nullTerminated(name, "lava_tile")) {
        result = WORLD_TILE_LAVA;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_tile")) {
        result = WORLD_TILE_ROCK;
    } else if(easyString_stringsMatch_nullTerminated(name, "cobble")) {
        result = WORLD_TILE_COBBLE;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road")) {
        result = WORLD_TILE_PATH;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_horzintoal")) {
        result = WORLD_TILE_PATH1;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_corner")) {
        result = WORLD_TILE_PATH2;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_corner-bottom")) {
        result = WORLD_TILE_PATH3;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_corner-right")) {
        result = WORLD_TILE_PATH4;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_corner-left")) {
        result = WORLD_TILE_PATH5;
    } else if(easyString_stringsMatch_nullTerminated(name, "sand")) {
        result = WORLD_TILE_BEACH;
    } else if(easyString_stringsMatch_nullTerminated(name, "water")) {
        result = WORLD_TILE_SEA;
    } else if(easyString_stringsMatch_nullTerminated(name, "water2")) {
        result = WORLD_TILE_SEA1;
    } else if(easyString_stringsMatch_nullTerminated(name, "sand-grass")) {
        result = WORLD_TILE_BEACH_GRASS;
    } else if(easyString_stringsMatch_nullTerminated(name, "sand-water")) {
        result = WORLD_TILE_SAND_WATER;
    } else if(easyString_stringsMatch_nullTerminated(name, "sand-water2")) {
        result = WORLD_TILE_SAND_WATER1;
    } else if(easyString_stringsMatch_nullTerminated(name, "pier_middle_middle")) {
        result = WORLD_TILE_PIER_MIDDLE;
    } else if(easyString_stringsMatch_nullTerminated(name, "pier_middle_middle-end-sand")) {
        result = WORLD_TILE_PIER_SAND;
    } else if(easyString_stringsMatch_nullTerminated(name, "piermiddle")) {
        result = WORLD_TILE_PIER_SIDE_LEFT;
    } else if(easyString_stringsMatch_nullTerminated(name, "piermiddle-right")) {
        result = WORLD_TILE_PIER_SIDE_RIGHT;
    } else if(easyString_stringsMatch_nullTerminated(name, "pier-top-left-sand")) {
        result = WORLD_TILE_PIER_SAND_CORNER_LEFT;
    } else if(easyString_stringsMatch_nullTerminated(name, "pier-top-right-sand")) {
        result = WORLD_TILE_PIER_SAND_CORNER_RIGHT;
    } else if(easyString_stringsMatch_nullTerminated(name, "piermiddle_right_corner")) {
        result = WORLD_TILE_PIER_SEA_CORNER_RIGHT;
    } else if(easyString_stringsMatch_nullTerminated(name, "piercorner")) {
        result = WORLD_TILE_PIER_SEA_CORNER_LEFT;
    } else if(easyString_stringsMatch_nullTerminated(name, "dirt_tile")) {
        result = WORLD_TILE_DIRT;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_nub")) {
        result = WORLD_TILE_PATH_NUB1;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_nub_bottom")) {
        result = WORLD_TILE_PATH_NUB2;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_nub_left")) {
        result = WORLD_TILE_PATH_NUB3;
    } else if(easyString_stringsMatch_nullTerminated(name, "grass_road_nub_right")) {
        result = WORLD_TILE_PATH_NUB4;
    } else if(easyString_stringsMatch_nullTerminated(name, "brick")) {
        result = WORLD_TILE_BRICK;
    } else if(easyString_stringsMatch_nullTerminated(name, "wood_floor_tile")) {
        result = WORLD_TILE_WOOD_FLOOR;
    } else {
        result = WORLD_TILE_NULL;
    }
    


    return result;
}

static char *getInventoryString(EntityType type) {
    char *result = "Empty";
    switch(type) {
        case ENTITY_HEALTH_POTION_1: {
            result = "Restore 3 health points with this bubbly potion.";
            // assert(false);
        } break;
        case ENTITY_STAMINA_POTION_1: {
            result = "Restore 3 stamina points with this yellow mixture.";
            // assert(false);
        } break;
        case ENTITY_SWORD: {
            result = "Sharp sword";
        } break;
        case ENTITY_SHEILD: {
            result = "Protective shield";
        } break;
        case ENTITY_KEY: {
            result = "A key to open locks.";
        } break;
        case ENTITY_BOMB: {
            result = "A bomb";
        } break;
        case ENTITY_NETTLE: {
            result = "Stinging Nettle plant";
        } break;
        case ENTITY_DOCK: {
            result = "Bitter Dock leaf";
        } break;
        default: {

        }
    }
    return result;
}

float getWorldTextWidth(float size, EasyFont_Font *gameFont, char *at) {
    float fontSize_to_worldSize = size*((float)GLOBAL_DEFINE_defaultMetersPerLetter_floatingText / (float)gameFont->fontHeight);

    float xAt = 0;

    while(*at) {

        unsigned int unicodePoint = easyUnicode_utf8_codepoint_To_Utf32_codepoint((char **)&at, true);
        
        EasyFont_Glyph *g = easyFont_findGlyph(gameFont, unicodePoint); 
        assert(g->codepoint == unicodePoint);

        xAt += (g->texture.width + g->xOffset)*fontSize_to_worldSize;
    }

    return xAt;
}

struct ParticleSystemListItem {
    particle_system ps;
    V3 *position; //pointing to position in Entity Transform
    V3 offset;
    V4 color;
};



static V3 roundToGridBoard(V3 in, float tileSize) {
    float xMod = (in.x < 0) ? -tileSize : tileSize;
    float yMod = (in.y < 0) ? -tileSize : tileSize;
    
    V3 result = {};
    if(tileSize == 1) {
        result = v3((int)(in.x + xMod*0.5f), (int)(in.y + yMod*0.5f), (int)(in.z));
    } else {
        result = v3((int)(in.x + xMod*0.5f), (int)(in.y + yMod*0.5f), (int)(in.z));

        result.x -= ((int)result.x) % (int)tileSize;
        result.y -= ((int)result.y) % (int)tileSize;
    }
    
    return result;
}


typedef struct {
    Array_Dynamic entities;

    Array_Dynamic entitiesToAddForFrame;
    Array_Dynamic entitiesToDeleteForFrame;
    Array_Dynamic entitiesToDeleteOnSave;

    Array_Dynamic damageNumbers;
    Array_Dynamic activeParticleSystems;

    int lastEntityIndex;
    void *player;
} EntityManager;    


bool gameScene_doesSceneExist(char *sceneName);
void gameScene_loadScene(GameState *gameState, EntityManager *manager, char *sceneName_, GameWeatherState *weatherState);
#include "entity.c"
#include "shop.c"
#include "crafting.c"

static bool drawAndUpdateMessageSystem(OSAppInfo *appInfo, GameState *gameState, EntityManager *manager, GameWeatherState *weatherState, EasyTransitionState *transitionState, float tweakY, float tweakWidth, float tweakSpacing, EasyFont_Font *gameFont, FrameBuffer *mainFrameBuffer) {

    if(gameState->enteredTalkModeThisFrame) {
        //NOTE Clear the queued dialog
        gameState->queuedActionAfterDialog = DIALOG_ACTION_NULL;
    }

    bool isCompletelyFinished = false;
    bool endMessagesImmediately = false;

    EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);

    char *currentTalkText = gameState->currentTalkText->texts[gameState->messageIndex];

    float fuaxWidth = 1920.0f;
    float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

    float xOffset_leftColumn = 0;
    float xOffset_rightColumn = 0;

    GameModeType nextModeToGoTo = GAME_MODE_PLAY; 

    float alpha = 0.0f;

    EntityDialogNode *nextTalkNode = 0;

    float revealChoicePeriod = 0.6f;

    //update Timer
    if(gameState->dialogChoiceTimer >= 0.0f) {
     gameState->dialogChoiceTimer += appInfo->dt;

     float canVal = clamp01(gameState->dialogChoiceTimer / revealChoicePeriod);

     alpha = canVal; 

     xOffset_leftColumn = smoothStep01(0, canVal, -400);                
     xOffset_rightColumn = smoothStep01(0, canVal, 150);                

    }    

    float returnToCenterPeriod = 0.3f;

    //update Timer
    if(gameState->dialogChoiceTimerReturn >= 0.0f) {
     gameState->dialogChoiceTimerReturn += appInfo->dt;

     float canVal = clamp01(gameState->dialogChoiceTimerReturn / returnToCenterPeriod);

     alpha = 1.0f - canVal; 

     xOffset_leftColumn = smoothStep01(-400, canVal, 0);                
     xOffset_rightColumn = smoothStep01(150, canVal, 0);

     if(canVal >= 1.0f) {
         gameState->dialogChoiceTimerReturn = -1.0f;
     }                
    }

    bool isFontWriterFinished = false;

    setViewTransform(globalRenderGroup, mat4());
    Matrix4 projection = OrthoMatrixToScreen(fuaxWidth, fuaxHeight);
    setProjectionTransform(globalRenderGroup, projection);

    //draw the font writer
    {

        EasyTransform T;
        easyTransform_initTransform_withScale(&T, v3(xOffset_leftColumn, -300, 0.4f), v3(1000, 400, 1), EASY_TRANSFORM_NO_ID); 

        setModelTransform(globalRenderGroup, easyTransform_getTransform(&T));
        
        renderSetShader(globalRenderGroup, &textureProgram);
        renderDrawSprite(globalRenderGroup, findTextureAsset("quill.png"), COLOR_WHITE);

        renderSetShader(globalRenderGroup, &fontProgram);

        //if we're returning back to center we don't start writing 
        float tUpdate = appInfo->dt;

        if(gameState->dialogChoiceTimerReturn >= 0.0f && gameState->dialogChoiceTimerReturn < 0.9f*returnToCenterPeriod) {
           tUpdate = 0;
        }

        isFontWriterFinished = easyFontWriter_updateFontWriter(&gameState->fontWriter, currentTalkText, tUpdate, &T, v3(xOffset_leftColumn + -400, -200, 0.1f), false); 

    }

    //Since we advance from the current node when we animate back in, we keep the last available node around to draw the right options. 
     bool drawOptions = false;

     EntityDialogNode *nodeToDrawOptionsFrom = gameState->currentTalkText;

    if(gameState->dialogChoiceTimerReturn >= 0.0f) {
      drawOptions = true;
      nodeToDrawOptionsFrom = gameState->prevTalkNode;
    }

    if(isFontWriterFinished && gameState->messageIndex >= (gameState->currentTalkText->textCount - 1) && gameState->currentTalkText->choiceCount > 0) {

     if(gameState->gameModeSubType != GAME_MODE_SUBTYPE_TALKING_CHOOSE_OPTION) {
         gameState->dialogChoiceTimer = 0;
         gameState->choiceOptionIndex = 0;
     }

     drawOptions = true;

     //Check if there are any resposes
       gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING_CHOOSE_OPTION; 


       //don't let the player choose while is still animating (up until 90% through animation)
       if((gameState->dialogChoiceTimer / revealChoicePeriod) > 0.9f) {
           //Choose the option 
           if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_DOWN)) {
             gameState->choiceOptionIndex++;
           }

           if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_UP)) {
             gameState->choiceOptionIndex--;
           }

           gameState->choiceOptionIndex = clamp(0, gameState->choiceOptionIndex, gameState->currentTalkText->choiceCount - 1);
           ////////////////////////////////////////////////////////////

           if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_SPACE)) {
            //Where we actually commit our choice

            //NOTE: Activate any action that needs doing
            DialogActionType actionType = gameState->currentTalkText->actions[gameState->choiceOptionIndex];
            if(actionType == DIALOG_ACTION_SAVE_PROGRESS) {
                playerGameState_saveProgress(gameState, manager, appInfo->saveFolderLocation);                
                playMenuSound(&globalLongTermArena, gameState->saveSuccessSound, 0, AUDIO_FOREGROUND);
            } else if(actionType == DIALOG_ACTION_EXIT_DIALOG) {
                playMenuSound(&globalLongTermArena, gameState->exitUiSound, 0, AUDIO_FOREGROUND);
            } else if(actionType == DIALOG_ACTION_GO_TO_SHOP) {
                enterGameShop(gameState->townShop, gameState);

               endMessagesImmediately = true;

               nextModeToGoTo = GAME_MODE_SHOP;
                    
                // gameState->queuedActionAfterDialog = DIALOG_ACTION_GO_TO_SHOP;
                playMenuSound(&globalLongTermArena, gameState->exitUiSound, 0, AUDIO_FOREGROUND);

            } else if(actionType == DIALOG_ACTION_GO_TO_CRAFTING) {
                enterGameShop(gameState->townShop, gameState);

               endMessagesImmediately = true;

               enterGameCrafting(gameState);

               nextModeToGoTo = GAME_MODE_CRAFTING;
                    
                // gameState->queuedActionAfterDialog = DIALOG_ACTION_GO_TO_SHOP;
                playMenuSound(&globalLongTermArena, gameState->exitUiSound, 0, AUDIO_FOREGROUND);
            } else if(actionType == DIALOG_ACTION_SLEEP) {
                //NOTE: Play animation of getting in bed? 
                //Fade out briefly
                EntitySleepInBedData *data = (EntitySleepInBedData *)easyPlatform_allocateMemory(sizeof(EntitySleepInBedData), EASY_PLATFORM_MEMORY_ZERO);
                data->weatherState = weatherState;
                data->gameState = gameState;
                data->hoursToSleep = 8;

                EasySceneTransition *transition = EasyTransition_PushTransition(transitionState, sleepInBed, data, EASY_TRANSITION_FADE);
                transition->shouldHold = true;
                gameState->queuedActionAfterDialog = DIALOG_ACTION_SLEEP; 

            }

            //NOTE: Move to the next node
             nextTalkNode = gameState->currentTalkText->next[gameState->choiceOptionIndex];
             // assert(nextTalkNode);

           }
       }


       
    }


    if(drawOptions) {
     EasyTransform T;
     easyTransform_initTransform_withScale(&T, v3(0, tweakY, 0), v3(1, 1, 1), EASY_TRANSFORM_NO_ID); 
     

     EasyTransform quillT;
     easyTransform_initTransform_withScale(&quillT, v3(0, tweakY, 0.7f), v3(tweakWidth, 100, 1), EASY_TRANSFORM_NO_ID); 

     //Draw the options
     for(int i = 0; i < nodeToDrawOptionsFrom->choiceCount; ++i) {
           char *str = nodeToDrawOptionsFrom->choices[i];

           T.pos.y -= tweakSpacing;

           quillT.pos.x = xOffset_rightColumn + 300;
           quillT.pos.y = T.pos.y;
           

           setModelTransform(globalRenderGroup, easyTransform_getTransform(&quillT));
           renderSetShader(globalRenderGroup, &textureProgram);

           V4 c = COLOR_WHITE;

           if(gameState->choiceOptionIndex == i) {
               c = COLOR_GOLD;
           }
           
           c.w = alpha;
           renderDrawSprite(globalRenderGroup, findTextureAsset("quill.png"), c);

           renderSetShader(globalRenderGroup, &fontProgram);
          
           easyFont_drawString(str, &T, v3(xOffset_rightColumn, T.pos.y, 0.6f), gameFont, false, v4(0, 0, 0, alpha));
         }
    }

    // float fontx = 0.2f*gameState->fuaxResolution.x;

    // V2 size = getBounds(currentTalkText, rect2fMinMax(fontx, 0, 0.9f*gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 1.5f, gameState->fuaxResolution, 1);
     

    // float fontx = -0.5f*size.x + 0.5f*fuaxWidth; 
    // float fonty = fuaxHeight - 0.3f*textBg_height;

    // outputTextNoBacking(gameFont, fontx, fonty, 0.1f, v2(fuaxWidth, fuaxHeight), currentTalkText, rect2fMinMax(0.2f*gameState->fuaxResolution.x, 0, 1.0f*gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 1.5f, true, 1);

    renderSetShader(globalRenderGroup, &textureProgram);
    //Draw prompt button to continue
    Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(100, 100*gameState->spacePrompt->aspectRatio_h_over_w, 0)), v3(0.4f*fuaxWidth, -0.35f*fuaxHeight, 0.4f));
    setModelTransform(globalRenderGroup, T);
    renderDrawSprite(globalRenderGroup, gameState->spacePrompt, COLOR_WHITE);
    ///////

    drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

    renderClearDepthBuffer(mainFrameBuffer->bufferId);

    easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

     bool returningToCenter = (gameState->dialogChoiceTimerReturn >= 0.0f && gameState->dialogChoiceTimerReturn < 0.9f*returnToCenterPeriod);

    //Exit back to game
    if(((wasPressed(appInfo->keyStates.gameButtons, BUTTON_SPACE) || gameState->currentTalkText->isEndNode) && gameState->gameModeSubType == GAME_MODE_SUBTYPE_TALKING && !gameState->enteredTalkModeThisFrame && !returningToCenter)) {
         // if(gameState->talkingNPC) {
         //     easySound_endSound(gameState->talkingNPC);
         // }

         bool writerFinished = easyFontWriter_isFontWriterFinished(&gameState->fontWriter, currentTalkText);

         //still messages
         if((gameState->messageIndex < (gameState->currentTalkText->textCount - 1) || (gameState->messageIndex == (gameState->currentTalkText->textCount - 1) && !writerFinished))) {
             assert(gameState->gameModeSubType == GAME_MODE_SUBTYPE_TALKING);

             if(!writerFinished) {
                 easyFontWriter_finishFontWriter(&gameState->fontWriter, currentTalkText); 
             } else {
                 //still messages left in array 
                 gameState->messageIndex++;

                 easyFontWriter_resetFontWriter(&gameState->fontWriter);

                 WavFile *sound = 0;
                 // if(gameState->currentTalkText.audioArray) {
                 //     Asset *asset = findAsset(gameState->currentTalkText.audioArray[gameState->messageIndex]);
                 //     if(asset) {
                 //         sound = (WavFile *)asset->file;
                 //     }
                 //     if(sound) {
                 //         gameState->talkingNPC = playGameSound(&globalLongTermArena, sound, 0, AUDIO_FOREGROUND);
                 //         gameState->talkingNPC->volume = 3.0f;
                 //     }
                 // }    
             }

         } else if(gameState->currentTalkText->choiceCount > 0) {
             //go to choices
             assert(gameState->gameModeSubType != GAME_MODE_SUBTYPE_TALKING_CHOOSE_OPTION);

             gameState->dialogChoiceTimer = 0;
             gameState->choiceOptionIndex = 0;
             gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING_CHOOSE_OPTION;
             easyFontWriter_finishFontWriter(&gameState->fontWriter, currentTalkText); 
         } else {
            endMessagesImmediately = true;
         }
     }

     if(endMessagesImmediately) {
          //finished
          gameState->gameModeType = nextModeToGoTo;
          
          if(nextModeToGoTo == GAME_MODE_PLAY) {
              gameState->gameIsPaused = false;
          }
          // gameState->talkingNPC = 0;
          //go back to start just to make sure the next time someone else tries to play it
          gameState->messageIndex = 0;
          isCompletelyFinished = true;
          assert(!nextTalkNode);

          if(gameState->queuedActionAfterDialog == DIALOG_ACTION_SLEEP) {
             gameState->gameIsPaused = true;
         }

          gameState->dialogChoiceTimer = -1;
          gameState->messageIndex = 0;
          gameState->dialogChoiceTimerReturn = -1;

         //Clear the queued action
         gameState->queuedActionAfterDialog = DIALOG_ACTION_NULL;

          if(gameState->gameDialogs.perDialogArenaMark.arena) {
             releaseMemoryMark(&gameState->gameDialogs.perDialogArenaMark);
             //clear the arena to make sure we don't enter here again if user hasn't specifically set memory mark
             easyMemory_zeroStruct(&gameState->gameDialogs.perDialogArenaMark, MemoryArenaMark);
          }
      }

     if(nextTalkNode) {
         //Store the previous node   
         gameState->prevTalkNode = gameState->currentTalkText;

         gameState->currentTalkText = nextTalkNode;
         gameState->gameModeSubType = GAME_MODE_SUBTYPE_TALKING;
         easyFontWriter_resetFontWriter(&gameState->fontWriter);
         gameState->dialogChoiceTimer = -1;
         gameState->messageIndex = 0;
         gameState->dialogChoiceTimerReturn = 0;
     }
     
     gameState->enteredTalkModeThisFrame = false;

     return isCompletelyFinished;
}



#include "saveload.c"




#define MSF_GIF_IMPL
#include "msf_gif.h"

// #define STB_IMAGE_RESIZE_IMPLEMENTATION
// #include "stb_resize.h"

#define GIF_MODE 0


static void drawTriggerCollider_DEBUG(Entity *e, EasyCollider *collider) {
        EasyTransform prevT = e->T;

        e->T.scale.x *= collider->dim2f.x;
        e->T.scale.y *= collider->dim2f.y;

        Quaternion Q = e->T.Q;

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
        
        e->T = prevT;
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
        
        easyOS_setupApp(appInfo, &resolution, RESOURCE_PATH_EXTENSION, "/Illoway");

        //Gif Recording stuff    
        int gif_width = resolution.x, gif_height = resolution.y, centisecondsPerFrame = 10, bitDepth = 16;
        MsfGifState gifState = {};
        /////

        

        FrameBuffer mainFrameBuffer;
        FrameBuffer toneMappedBuffer;
        FrameBuffer bloomFrameBuffer;
        FrameBuffer cachedFrameBuffer;
        FrameBuffer shadowMapBuffer;

        {
            DEBUG_TIME_BLOCK_NAMED("Create frame buffers");
            
            //******** CREATE THE FRAME BUFFERS ********///
            
            mainFrameBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_DEPTH | FRAMEBUFFER_STENCIL | FRAMEBUFFER_HDR, 2);
            toneMappedBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_DEPTH | FRAMEBUFFER_STENCIL, 1);
            
            bloomFrameBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_HDR, 1);

            shadowMapBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_DEPTH, 0);
            
            cachedFrameBuffer = createFrameBuffer(resolution.x, resolution.y, FRAMEBUFFER_COLOR | FRAMEBUFFER_HDR, 1);
            //////////////////////////////////////////////////
        
        }

        RenderGroup *shadowMapRenderGroup = pushStruct(&globalLongTermArena, RenderGroup);
        initRenderGroup(shadowMapRenderGroup, resolution.x, resolution.y);

        //used to store entities in so we can draw them after the terrain, 
        RenderGroup *entitiesRenderGroup = pushStruct(&globalLongTermArena, RenderGroup);
        initRenderGroup(entitiesRenderGroup, resolution.x, resolution.y);
        

        loadAndAddImagesToAssets("img/engine_icons/", false);
        loadAndAddImagesToAssets("img/temp_images/", false);
        loadAndAddImagesToAssets("img/loadOnStart/", true);
        loadAndAddImagesToAssets("img/inventory/", true);
        loadAndAddImagesToAssets("img/tilesets/", true);


       



        //Setup the gravity if it's on or not. GAMEPLAY: Could be an interesting gameplay feature: magentic rooms
        if(DEBUG_GRAVITY) {
            global_easyPhysics_gravityModifier = 1;
        } else {
            global_easyPhysics_gravityModifier = 0;
        }
        ///
        
        EasyCamera camera;
        easy3d_initCamera(&camera, v3(0, 0, -40));
        camera.zoom = 6.0f;

        camera.orientation = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);
        
        EasyTransform sunTransform;
        easyTransform_initTransform(&sunTransform, v3(0, 0, -40), EASY_TRANSFORM_TRANSIENT_ID);

        sunTransform.Q = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);
        
        EasyLight *light = easy_makeLight(&sunTransform, EASY_LIGHT_DIRECTIONAL, 1.0f, v3(1, 1, 1));
        light->shadowMapId = shadowMapBuffer.depthId;
        easy_addLight(globalRenderGroup, light); 
        easy_addLight(shadowMapRenderGroup, light);
        easy_addLight(entitiesRenderGroup, light);
            
        easyRender_setDepthFuncType(shadowMapRenderGroup, RENDER_DEPTH_FUNC_LESS);
        
        GameState *gameState = initGameState((resolution.y / resolution.x));

        GameWeatherState *weatherState = initWeatherState();



        ///////// LOAD ALL ANIMATION FILES

        
        {
            char *str = "Empty";
            addElementInfinteAlloc_notPointer(&gameState->splatListAnimations, str);
            char *imgFileTypes[] = {"jpg", "jpeg", "png", "bmp", "PNG"};
            char *folderName = concatInArena(globalExeBasePath, "img/fantasy_sprites/animaionPreLoad/", &globalPerFrameArena);
            FileNameOfType splatAnimationFileNames = getDirectoryFilesOfType(folderName, imgFileTypes, arrayCount(imgFileTypes));
            int splatAnimationCount = splatAnimationFileNames.count;
            for(int i = 0; i < splatAnimationFileNames.count; ++i) {
                char *fullName = splatAnimationFileNames.names[i];
                char *shortName = getFileLastPortion(fullName);
                if(shortName[0] != '.') { //don't load hidden file 
                    addElementInfinteAlloc_notPointer(&gameState->splatListAnimations, shortName);

                    Animation *animation = (Animation *)easyPlatform_allocateMemory(sizeof(Animation), EASY_PLATFORM_MEMORY_NONE);

                    easyAnimation_initAnimation(animation, shortName);
                    loadAndAddImagesStripToAssets_(animation, fullName, 0, false, true);

                    addAssetAnimation(shortName, shortName, animation);

                    addElementInfinteAlloc_notPointer(&gameState->splatAnimations, animation);
                }
                free(fullName);
            }
        }

        ////////////////////////////////////////


        easyAnimation_initAnimation(&gameState->seagullAnimation, "seagullAnimation");
        loadAndAddImagesStripToAssets(&gameState->seagullAnimation, "img/fantasy_sprites/seagull_animation.png", 32, false);

        // easyAnimation_initAnimation(&gameState->barrelWater, "barrelWater");
        // loadAndAddImagesStripToAssets(&gameState->barrelWater, "img/fantasy_sprites/barrelWater.png", 32, false);

        easyAnimation_initAnimation(&gameState->firePitAnimation, "firepit_idle");
        loadAndAddImagesStripToAssets(&gameState->firePitAnimation, "img/fantasy_sprites/firePlace.png", 64, false);

        easyAnimation_initAnimation(&gameState->torchAnimation, "torch_idle");
        loadAndAddImagesStripToAssets(&gameState->torchAnimation, "img/fantasy_sprites/torch.png", 64, false);

        ///////////// Tile Animations ////////////////////
        easyAnimation_initAnimation(&gameState->seaTileAnimation, "seaTileAnimation");
        easyAnimation_pushFrame(&gameState->seaTileAnimation, "water.png");
        easyAnimation_pushFrame(&gameState->seaTileAnimation, "water2.png");
        easyAnimation_pushFrame(&gameState->seaTileAnimation, "water3.png");

        easyAnimation_initAnimation(&gameState->sandWaterTileAnimation, "sandWaterTileAnimation");
        easyAnimation_pushFrame(&gameState->sandWaterTileAnimation, "sand-water.png");
        easyAnimation_pushFrame(&gameState->sandWaterTileAnimation, "sand-water2.png");
        

        {
            //WIZARD ANIMATIONS

            gameState->wizardBottom = gameState_findSplatAnimation(gameState, "Man_forward_4.png");

            // easyAnimation_initAnimation(&gameState->wizardForward, "wizardForward");
            // loadAndAddImagesStripToAssets_count_offset(&gameState->wizardForward, "img/fantasy_sprites/man.png", 41, false, 4, 4);
            // easyAnimation_pushFrame(&gameState->wizardRun, "player.png");
            // easyAnimation_pushFrame(&gameState->wizardRun, "player.png");

            // easyAnimation_initAnimation(&gameState->wizardForward, "wizardForward");
            // loadAndAddImagesStripToAssets_count_offset(&gameState->wizardForward, "img/fantasy_sprites/man.png", 41, false, 4, 4);


            easyAnimation_initAnimation(&gameState->wizardSwimLeft, "wizardSwimLeft");
            loadAndAddImagesStripToAssets_count_offset(&gameState->wizardSwimLeft, "img/fantasy_sprites/swim.png", 41, false, 1, 3);
            easyAnimation_initAnimation(&gameState->wizardSwimRight, "wizardSwimRight");
            loadAndAddImagesStripToAssets_count_offset(&gameState->wizardSwimRight, "img/fantasy_sprites/swim1.png", 41, false, 1, 1);
            easyAnimation_initAnimation(&gameState->wizardSwimUp, "wizardSwimUp");
            loadAndAddImagesStripToAssets_count_offset(&gameState->wizardSwimUp, "img/fantasy_sprites/swim2.png", 41, false, 1, 2);
            easyAnimation_initAnimation(&gameState->wizardSwimDown, "wizardSwimDown");
            loadAndAddImagesStripToAssets_count_offset(&gameState->wizardSwimDown, "img/fantasy_sprites/swim3.png", 41, false, 1, 0);

            easyAnimation_initAnimation(&gameState->wizardSwordAttackLeft, "wizardAttackLeft");
            loadAndAddImagesStripToAssets(&gameState->wizardSwordAttackLeft, "img/fantasy_sprites/man_attack_left.png", 58, false);

            easyAnimation_initAnimation(&gameState->wizardSwordAttackRight, "wizardAttackRight");
            loadAndAddImagesStripToAssets(&gameState->wizardSwordAttackRight, "img/fantasy_sprites/man_attack_right.png", 58, false);

            gameState->wizardSwordAttackFront = gameState_findSplatAnimation(gameState, "Man_forward_attack_4.png");


            easyAnimation_initAnimation(&gameState->wizardSwordAttackBack, "wizardAttackBack");
            loadAndAddImagesStripToAssets(&gameState->wizardSwordAttackBack, "img/fantasy_sprites/man_attack_back.png", 42, false);



            easyAnimation_initAnimation(&gameState->wizardIdle, "wizardIdle");
            easyAnimation_pushFrame(&gameState->wizardIdle, "player.png");
            // loadAndAddImagesStripToAssets(&gameState->wizardIdle, "img/fantasy_sprites/wizard/Idle.png", 231);

            gameState->wizardIdleForward = gameState_findSplatAnimation(gameState, "Man_back_idle_6.png");
            gameState->wizardIdleBottom = gameState_findSplatAnimation(gameState, "Man_forward_idle_6.png");
            gameState->wizardIdleLeft = gameState_findSplatAnimation(gameState, "Man_left_idle_6.png");
            gameState->wizardIdleRight = gameState_findSplatAnimation(gameState, "Man_left_idle_6.png");

            gameState->wizardForward = gameState_findSplatAnimation(gameState, "Man_back_4.png");

            easyAnimation_initAnimation(&gameState->wizardGetItem, "wizardGetItem");
            loadAndAddImagesStripToAssets(&gameState->wizardGetItem, "img/fantasy_sprites/man_getitem.png", 41, false);
            

            gameState->wizardRight = gameState->wizardLeft = gameState_findSplatAnimation(gameState, "Man_left_walk_4.png");

            gameState->wizard_sideways_forward = gameState_findSplatAnimation(gameState, "Man_forward_sideways_4.png");
            gameState->wizard_sideways_back = gameState_findSplatAnimation(gameState, "Man_back_sideways_4.png");
            
            // easyAnimation_initAnimation(&gameState->wizardLeft, "wizardLeft");
            // loadAndAddImagesStripToAssets_count_offset(&gameState->wizardLeft, "img/fantasy_sprites/man2.png", 41, false, 4, 8);

            // easyAnimation_initAnimation(&gameState->wizardRight, "wizardRight");
            // loadAndAddImagesStripToAssets_count_offset(&gameState->wizardRight, "img/fantasy_sprites/man3.png", 41, false, 4, 0);

            ////////////////////////////////////////////////////////////////////////////

            easyAnimation_initAnimation(&gameState->wizardAttack, "wizardAttack");
            loadAndAddImagesStripToAssets(&gameState->wizardAttack, "img/fantasy_sprites/wizard/Attack1.png", 231, false);

            easyAnimation_initAnimation(&gameState->wizardAttack2, "wizardAttack2");
            loadAndAddImagesStripToAssets(&gameState->wizardAttack2, "img/fantasy_sprites/wizard/Attack2.png", 231, false);

            easyAnimation_initAnimation(&gameState->wizardDeath, "wizardDeath");
            loadAndAddImagesStripToAssets(&gameState->wizardDeath, "img/fantasy_sprites/wizard/Death.png", 231, false);

            easyAnimation_initAnimation(&gameState->wizardHit, "wizardHit");
            loadAndAddImagesStripToAssets(&gameState->wizardHit, "img/fantasy_sprites/wizard/Hit.png", 231, false);


            easyAnimation_initAnimation(&gameState->wizardJump, "wizardJump");
            loadAndAddImagesStripToAssets(&gameState->wizardJump, "img/fantasy_sprites/wizard/Jump.png", 231, false);

            easyAnimation_initAnimation(&gameState->wizardFall, "wizardFall");
            loadAndAddImagesStripToAssets(&gameState->wizardFall, "img/fantasy_sprites/wizard/Fall.png", 231, false);

        }

        {
            easyAnimation_initAnimation(&gameState->werewolfIdle, "werewolfIdle");
            // loadAndAddImagesStripToAssets(&gameState->wizardRun, "img/fantasy_sprites/wizard/Run.png", 231);
            easyAnimation_pushFrame(&gameState->werewolfIdle, "werewolf2.png");
            
        }

        {
            easyAnimation_initAnimation(&gameState->skeletonAttack, "skeltonAttack");
            loadAndAddImagesStripToAssets(&gameState->skeletonAttack, "img/fantasy_sprites/skeleton/SAttack.png", 150, false);

            easyAnimation_initAnimation(&gameState->skeletonDeath, "skeltonDeath");
            loadAndAddImagesStripToAssets(&gameState->skeletonDeath, "img/fantasy_sprites/skeleton/SDeath.png", 150, false);

            easyAnimation_initAnimation(&gameState->skeltonIdle, "skeltonIdle");
            loadAndAddImagesStripToAssets(&gameState->skeltonIdle, "img/fantasy_sprites/skeleton/SIdle.png", 150, false);

            easyAnimation_initAnimation(&gameState->skeltonShield, "skeltonShield");
            loadAndAddImagesStripToAssets(&gameState->skeltonShield, "img/fantasy_sprites/skeleton/SShield.png", 150, false);

            easyAnimation_initAnimation(&gameState->skeltonHit, "skeltonHit");
            loadAndAddImagesStripToAssets(&gameState->skeltonHit, "img/fantasy_sprites/skeleton/SHit.png", 150, false);

            easyAnimation_initAnimation(&gameState->skeltonWalk, "skeltonWalk");
            loadAndAddImagesStripToAssets(&gameState->skeltonWalk, "img/fantasy_sprites/skeleton/SWalk.png", 150, false);




        }


        // gameState->cobbleFalling = gameState_findSplatAnimation(gameState, "cobble_falling_11.png");


        // char *tempStr = easy_createString_printf(&globalPerFrameArena, "%d %d %s", 1, 2, "HELLO");

        EntityManager *manager = pushStruct(&globalLongTermArena, EntityManager);
        initEntityManager(manager);

        easyConsole_addToStream(DEBUG_globalEasyConsole, (char *)easyOS_getExeFilePath());


        int canCameraMove = 0;//EASY_CAMERA_MOVE;
        int canCamRotate = 0;

        ///////////************************/////////////////

        Matrix4 cameraLookAt_straight = easy3d_lookAt(v3(0, 0, -10), v3(0, 0, 0), v3(0, 1, 0));


        V4 lightBrownColor = hexARGBTo01Color(0xFFF5DEB3);
        V4 settingsYellowColor = hexARGBTo01Color(0xFFFEF2C2);

        Texture *t_square = findTextureAsset("inventory_equipped.png");
        Texture *particleImage = findTextureAsset("light_03.png");

        V3 itemPosition1 = v3(100, 100, 0.2f);
        V3 itemPosition2 = v3(250, 100, 0.2f);

        bool recording = false;
        float timeSinceLastFrame = 0.0f;

        loadAndAddSoundsToAssets("sounds/engine_sounds/manbytree/", &appInfo->audioSpec);

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
                
                
                // easyConsole_addToStream(DEBUG_globalEasyConsole, shortName);
            }
            free(fullName);
            // free(shortName);
        }

        ////////////////////////

        {
            char *imgFileTypes[] = {"jpg", "jpeg", "png", "bmp", "PNG"};
            char *folderName = concatInArena(globalExeBasePath, "img/tilesets/", &globalPerFrameArena);
            FileNameOfType splatFileNames = getDirectoryFilesOfType(folderName, imgFileTypes, arrayCount(imgFileTypes));
            int splatCount = splatFileNames.count;
            for(int i = 0; i < splatFileNames.count; ++i) {
                char *fullName = splatFileNames.names[i];
                char *shortName = getFileLastPortion(fullName);
                if(shortName[0] != '.') { //don't load hidden file 
                    addElementInfinteAlloc_notPointer(&gameState->splatList_tiles, shortName);

                    Texture texOnStack = loadImage(fullName, TEXTURE_FILTER_LINEAR, true, true);
                    Texture *tex = (Texture *)easyPlatform_allocateMemory(sizeof(Texture), EASY_PLATFORM_MEMORY_ZERO);

                    memcpy(tex, &texOnStack, sizeof(Texture));
                    addElementInfinteAlloc_notPointer(&gameState->splatTextures_tiles, tex);
                }
                free(fullName);
            }
        }




        easyAnimation_initController(&gameState->cauldronAnimationController);
        

        //////////////////////////////////////////////////

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

        gameState->potionModel = findModelAsset_Safe("potion.obj");



        #define LOAD_SCENE_FROM_FILE 1
        #if LOAD_SCENE_FROM_FILE
                //NOTE: Load the player's progress file 
                playerGameState_loadProgress(gameState, manager, appInfo->saveFolderLocation);

                char *sceneNameToLoad = "new";

                if(easyString_getSizeInBytes_utf8(gameState->playerSaveProgress.playerInfo.sceneName) > 0) {
                    sceneNameToLoad = gameState->playerSaveProgress.playerInfo.sceneName;
                }

                gameScene_loadScene(gameState, manager, sceneNameToLoad, weatherState);
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

        easyConsole_pushInt(DEBUG_globalEasyConsole, GLOBAL_transformID_static);

        EasyTransform outlineTransform;
        easyTransform_initTransform_withScale(&outlineTransform, v3(0, 0, 0), v3(1, 1, 1), EASY_TRANSFORM_TRANSIENT_ID); 

        Texture *outlineSprite = findTextureAsset("outline.png");

        Texture *fadeBlackTexture = findTextureAsset("fade_black.png");

        EasyFont_Font *gameFont = globalDebugFont;//easyFont_loadFontAtlas(concatInArena(globalExeBasePath, "fontAtlas_BebasNeue-Regular", &globalPerFrameArena), &globalLongTermArena);

        easyFont_initFontWriter(&gameState->fontWriter, gameFont, 840, easyAudio_findSound("ui_soft.wav"));

        //Fade in for player
        EasySceneTransition *transition = EasyTransition_PushTransition(appInfo->transitionState, 0, 0, EASY_TRANSITION_FADE);
        transition->direction = false;

        float tweakWidth;
        float tweakY;
        float tweakSpacing;

        
        while(appInfo->running) {

            if(refreshTweakFile(tweakerFileName, tweaker)) {
                gameState->jumpPower = getIntFromTweakData(tweaker, "jumpPower");
                gameState->walkPower = getIntFromTweakData(tweaker, "walkPower");
                gameState->gravityScale = (float)getFloatFromTweakData(tweaker, "gravityScale");
                gameState->cameraSnapDistance = (float)getFloatFromTweakData(tweaker, "cameraSnapDistance");

                tweakWidth = getIntFromTweakData(tweaker, "tweakWidth");
                tweakY = getIntFromTweakData(tweaker, "tweakY");
                tweakSpacing = getIntFromTweakData(tweaker, "tweakSpacing");

                gameState->werewolf_attackSpeed = (float)getFloatFromTweakData(tweaker, "werewolf_attackSpeed");
                gameState->werewolf_restSpeed = (float)getFloatFromTweakData(tweaker, "werewolf_restSpeed");

                gameState->werewolf_knockback_distance = (float)getFloatFromTweakData(tweaker, "werewolf_knockback_distance");
                gameState->player_knockback_distance = (float)getFloatFromTweakData(tweaker, "player_knockback_distance");

                global_cameraFollow_yOffset = (float)getFloatFromTweakData(tweaker, "cameraFollow_yOffset");

                for(int i = 0; i < manager->entities.count; ++i) {
                    Entity *e = (Entity *)getElement(&manager->entities, i);
                    if(e && e->rb && e->rb->gravityFactor > 0) {
                        e->rb->gravityFactor = gameState->gravityScale;
                    }
                }
            }


            easyOS_processKeyStates(appInfo, &appInfo->keyStates, resolution, &screenDim, &appInfo->running, !appInfo->hasBlackBars);
            easyOS_beginFrame(resolution, appInfo);
                
            if(!appInfo->keyStates.usingController) {
                gameState->spacePrompt = findTextureAsset("space_prompt.png");
            } else {
                gameState->spacePrompt = findTextureAsset("playstation_x.png");
            }
            
            clearBufferAndBind(shadowMapBuffer.bufferId, COLOR_BLACK, shadowMapBuffer.flags, shadowMapRenderGroup);
            clearBufferAndBind(appInfo->frameBackBufferId, COLOR_BLACK, FRAMEBUFFER_COLOR, 0);
            clearBufferAndBind(mainFrameBuffer.bufferId, COLOR_BLACK, mainFrameBuffer.flags, globalRenderGroup);
            
            renderEnableDepthTest(globalRenderGroup);
            renderEnableCulling(globalRenderGroup);
            setBlendFuncType(globalRenderGroup, BLEND_FUNC_STANDARD_PREMULTIPLED_ALPHA);
            renderSetViewport(globalRenderGroup, 0, 0, resolution.x, resolution.y);


            renderEnableDepthTest(entitiesRenderGroup);
            renderEnableCulling(entitiesRenderGroup);
            setBlendFuncType(entitiesRenderGroup, BLEND_FUNC_STANDARD_PREMULTIPLED_ALPHA);
            renderSetViewport(entitiesRenderGroup, 0, 0, resolution.x, resolution.y);
            

            renderEnableDepthTest(shadowMapRenderGroup);
            renderEnableCulling(shadowMapRenderGroup);
            setBlendFuncType(shadowMapRenderGroup, BLEND_FUNC_STANDARD_PREMULTIPLED_ALPHA);
            renderSetViewport(shadowMapRenderGroup, 0, 0, resolution.x, resolution.y);

            renderSetFrameBuffer(shadowMapBuffer.bufferId, shadowMapRenderGroup);
            renderSetFrameBuffer(mainFrameBuffer.bufferId, entitiesRenderGroup);

            ////////////////////////////////////////////////////////////////////

            u32 moveType = EASY_CAMERA_MOVE_NULL;
            if(!DEBUG_CONTROL_PLAYER) {
                moveType = EASY_CAMERA_MOVE_XY | EASY_CAMERA_MOVE; 
            }

            EasyCamera_MoveType camMove = (EasyCamera_MoveType)(EASY_CAMERA_ZOOM | moveType);

            camMove = (EasyCamera_MoveType)((int)camMove | canCameraMove | canCamRotate);
    
            //FOLLOW PLAYER  
            if(DEBUG_CONTROL_PLAYER) {
                V3 worldP = easyTransform_getWorldPos(&((Entity *)(manager->player))->T);
                {//update x position
                    float distance = absVal(worldP.x - camera.hidden_pos.x);

                    if(distance > gameState->cameraSnapDistance) {
                        float newPosX = lerp(camera.hidden_pos.x, clamp01(appInfo->dt*10.f), worldP.x);
                        camera.hidden_pos.x = newPosX;
                    }
                }

                {//update y position
                    float distance = absVal(worldP.y - global_cameraFollow_yOffset - camera.hidden_pos.y);

                    if(distance > gameState->cameraSnapDistance) {
                        float newPosY = lerp(camera.hidden_pos.y, clamp01(appInfo->dt*10.f), worldP.y - global_cameraFollow_yOffset);
                        camera.hidden_pos.y = newPosY;
                    }
                }
            } else {
                if(!editorState->entitySelected) {
                    editorState->lerpTowardsTimer = -1;   
                }

                if(editorState->lerpTowardsTimer >= 0) {
                    editorState->lerpTowardsTimer += appInfo->dt;

                    float canVal = editorState->lerpTowardsTimer / 0.5f;

                    Entity *e = (Entity *)editorState->entitySelected;
                    camera.pos.xy = lerpV2(editorState->startLerpP, smoothStep01(0, canVal, 1), v3_minus(easyTransform_getWorldPos(&e->T), v3(0, global_cameraFollow_yOffset, 0)).xy);
                    camera.hidden_pos.xy = camera.pos.xy;

                    if(canVal >= 1) {
                        editorState->lerpTowardsTimer = -1;
                    }
                }
            }
            ////////////////


            AppKeyStates cameraKeyStates = appInfo->keyStates;
            if(appInfo->editor->isHovering) {
                cameraKeyStates.scrollWheelY = 0;
            }
            easy3d_updateCamera(&camera, &cameraKeyStates, 1, 1000.0f, appInfo->dt, camMove);

            easy_setEyePosition(globalRenderGroup, camera.pos);
                

            // update camera first
            Matrix4 viewMatrix = easy3d_getWorldToView(&camera);

            //cast ray against floor to find reference point
            {

                V3 hitP;
                float tAt;

                EasyPlane floor = {};

                floor.origin = v3(0, 0, 0);
                floor.normal = v3(0, 0, -1);

                EasyRay cameraRay = {};
                cameraRay.origin = camera.pos;
                cameraRay.direction = EasyCamera_getZAxis(&camera);


                if(easyMath_castRayAgainstPlane(cameraRay, floor, &hitP, &tAt)) {

                    float sunX = 80*(2*weatherState->timeOfDay - 1.f);
                    float sunZ = 20*sin(weatherState->timeOfDay*PI32);
                    light->T->pos = v3_plus(hitP, v3(sunX, -20, -sunZ));
                    // easyConsole_pushV3(DEBUG_globalEasyConsole, light->T->pos);
                    // easyConsole_pushV3(DEBUG_globalEasyConsole, camera.pos);

                    light->rayDirection_towardsLight_inWorldSpace = normalizeV3(v3_minus(light->T->pos, hitP));


                    //the sun is just the same position of the camera for now
                    light->worldToLightSpace = Mat4Mult(OrthoMatrixToScreen(20, 20*appInfo->aspectRatio_yOverX), easy3d_lookAt(light->T->pos, hitP, v3(0, SIN45, -SIN45)));
                } else {
                    //Has to hit the ground 
                    assert(false);
                }

            }
           


            setViewTransform(shadowMapRenderGroup, mat4());
            setProjectionTransform(shadowMapRenderGroup, light->worldToLightSpace);

            float zoom = 50.0f*(camera.zoom / 90.0f);
            Matrix4 perspectiveMatrix = projectionMatrixFOV(camera.zoom, resolution.x/resolution.y);

            V2 orthoResolution = v2(zoom, zoom*(resolution.y/resolution.x));
            if(DEBUG_USE_ORTHO_MATRIX) {
                perspectiveMatrix = OrthoMatrixToScreen(orthoResolution.x, orthoResolution.y);
            }

            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_1) && !gameState->isEditorOpen) {
                gameState->gameModeType = GAME_MODE_PAUSE_MENU;
                //Open  close menu sound
                playGameSound(&globalLongTermArena, gameState->openMenuSound, 0, AUDIO_BACKGROUND);
            }



            if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_I) && gameState->gameModeType == GAME_MODE_PLAY) {
                gameState->isLookingAtItems = !gameState->isLookingAtItems;
                // gameState->lookingAt_animTimer.current = UI_ITEM_RADIUS_MIN;
                // gameState->lookingAt_animTimer.target = UI_ITEM_RADIUS_MAX;

                gameState->inventoryBreathSelector = 0;

                if(gameState->gameIsPaused) { gameState->gameIsPaused = false; }

                gameState->indexInItems = 0;
                gameState->animationItemTimers[0].target = UI_ITEM_PICKER_MAX_SIZE;

                gameState->inventoryMenuType = GAME_INVENTORY_MENU_MAIN;

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

                switch(gameState->inventoryMenuType) {
                    case GAME_INVENTORY_MENU_MAP: {
                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT)) {
                            gameState->inventoryMenuType = GAME_INVENTORY_MENU_MAIN;
                        }

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_RIGHT)) {
                            gameState->inventoryMenuType = GAME_INVENTORY_MENU_ITEMS;
                        }
                    } break;
                    case GAME_INVENTORY_MENU_ITEMS: {
                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT)) {
                            gameState->inventoryMenuType = GAME_INVENTORY_MENU_MAP;
                        }
                    } break;
                    case GAME_INVENTORY_MENU_MAIN: {
                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_LEFT) && (gameState->indexInItems  != 0 && gameState->indexInItems  != 4 && gameState->indexInItems  != 8)) {
                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;


                            gameState->indexInItems--;
                            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);

                            easyConsole_pushInt(DEBUG_globalEasyConsole, gameState->indexInItems);
                            gameState->inventoryBreathSelector = 0;

                            if(gameState->indexInItems < 0) {
                                gameState->indexInItems = 0;
                            }

                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                        }

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_RIGHT) && (gameState->indexInItems == 3 || gameState->indexInItems == 7 || gameState->indexInItems == 11)) {
                            //go to the Map from here
                            gameState->inventoryMenuType = GAME_INVENTORY_MENU_MAP;
                        }

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_RIGHT) && (gameState->indexInItems  != 3 && gameState->indexInItems != 7 && gameState->indexInItems  != 11)) {
                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                            gameState->inventoryBreathSelector = 0;


                            gameState->indexInItems++;
                            easyConsole_pushInt(DEBUG_globalEasyConsole, gameState->indexInItems);
                            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
                            if(gameState->indexInItems >= arrayCount(gameState->itemSpots)) {
                                gameState->indexInItems = arrayCount(gameState->itemSpots) - 1;

                            }

                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                        }

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_UP) && (gameState->indexInItems + 4) < 12) {
                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                            gameState->indexInItems += 4;
                            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);

                            gameState->inventoryBreathSelector = 0;

                            if(gameState->indexInItems < 0) {
                                gameState->indexInItems = 0;
                            }

                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                        }

                        if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_DOWN) && (gameState->indexInItems - 4) >= 0) {
                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MIN_SIZE;

                            gameState->inventoryBreathSelector = 0;

                            gameState->indexInItems -= 4;
                            playGameSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_BACKGROUND);
                            if(gameState->indexInItems >= arrayCount(gameState->itemSpots)) {
                                gameState->indexInItems = arrayCount(gameState->itemSpots) - 1;
                            }

                            gameState->animationItemTimers[gameState->indexInItems].target = UI_ITEM_PICKER_MAX_SIZE;
                        }
                    } break;
                }
                
            } else  { //if(!gameState->gameIsPaused)
                EasyPhysics_UpdateWorld(&gameState->physicsWorld, appInfo->dt);    
                
                //NOTE: Not sure if this is the best place to do this but we make sure the player isn't outside of the world based on the tile grid

                Entity *player = (Entity *)manager->player;
                V3 playerP = easyTransform_getWorldPos(&player->T);

                if(!player->aiController) {
                    player->aiController = easyAi_initController(&globalPerSceneArena);
                }   

                EasyAi_Node *node = easyAi_getClosestNode(player->aiController, playerP);

                if(node) {
                    V3 closestP = node->pos;
                    
                    V3 diffP = v3_minus(playerP, closestP);

                    //NOTE: check if the player is outside the square first
                    if(inRect(diffP.xy, rect2fCenterDim(0, 0, GLOBAL_WORLD_TILE_SIZE, GLOBAL_WORLD_TILE_SIZE))) {
                        //NOTE: in Square so safe 
                    } else {
                        //NOTE: Out of bounds so put player back in
                        //NOTE: Now find the _closest direction vector_ to move the player back into safety and find the _wall normal_ of the square.

                        V3 normal = {};
                        V3 directionVector = {};

                        int cornerType = -1;

                        diffP.z = 0;

                        float halfTileSize = 0.5f*GLOBAL_WORLD_TILE_SIZE;

                        //Find the closest edge by seperating the square into 8 different quadrants - 4 for the sides and 4 for the corners
                        if(diffP.x < -halfTileSize) {
                            // can be in three guadrants - one side and two corners - now find which one
                            if(diffP.y < -halfTileSize) {
                                //bottom left corner
                                cornerType = 0;
                            } else if(diffP.y >= halfTileSize) {
                                //top left corner
                                cornerType = 1;
                            } else {
                                //left side
                                normal = v3(1, 0, 0);
                                directionVector = v3(absVal(diffP.x) - halfTileSize, 0, 0);
                            }
                        } else if(diffP.x >= halfTileSize) {
                            // can be in three guadrants - one side and two corners - now find which one
                            if(diffP.y < -halfTileSize) {
                                //bottom right corner
                                cornerType = 2;
                            } else if(diffP.y >= halfTileSize) {
                                //top right corner
                                cornerType = 3;
                            } else {
                                //right side
                                normal = v3(-1, 0, 0);
                                directionVector = v3(-(diffP.x - halfTileSize), 0, 0);
                            }
                        } else if(diffP.y < -halfTileSize) {
                            // can be in three guadrants - one side and two corners - now find which one
                            if(diffP.x < -halfTileSize) {
                                //bottom left corner
                                cornerType = 0; 
                            } else if(diffP.x >= halfTileSize) {
                                //bottom right corner
                                cornerType = 2;
                            } else {
                                //bottom side
                                normal = v3(0, 1, 0);
                                directionVector = v3(0, absVal(diffP.y) - halfTileSize, 0);
                            }
                        } else if(diffP.y >= halfTileSize) {
                            // can be in three guadrants - one side and two corners - now find which one
                            if(diffP.x < -halfTileSize) {
                                //top left corner
                                cornerType = 1;
                            } else if(diffP.x >= halfTileSize) {
                                //top right corner
                                cornerType = 3;
                            } else {
                                //top side
                                normal = v3(0, -1, 0);
                                directionVector = v3(0, -(diffP.y - halfTileSize), 0);
                            }
                        }

                        if(cornerType == 0) {
                            //bottom left corner
                            V3 localP = v3_minus(v3(-0.5f*GLOBAL_WORLD_TILE_SIZE, -0.5f*GLOBAL_WORLD_TILE_SIZE, 0), diffP);
                            normal = normalizeV3(localP);
                            directionVector = localP;
                        } else if(cornerType == 1) {
                            //top left corner
                            V3 localP = v3_minus(v3(-0.5f*GLOBAL_WORLD_TILE_SIZE, 0.5f*GLOBAL_WORLD_TILE_SIZE, 0), diffP);
                            normal = normalizeV3(localP);
                            directionVector = localP;
                        } else if(cornerType == 2) {
                            //bottom right corner
                            V3 localP = v3_minus(v3(0.5f*GLOBAL_WORLD_TILE_SIZE, -0.5f*GLOBAL_WORLD_TILE_SIZE, 0), diffP);
                            normal = normalizeV3(localP);
                            directionVector = localP;
                        } else if(cornerType == 3) {
                            //top right corner
                            V3 localP = v3_minus(v3(0.5f*GLOBAL_WORLD_TILE_SIZE, 0.5f*GLOBAL_WORLD_TILE_SIZE, 0), diffP);
                            normal = normalizeV3(localP);
                            directionVector = localP;
                        }

                        directionVector.z = 0;

                        player->T.pos = v3_plus(player->T.pos, directionVector);
                        // player->rb->dP = v3_plus(player->rb->dP, v3_scale(dotV3(player->rb->dP, normal), normal));
                    }   
                }

            }

            setViewTransform(globalRenderGroup, viewMatrix);
            setProjectionTransform(globalRenderGroup, perspectiveMatrix);

            setViewTransform(entitiesRenderGroup, viewMatrix);
            setProjectionTransform(entitiesRenderGroup, perspectiveMatrix);


            RenderProgram *mainShader = &glossProgram;
            renderSetShader(globalRenderGroup, mainShader);

            AppKeyStates gameKeyStates = appInfo->keyStates;
            AppKeyStates consoleKeyStates = appInfo->keyStates;
            if(appInfo->console.isInFocus || canCameraMove || gameState->gameModeType == GAME_MODE_PAUSE_MENU || gameState->gameIsPaused) {
                gameKeyStates = {};
            } else if(easyConsole_isOpen(&appInfo->console)) {
                consoleKeyStates = {};
                consoleKeyStates.gameButtons[BUTTON_ESCAPE] = gameKeyStates.gameButtons[BUTTON_ESCAPE];
            }
            
            //Update the time of day
            {
                weatherState->timeOfDay += weatherState->timeOfDaySpeed*appInfo->dt;
                while(weatherState->timeOfDay >= 1.0f) {
                    weatherState->timeOfDay -= 1.0f;
                }    
            }
            

            //SET DAY TIME COLOR

            V4 dayColor = v4(1, 1, 1, 1);

            if(getTimeOfDay(weatherState) > 18) {
                // dayColor = v4(1, 0.5, 0, 1); 
            }

            float minColor = 0.4f;

            float timeDiff = 1.0f - (2*absVal(weatherState->timeOfDay - 0.5f)); //expand from -0.5f - 0.5f to between 0 - 1


            float time = clamp(0.4f, timeDiff, 1);
            dayColor = v4_scale(time, dayColor);
            dayColor.w = 1;

            if(!gameState->useSunlightColor) {
                dayColor = v4_scale(0.5f, v4(1, 1, 1, 1));    
                dayColor.w = 1;
            } 

            globalRenderGroup->timeOfDayColor = dayColor;
            entitiesRenderGroup->timeOfDayColor = dayColor;

            ///////////////////////////////////////////////
           
           V3 mouseP_inWorldP = screenSpaceToWorldSpace(perspectiveMatrix, gameKeyStates.mouseP_left_up, resolution, -camera.pos.z, easy3d_getViewToWorld(&camera));
           EasyRay mouseP_worldRay = {};
           mouseP_worldRay.origin = camera.pos;
           mouseP_worldRay.direction = v3_minus(mouseP_inWorldP, camera.pos);

           if(DEBUG_USE_ORTHO_MATRIX) {
                mouseP_worldRay.origin = screenSpaceToWorldSpace_orthographicView(gameKeyStates.mouseP_left_up, resolution, orthoResolution, EasyCamera_getXAxis(&camera), EasyCamera_getYAxis(&camera), camera.pos);
                mouseP_worldRay.direction = EasyCamera_getZAxis(&camera);
           }

           gameState->rotationUpdate += appInfo->dt;

           if(gameState->rotationUpdate > TAU32) {
                gameState->rotationUpdate -= TAU32;
           }

           //  /////////////////

             Entity *insideEntity = 0;
            //DEBUG
            // if(true)
            {

                
                int idCount = 0;
                EditorEntitySelectInfo selectInfos[128];

                bool found = false;
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
                            if(castInfo.didHit && e->type != ENTITY_TERRAIN && !found) {
                                insideEntity = e;

                                if(!appInfo->editor->isHovering && wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && editorState->createMode == EDITOR_CREATE_SELECT_MODE) {
                                    if(!editorState_idInList(editorState, e->T.id)) {

                                        editorState->entitySelected = e;
                                        editorState->entityIndex = i;

                                        lerpToSelectedEntity(editorState, camera.pos.xy);

                                        editorState->grabOffset = v3_minus(position, castInfo.hitP);

                                        if(!editorState_addToEndOfList(editorState, e->T.id)) { easyFlashText_addText(&globalFlashTextManager, "Ran out of room in editor selected list"); }

                                        found = true;
                                        idCount = 0;
                                        easyConsole_pushV3(DEBUG_globalEasyConsole, editorState->grabOffset);
                                    } else {
                                        assert(!found);
                                        //Keep looking for one with haven't selected recently. BUt add to list just in case
                                        assert(idCount < arrayCount(selectInfos));

                                        EditorEntitySelectInfo *info = selectInfos + idCount++;

                                        info->e = e;
                                        info->hitP = castInfo.hitP;
                                        info->id = e->T.id;
                                        info->index = i;
                                        info->entP = position;
                                    }
                                }    
                            }

                        }


                        //Draw the DEBUG collider info 
                        if(e->collider && DEBUG_DRAW_COLLISION_BOUNDS) {   
                            EasyTransform prevT = e->T;

                            e->T.scale.x *= e->collider->dim2f.x;
                            e->T.scale.y *= e->collider->dim2f.y;

                            e->T.pos.x += e->collider->offset.x;
                            e->T.pos.y += e->collider->offset.y; 

                            Quaternion Q = e->T.Q;

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
                            
                            
                            e->T = prevT;
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

                if(idCount > 0) {
                    //We've selected all entities before, so find the _least_recent_ id on the list, which is at index zero
                    assert(editorState->idsLastSelectedCount > 0);

                    int newId = 0;

                    /////Find the right id

                    int lowestIndex = INT_MAX;

                    //The id has to be in selectInfos, so loop through all ids in select info and find the lowest one on the list 
                     EditorEntitySelectInfo *info = 0;
                    for(int i = 0; i < idCount; ++i) {
                        EditorEntitySelectInfo *a = selectInfos + i;

                        bool found = false;
                        for(int j = 0; j < editorState->idsLastSelectedCount && !found; j++) {
                            if(editorState->idsLastSelected[j] == a->id && j < lowestIndex) { //check if it is the lowest index so far
                                lowestIndex = j;
                                newId = a->id;
                                info = a;
                                found = true;
                            }
                        }
                    }

                    assert(info);

                    ///////////////////////////////////

                    editorState_moveToEnd(editorState, newId);
                    

                    //Grab new entity
                    lerpToSelectedEntity(editorState, camera.pos.xy);
                    editorState->entitySelected = info->e;
                    editorState->entityIndex = info->index;

                    editorState->grabOffset = v3_minus(info->entP, info->hitP);
                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
                renderClearDepthBuffer(mainFrameBuffer.bufferId);

                // if(wasReleased(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                //     letGoOfSelectedEntity(editorState);
                // }

                
            }
           //  //////////////////////////////////////////
           //Draw the square to where the mouse is pointing
           if(gameState->isEditorOpen) {
               V3 hitP;
               float tAt;

               EasyPlane floor = {};

               floor.origin = v3(0, 0, 0);
               floor.normal = v3(0, 0, -1);

               if(easyMath_castRayAgainstPlane(mouseP_worldRay, floor, &hitP, &tAt)) {
                    float tileScale = 1;
                    if(editorState->createMode == EDITOR_CREATE_TILE_MODE) {
                       tileScale = GLOBAL_WORLD_TILE_SIZE;
                    }

                   
                   hitP = roundToGridBoard(hitP, tileScale);

                   hitP.z = -0.1f;
                   outlineTransform.pos = hitP;
                   
                   outlineTransform.scale = v3(tileScale, tileScale, tileScale);

                   // easyConsole_pushFloat(DEBUG_globalEasyConsole, outlineTransform.pos.y);

                   Matrix4 outlineT = easyTransform_getTransform(&outlineTransform);
                   setModelTransform(globalRenderGroup, outlineT);
                   renderDrawSprite(globalRenderGroup, outlineSprite, COLOR_WHITE);

                   drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
               }
           }

           ////////////////////////////

           float t_24hr = getTimeOfDay(weatherState);
           //Collect all the lights before rendering terrain
           for(int i = 0; i < manager->entities.count; ++i) {
               Entity *e = (Entity *)getElement(&manager->entities, i);
               if(e) {
                    bool timeOfDay = (t_24hr > 18 || t_24hr < 6);

                    //NOTE: Always push lights when not using sunlight color
                   if(!gameState->useSunlightColor) {
                     timeOfDay = true;
                   }

                   if((e->type == ENTITY_LAMP_POST && timeOfDay) || ((e->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE || e->triggerType == ENTITY_TRIGGER_FIRE_POST) && e->chestIsOpen)) {
                       float perlinFactor = lerp(0.4f, perlin1d(globalTimeSinceStart*0.1f, 10, 4), 1.0f);

                       float extraIntensity = 1;
                       if(e->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE || e->triggerType == ENTITY_TRIGGER_FIRE_POST) {
                            float diff_from_12 = absVal(12 - t_24hr);
                            extraIntensity = clamp01(diff_from_12 / 6);

                            if(!gameState->useSunlightColor) {
                                extraIntensity = 1;
                            }
                       }

                       V3 lightPos = v3_plus(easyTransform_getWorldPos(&e->T), v3(0, 0, -1.5f));
                       easyRender_push2dLight(globalRenderGroup, lightPos, e->lightColor, extraIntensity*e->lightIntensity*perlinFactor, e->innerRadius, e->outerRadius);
                       easyRender_push2dLight(entitiesRenderGroup, lightPos, e->lightColor, extraIntensity*e->lightIntensity*perlinFactor, e->innerRadius, e->outerRadius);
                   }    

               }
               
           }


           renderDisableBatchOnZ(globalRenderGroup);
           renderDisableBatchOnZ(entitiesRenderGroup);

           // if(isDown(gameKeyStates.gameButtons, BUTTON_CTRL)) {
                // easyConsole_addToStream(DEBUG_globalEasyConsole, "ctrl doen");
           // }
           ////////////////////////
           //NOTE: Update the shops
           if(gameState->gameIsPaused) {
                updateShop(gameState->townShop, appInfo->dt);
                updateCrafting(gameState->crafting, appInfo->dt);
           }

           for(int i = 0; i < manager->entities.count; ++i) {
               Entity *e = (Entity *)getElement(&manager->entities, i);
               if(e) {
                   updateEntity(gameFont, manager, e, gameState, appInfo->dt, &gameKeyStates, &appInfo->console, &camera, ((Entity *)(manager->player)), gameState->gameIsPaused, EasyCamera_getZAxis(&camera), appInfo->transitionState, globalSoundState, editorState, shadowMapRenderGroup, entitiesRenderGroup, weatherState, appInfo->saveFolderLocation);        
                
                   if(e->isDead) {
                       ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteForFrame);
                       int *indexToAdd = (int *)arrayInfo.elm;
                       *indexToAdd = i;
                   }
               }
           }

           
            drawRenderGroup(shadowMapRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
           

           /////////////////////////////////////////////////////////////////////


           //  //DRAW THE TERRATIN FIRST
            if(gameState->currentTerrainEntity && DEBUG_DRAW_TERRAIN) {
                Entity *terrainEntity = (Entity *)(gameState->currentTerrainEntity);
                setModelTransform(globalRenderGroup, easyTransform_getTransform(&terrainEntity->T));
                renderDrawTerrain2d(globalRenderGroup, v4(1, 1, 1, 1), &gameState->terrainPacket);
                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
            }

            if(DEBUG_DRAW_TERRAIN) {
                Texture *sprite = 0;
                
                gameState->tempTransform.Q = identityQuaternion();

                gameState->tempTransform.scale = v3(GLOBAL_WORLD_TILE_SIZE, GLOBAL_WORLD_TILE_SIZE, 1);

                renderSetShader(globalRenderGroup, &pixelArtProgram);

                for(int i = 0; i < gameState->tileSheet.tileCount; ++i) {
                    bool draw = true;

                    WorldTile *t = gameState->tileSheet.tiles + i;

                    gameState->tempTransform.pos.x = t->x;
                    gameState->tempTransform.pos.y = t->y;
                    gameState->tempTransform.pos.z = t->z;

                    if(t->z < 0) {
                        //breakl
                        int h = 0;
                    }

                    if(t->type == WORLD_TILE_NULL) {
                        sprite = &globalPinkTexture;
                    } else if(t->type == WORLD_TILE_GRASS) {
                        sprite = findTextureAsset("grass_tile.png");
                    } else if(t->type == WORLD_TILE_WOOD_FLOOR) {
                        sprite = findTextureAsset("wood_floor_tile.png");
                    } else if(t->type == WORLD_TILE_BRICK) {
                        sprite = findTextureAsset("brick.png");
                    } else if(t->type == WORLD_TILE_DIRT) {
                        sprite = findTextureAsset("dirt_tile.png");

                    } else if(t->type == WORLD_TILE_LAVA) {
                        sprite = findTextureAsset("lava_tile.png");
                    } else if(t->type == WORLD_TILE_ROCK) {
                        sprite = findTextureAsset("lava_tile.png");

                    } else if(t->type == WORLD_TILE_COBBLE) {
                        sprite = findTextureAsset("cobble.png");

                        //assign the normal map id
                        if(sprite->normalMapId <= 0) {
                            Texture *normalMap = findTextureAsset("cobble_n.png");
                            assert(normalMap);
                            sprite->normalMapId = normalMap->id;
                        }
                        
                    } else if(t->type == WORLD_TILE_PATH) {
                        sprite = findTextureAsset("grass_road.png");
                    } else if(t->type == WORLD_TILE_PATH1) {
                        sprite = findTextureAsset("grass_road_horzintoal.png");
                    } else if(t->type == WORLD_TILE_PATH2) {
                        sprite = findTextureAsset("grass_road_corner.png");
                    } else if(t->type == WORLD_TILE_PATH3) {
                        sprite = findTextureAsset("grass_road_corner-bottom.png");
                    } else if(t->type == WORLD_TILE_PATH4) {
                        sprite = findTextureAsset("grass_road_corner-right.png");
                    } else if(t->type == WORLD_TILE_PATH5) {
                        sprite = findTextureAsset("grass_road_corner-left.png");
                    } else if(t->type == WORLD_TILE_BEACH) {
                        sprite = findTextureAsset("sand.png");
                    } else if(t->type == WORLD_TILE_SEA || t->type == WORLD_TILE_SEA1) {
                        if(easyAnimation_isControllerEmpty(&t->animationController)) {
                            easyAnimation_addAnimationToController(&t->animationController, &gameState->animationFreeList, &gameState->seaTileAnimation, 0.5f);    
                        }

                        char *animationFileName = easyAnimation_updateAnimation(&t->animationController, &gameState->animationFreeList, appInfo->dt);
                        sprite = findTextureAsset(animationFileName);   
                        
                    } else if(t->type == WORLD_TILE_BEACH_GRASS) {
                        sprite = findTextureAsset("sand-grass.png");
                    } else if(t->type == WORLD_TILE_SAND_WATER) {

                        if(easyAnimation_isControllerEmpty(&t->animationController)) {
                            easyAnimation_addAnimationToController(&t->animationController, &gameState->animationFreeList, &gameState->sandWaterTileAnimation, 0.8f);    
                        }

                        char *animationFileName = easyAnimation_updateAnimation(&t->animationController, &gameState->animationFreeList, appInfo->dt);
                        sprite = findTextureAsset(animationFileName);   
                    } else if(t->type == WORLD_TILE_SAND_WATER1) {
                        sprite = findTextureAsset("sand-water2.png");
                    } else if(t->type == WORLD_TILE_PIER_MIDDLE) {
                     sprite = findTextureAsset("pier_middle_middle.png");   
                    } else if(t->type == WORLD_TILE_PIER_SAND) {
                        sprite = findTextureAsset("pier_middle_middle-end-sand.png");
                    } else if(t->type == WORLD_TILE_PIER_SIDE_LEFT) {
                        sprite = findTextureAsset("piermiddle.png");
                     } else if(t->type == WORLD_TILE_PIER_SIDE_RIGHT) {
                        sprite = findTextureAsset("piermiddle-right.png");
                    } else if(t->type == WORLD_TILE_PIER_SAND_CORNER_LEFT) {
                        sprite = findTextureAsset("pier-top-left-sand.png");
                    } else if(t->type == WORLD_TILE_PIER_SAND_CORNER_RIGHT) {
                        sprite = findTextureAsset("pier-top-right-sand.png");
                    } else if(t->type == WORLD_TILE_PIER_SEA_CORNER_RIGHT) {
                        sprite = findTextureAsset("piercorner.png");
                    } else if(t->type == WORLD_TILE_PIER_SEA_CORNER_LEFT) {
                        sprite = findTextureAsset("piermiddle_right_corner.png");
                    } else if(t->type == WORLD_TILE_PATH_NUB1) {
                        sprite = findTextureAsset("grass_road_nub.png");
                    } else if(t->type == WORLD_TILE_PATH_NUB2) {
                        sprite = findTextureAsset("grass_road_nub_bottom.png");
                    }   else if(t->type == WORLD_TILE_PATH_NUB3) {
                        sprite = findTextureAsset("grass_road_nub_left.png");
                    }   else if(t->type == WORLD_TILE_PATH_NUB4) {
                        sprite = findTextureAsset("grass_road_nub_right.png");
                    }      

                    V4 color = COLOR_WHITE;

                    if(t->rotation == WORLD_TILE_ROTATION_FLAT) {
                        gameState->tempTransform.Q = identityQuaternion();
                    } else if(t->rotation == WORLD_TILE_ROTATION_UP) {
                        gameState->tempTransform.Q = gameState->angledQ;;//eulerAnglesToQuaternion(-PI32, 0, 0);

                        Entity *playerEnt = (Entity *)manager->player;

                        //NOTE: If we want the standing tiles to fade out a bit
                        // if(playerEnt->T.pos.y > t->yAxis) {
                        //     color.w = 0.3f;

                        //     EntityRender_Alpha alphaStruct = {};
                        //     alphaStruct.sprite = sprite;
                        //     alphaStruct.T = easyTransform_getTransform(&gameState->tempTransform);
                        //     alphaStruct.shader = &pixelArtProgram;
                        //     alphaStruct.colorTint = color; 

                        //     addElementInfinteAlloc_notPointer(&gameState->alphaSpritesToRender, alphaStruct);

                        //     draw = false;
                            
                        // } 

                    } 

                    if(draw) {
                        setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
                        renderDrawSprite(globalRenderGroup, sprite, color);
                    }
                }
                
                gameState->tempTransform.Q = gameState->angledQ;

            }

   
            {
                for(int i = 0; i < manager->entities.count; ++i) {
                    Entity *e = (Entity *)getElement(&manager->entities, i);
                    if(e && e->renderFirstPass) {
                        Matrix4 T = easyTransform_getTransform(&e->T);
                        setModelTransform(globalRenderGroup, T);
                        if(e->sprite && DEBUG_DRAW_SCENERY_TEXTURES) { renderSetShader(globalRenderGroup, &pixelArtProgram); renderDrawSprite(globalRenderGroup, e->sprite, e->colorTint); }

                    }
                }
            }


            if(DEBUG_AI_BOARD_FOR_ENTITY && editorState->entitySelected) {
                Entity *e = (Entity *)editorState->entitySelected;

                if(e->aiController) {
                    //Draw the ai board
                    EasyAiController *aiController = e->aiController;
                    for(int i = 0; i < arrayCount(aiController->boardHash); ++i) {
                        EasyAi_Node *n = aiController->boardHash[i];

                        while(n) {

                            Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(1, 1, 0)), v3(n->pos.x, n->pos.y, -0.05)); 
                            setModelTransform(globalRenderGroup, T);
                            renderDrawQuad(globalRenderGroup, n->canSeePlayerFrom ? COLOR_GOLD : COLOR_AQUA);

                            n = n->next;
                        }
                    }

                    for(int i = 0; i < aiController->searchBouysCount; ++i) {
                        V3 *value = &aiController->searchBouys[i];

                        Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(0.5f, 0.5f, 0)), v3(value->x, value->y, -0.1)); 
                        setModelTransform(globalRenderGroup, T);
                        renderDrawQuad(globalRenderGroup, COLOR_BLUE);
                    }

                    
                }
            }


            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            // renderEnableBatchOnZ(globalRenderGroup);
            
            drawRenderGroup(entitiesRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
            

            //////////////////// DRAW ALPHA ENTITIES //////////////////////////////////////////

            for(int i = 0; i < gameState->alphaSpritesToRender.count; ++i) {
                EntityRender_Alpha *e = getElementFromAlloc(&gameState->alphaSpritesToRender, i, EntityRender_Alpha);

                setModelTransform(globalRenderGroup, e->T);
                renderSetShader(globalRenderGroup, e->shader); 
                renderDrawSprite(globalRenderGroup, e->sprite, e->colorTint);

            } 
            gameState->alphaSpritesToRender.count = 0;
            /////////////////////////////////////////////////

            //Render the object the player got from a chest
            if(gameState->gameModeType == GAME_MODE_ITEM_COLLECT) {

                gameState->tempTransform_model.pos = v3_plus(easyTransform_getWorldPos(&((Entity *)(manager->player))->T), v3(0, 0, -1.0f));
                float scale = 0.4f;
                gameState->tempTransform_model.scale = v3(scale, scale, scale);
                gameState->tempTransform_model.Q = eulerAnglesToQuaternion(0, -0.25f*PI32, 0);

                gameState->tempTransform_model.Q = quaternion_mult(gameState->tempTransform_model.Q, eulerAnglesToQuaternion(gameState->rotationUpdate, 0, 0));
                // 

                setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform_model));
                renderSetShader(globalRenderGroup, &pixelArtProgram);
                renderDrawSprite(globalRenderGroup, getInvetoryTexture(gameState->itemCollectType), COLOR_WHITE);
            }
            ///////////////////////////////////

            renderDisableBatchOnZ(globalRenderGroup);

            //Draw any particle systems
            {   

                    //DRAW THE PARTICLE SYSTEM
                    renderSetShader(globalRenderGroup, &textureProgram);
                    V3 worldP = v3_plus(easyTransform_getWorldPos(&((Entity *)(manager->player))->T), v3(0, 0, -1.5f));
                    drawAndUpdateParticleSystem(globalRenderGroup, &gameState->playerUseItemParticleSystem, worldP, appInfo->dt, v3(0, 0, 0), gameState->playerPotionParticleSystemColor, true);
                    ///////////////////////////////////////////

                    //////////////////////////////////////////////////////
                    // easyConsole_pushInt(DEBUG_globalEasyConsole, manager->activeParticleSystems.count);
                    for(int i = 0; i < manager->activeParticleSystems.count; ++i) {
                        ParticleSystemListItem *ps = (ParticleSystemListItem *)getElement(&manager->activeParticleSystems, i);
                        if(ps) {
                            V3 worldP = v3_plus(*ps->position, ps->offset);
                            drawAndUpdateParticleSystem(globalRenderGroup, &ps->ps, worldP, appInfo->dt, v3(0, 0, 0), ps->color, true);

                            if(!ps->ps.Active) {
                                removeElement_ordered(&manager->activeParticleSystems, i);
                            }
                        }
                    }
                    /////////////////////////////////////////////////////////

            }

            
            
            drawRenderGroup(entitiesRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
            
            renderEnableBatchOnZ(globalRenderGroup);
            renderEnableBatchOnZ(entitiesRenderGroup);


            { //NOTE: Update the damage numbers
                for(int i = 0; i < manager->damageNumbers.count; ++i) {
                    Entity_DamageNumber *num = (Entity_DamageNumber *)getElement(&manager->damageNumbers, i);

                    if(num) {
                        num->aliveTimer += appInfo->dt;

                        float canVal = num->aliveTimer / (num->totalLifeFraction*1.0f);

                        char *at = num->str;

                        float xAt = 0; 

                        float fontSize_to_worldSize = num->size*(GLOBAL_DEFINE_defaultMetersPerLetter_floatingText / (float)gameFont->fontHeight);

                        num->pos = v3_plus(num->pos, v3_scale(num->speed*appInfo->dt, v3(0, 0, -3)));  

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

                        Entity *e1 = 0;
                        if(e->type == ENTITY_HEALTH_POTION_1) {

                        
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



                            e1 = initEntity(manager, &gameState->firePitAnimation, e->position, size, v2(0.9f, 0.9f), gameState, e->type, inverse_weight, t, COLOR_WHITE, layer, true);
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

                            
                        } else {
                           e1 = initEntityOfType(gameState, manager, e->position, 0, e->type, e->subType, false, ENTITY_TRIGGER_NULL, 0, 0);
                           if(e1->rb) {
                                e1->rb->dP = e->dP;
                           }

                           if(e->rotation > 0) {
                            e1->T.Q = quaternion_mult(gameState->angledQ, eulerAnglesToQuaternion(0, 0, e->rotation));
                           }
                        }

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

                        if(e->collider1) {
                            EasyPhysics_removeCollider(&gameState->physicsWorld, e->collider1);
                        }

                        if(e->collider2) {
                            EasyPhysics_removeCollider(&gameState->physicsWorld, e->collider2);
                        }

                        if(e->collider3) {
                            EasyPhysics_removeCollider(&gameState->physicsWorld, e->collider3);
                        }

                        if(e->collider4) {
                            EasyPhysics_removeCollider(&gameState->physicsWorld, e->collider4);
                        }

                        if(e->rb) {
                            EasyPhysics_removeRigidBody(&gameState->physicsWorld, e->rb);
                        }


                        if(e->particleSystemIndexToEndOnDelete >= 0) {
                            ParticleSystemListItem *ps = (ParticleSystemListItem *)getElement(&manager->activeParticleSystems, e->particleSystemIndexToEndOnDelete);
                            if(ps) {
                                ps->ps.Active = false;
                            }
                        }

                        removeElement_ordered(&manager->entities, *indexToDelete);
                    }
                }

                easyArray_clear(&manager->entitiesToDeleteForFrame);
            }   



            if(!appInfo->console.isInFocus && wasPressed(appInfo->keyStates.gameButtons, BUTTON_F5)) {
                gameState->isEditorOpen = !gameState->isEditorOpen;
            }

            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            ////////////// DRAW FOG //////////////////////
            EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);

            renderSetShader(globalRenderGroup, &fogProgram); 
            for(int i = 0; i < manager->entities.count; ++i) {
                Entity *e = (Entity *)getElement(&manager->entities, i);
                if(e && e->type == ENTITY_FOG) {
                   setModelTransform(globalRenderGroup, easyTransform_getTransform(&e->T));
                   renderDrawSprite(globalRenderGroup, e->sprite, e->colorTint);
               }
            } 

            easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            ///////////////////////////////////////////////////////////////////////////////////

            {
            EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);

                renderSetShader(globalRenderGroup, &pixelArtProgramPlain);

               float fuaxWidth = 1920.0f;
               float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

               setViewTransform(globalRenderGroup, mat4());
               Matrix4 projection = OrthoMatrixToScreen_BottomLeft(fuaxWidth, fuaxHeight);
               setProjectionTransform(globalRenderGroup, projection);


                Texture *t = findTextureAsset("bulls_eye.png");

                gameState->tempTransform.Q = identityQuaternion();
                gameState->tempTransform.pos = v3(appInfo->keyStates.mouseP_01.x*fuaxWidth, (1.0f - appInfo->keyStates.mouseP_01.y)*fuaxHeight, 0.1f);

                gameState->tempTransform.scale.xy = v2(100, 100);

                setModelTransform(globalRenderGroup, easyTransform_getTransform(&gameState->tempTransform));
                renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);

                easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);
            }
            

            //DRAW THE PLAYER HUD
            if(gameState->gameModeType != GAME_MODE_CRAFTING) {

                EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);


                float fuaxWidth = 1920.0f;
                float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

                setViewTransform(globalRenderGroup, mat4());
                Matrix4 projection = OrthoMatrixToScreen_BottomLeft(fuaxWidth, fuaxHeight);
                setProjectionTransform(globalRenderGroup, projection);

                //DRAW THE CLOCK only outside
                if(gameState->useSunlightShadows) {

                   Texture *clock = findTextureAsset("sun_moon_clock1.png");
                   
                   float x = fuaxWidth - 200;
                   float y = 200;

                   float w = 400;
                   float h = clock->aspectRatio_h_over_w*w;

                   Rect2f r = rect2fMinDim(x - 0.5f*w, y, x + 0.5f*w, 0.5f*h);
                   easyRender_pushScissors(globalRenderGroup, r, 1, mat4(), projection, resolution);


                   Matrix4 Q = quaternionToMatrix(eulerAnglesToQuaternion(0, 0, -TAU32*weatherState->timeOfDay));
                   

                   Matrix4 T = Mat4Mult(Matrix4_translate(Matrix4_scale(mat4(), v3(w, h, 0)), v3(x, y, 0.4f)), Q);
                   setModelTransform(globalRenderGroup, T);
                   renderDrawSprite(globalRenderGroup, clock, COLOR_WHITE);
                   easyRender_disableScissors(globalRenderGroup);
                } 

                Entity *p = ((Entity *)(manager->player));

                float staminaPercent = p->stamina / p->maxStamina;

                float maxBarPixels = 300;
                float barHeight = 30;

                float x = 0.5*fuaxWidth - 0.6f*maxBarPixels;  
                float y = (0.1f*fuaxHeight);

                //Stamina points backing
                Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(maxBarPixels, barHeight, 0)), v3(x, y, 0.4f));
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, &globalWhiteTexture, COLOR_GREY);

                float barWidth = staminaPercent*maxBarPixels;
                float xOffset = 0.5f*(maxBarPixels - barWidth);

                //Stamina points percent bar
                T = Matrix4_translate(Matrix4_scale(mat4(), v3(barWidth, barHeight, 0)), v3(x - xOffset, y, 0.3f));

                renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, findTextureAsset("manabar.png"), COLOR_WHITE);

                // T = Matrix4_translate(Matrix4_scale(mat4(), v3(barWidth, barHeight, 0)), v3(x - xOffset, y, 0.2f));
                // setModelTransform(globalRenderGroup, T);
                // renderDrawSprite(globalRenderGroup, findTextureAsset("baroutline.png"), COLOR_WHITE);

                
                renderSetShader(globalRenderGroup, &textureProgram);

                //////////////////////////// The Coin ///////////////////////////////////////////
                { 
                    float xOffset = 100;
                    float yOffset = 100;
                    T = Matrix4_translate(Matrix4_scale(mat4(), v3(100, 100, 0)), v3(xOffset, fuaxHeight - yOffset, 0.3f));

                    renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, findTextureAsset("coin.png"), COLOR_WHITE);

                    //NOTE: Draw the amount of coins we have
                    xOffset += 70;
                    char coinCountString[256];
                    sprintf(coinCountString, "Coins:%d", gameState->playerSaveProgress.playerInfo.moneyCount);
                    outputTextNoBacking(gameFont, xOffset, yOffset, 0.3f, gameState->fuaxResolution, coinCountString, InfinityRect2f(), COLOR_WHITE, 1, true, 1);
                }   

                //////////////////////////// HEALTH BAR //////////////////////////////////////////


                float healthPercent = (float)p->health / (float)p->maxHealth;

                assert(healthPercent <= 1.0f);

                x = 0.5*fuaxWidth + 0.6f*maxBarPixels;

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
                renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
                setModelTransform(globalRenderGroup, T);
                renderDrawSprite(globalRenderGroup, findTextureAsset("healthbar.png"), COLOR_WHITE);

                // T = Matrix4_translate(Matrix4_scale(mat4(), v3(barWidth, barHeight, 0)), v3(x - xOffset, y, 0.2f));
                // setModelTransform(globalRenderGroup, T);
                // renderDrawSprite(globalRenderGroup, findTextureAsset("baroutline.png"), COLOR_WHITE);


                renderSetShader(globalRenderGroup, &textureProgram);
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
                // easyEditor_pushSlider(appInfo->editor, "value slider: ", &globalRenderGroup->shadowBias, 0, 0.005);
                {
                    bool numberKeyWasPressed = false;
                    int newValue = 0;
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_1)) {
                        numberKeyWasPressed = true;
                        newValue = 0;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_2)) {
                        numberKeyWasPressed = true;
                        newValue = 1;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_3)) {
                        numberKeyWasPressed = true;
                        newValue = 2;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_4)) {
                        numberKeyWasPressed = true;
                        newValue = 3;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_5)) {
                        numberKeyWasPressed = true;
                        newValue = 4;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_6)) {
                        numberKeyWasPressed = true;
                        newValue = 5;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_7)) {
                        numberKeyWasPressed = true;
                        newValue = 6;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_8)) {
                        numberKeyWasPressed = true;
                        newValue = 7;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_9)) {
                        numberKeyWasPressed = true;
                        newValue = 8;                    
                    }
                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_0)) {
                        numberKeyWasPressed = true;
                        newValue = 9;                    
                    }

                    if(numberKeyWasPressed && !easyEditor_isInteracting(appInfo->editor)) { easyEditor_alterListIndex(appInfo->editor, (int)newValue); } editorState->createMode = (EditorCreateMode)easyEditor_pushList(appInfo->editor, "Editor Mode: ", EditorCreateModesStrings, arrayCount(EditorCreateModesStrings));
                
                }


                easyEditor_pushCheckBox(appInfo->editor, "Control Player", &DEBUG_CONTROL_PLAYER);
                easyEditor_pushCheckBox(appInfo->editor, "Sunlight in Level", &gameState->useSunlightShadows);
                easyEditor_pushCheckBox(appInfo->editor, "Sun Color in Level", &gameState->useSunlightColor);


                int splatIndexOn = 0;
                // if(editorState->createMode == EDITOR_CREATE_HOUSE || editorState->createMode == EDITOR_CREATE_SCENERY || editorState->createMode == EDITOR_CREATE_SCENERY_RIGID_BODY || editorState->createMode == EDITOR_CREATE_ONE_WAY_PLATFORM || editorState->createMode == EDITOR_CREATE_SIGN || editorState->createMode == EDITOR_CREATE_LAMP_POST)
                {
                    splatIndexOn = easyEditor_pushSpriteList(appInfo->editor, "Sprites: ", (Texture **)gameState->splatTextures.memory, gameState->splatTextures.count);
                    // splatIndexOn = easyEditor_pushList(appInfo->editor, "Sprites: ", (char **)gameState->splatList.memory, gameState->splatList.count); 

                }

                


                int animationOn = 0;
                if(editorState->createMode == EDITOR_CREATE_SCENERY || editorState->createMode == EDITOR_CREATE_SCENERY_RIGID_BODY ||  editorState->createMode == EDITOR_CREATE_SIGN || editorState->createMode == EDITOR_CREATE_EMPTY_TRIGGER || editorState->createMode == EDITOR_CREATE_TRIGGER_WITH_RIGID_BODY || editorState->createMode == EDITOR_CREATE_LAMP_POST) {
                    animationOn = easyEditor_pushList(appInfo->editor, "Animations: ", (char **)gameState->splatListAnimations.memory, gameState->splatListAnimations.count); 
                }   

 

                EntityTriggerType triggerType;
                

                if(editorState->createMode == EDITOR_CREATE_EMPTY_TRIGGER) 
                {
                    
                    triggerType = (EntityTriggerType)easyEditor_pushList(appInfo->editor, "Trigger Type: ", MyEntity_TriggerTypeStrings, arrayCount(MyEntity_TriggerTypeStrings));



                }

                WorldTileRotation rotationType = WORLD_TILE_ROTATION_FLAT;
                if(editorState->createMode == EDITOR_CREATE_TILE_MODE) 
                {
                    rotationType = (WorldTileRotation)easyEditor_pushList(appInfo->editor, "Tile Rotation: ", MyTile_TileRotationTypeStrings, arrayCount(MyTile_TileRotationTypeStrings));
                }

                

                easyEditor_pushSlider(appInfo->editor, "Time of day: ", &weatherState->timeOfDay, 0, 1);
                easyEditor_pushSlider(appInfo->editor, "Time of Day Speed: ", &weatherState->timeOfDaySpeed, 0, 1);

                

                EasyModel *modelSelected = 0;

                // if(editorState->createMode == EDITOR_CREATE_3D_MODEL) {
                //     int modelIndex = easyEditor_pushList(appInfo->editor, "Models: ", modelsLoadedNames, allModelsForEditor.count); 

                //     modelSelected = allModelsForEditor.array[modelIndex].model;   

                // }   

                WorldTileType tileType;
                EditorTileOption tilerMode;


                if(editorState->createMode ==  EDITOR_CREATE_TILE_MODE) {
                    // editorState->tileType = (WorldTileType)easyEditor_pushList(appInfo->editor, "Tile Type: ", MyTiles_TileTypeStrings, arrayCount(MyTiles_TileTypeStrings));
                    editorState->tileOption = (EditorTileOption)easyEditor_pushList(appInfo->editor, "Tile Mode: ", MyTiles_editorOptionStrings, arrayCount(MyTiles_editorOptionStrings));
                    
                    int tileIndex = easyEditor_pushSpriteList(appInfo->editor, "Tiles: ", (Texture **)gameState->splatTextures_tiles.memory, gameState->splatTextures_tiles.count);
                    

                    editorState->tileType = getTileTypeBasedOnSprite(gameState, tileIndex);
                }
                

                bool pressed = wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE);

                Texture *splatTexture = ((Texture **)(gameState->splatTextures.memory))[splatIndexOn];

                // Texture *splatTexture_tile = ((Texture **)(gameState->splatTextures_tiles.memory))[splatIndexOn_tiles];
                bool justCreatedEntity = false;
                if(!appInfo->editor->isHovering) {

                    EasyPlane floor = {};

                    floor.origin = v3(0, 0, 0);
                    floor.normal = v3(0, 0, -1);

                    V3 hitP;
                    float tAt;

                    if(easyMath_castRayAgainstPlane(mouseP_worldRay, floor, &hitP, &tAt)) {
                        //NOTE: Lock entities to integer coordinates - grid base gameplay
                        if(DEBUG_LOCK_POSITION_TO_GRID) {
                            
                            float tileScale = 1;
                            if(editorState->createMode == EDITOR_CREATE_TILE_MODE) {
                               tileScale = GLOBAL_WORLD_TILE_SIZE;
                            }


                            hitP = roundToGridBoard(hitP, tileScale);
                        }
                    }

                    switch(editorState->createMode) {
                        case EDITOR_CREATE_SELECT_MODE: {
                            //do nothing
                        } break;
                        case EDITOR_CREATE_EMPTY_TRIGGER: {
                            if(pressed) {

                                editorState->entitySelected = initEmptyTrigger(gameState, manager, hitP, triggerType, splatTexture, 0);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                                justCreatedEntity = true;

                                if(animationOn > 0) {
                                    Animation *animation = ((Animation **)(gameState->splatAnimations.memory))[animationOn - 1];

                                    Entity *e = (Entity *)(editorState->entitySelected);
                                    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, animation, e->animationRate);  

                                    e->sprite = 0;
                                }


                            }
                        } break;
                        case EDITOR_CREATE_ENTITY_BOARD_MODE: {
                            if(editorState->entitySelected) {
                                Entity *e = (Entity *)editorState->entitySelected;
                                if(!e->aiController) {
                                    e->aiController = easyAi_initController(&globalPerSceneArena);    
                                }
                                    
                                if(isDown(gameKeyStates.gameButtons, BUTTON_CTRL)) {
                                    easyConsole_addToStream(DEBUG_globalEasyConsole, "ctrl down");
                                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                                        int bouyIndexAt = easyAi_hasSearchBouy(e->aiController, hitP);
                                        if(bouyIndexAt >= 0) {
                                            easyAi_removeSearchBouy(e->aiController, bouyIndexAt);
                                             easyFlashText_addText(&globalFlashTextManager, "Removed Search Bouy");
                                         } else {
                                            easyAi_pushSearchBouy(e->aiController, hitP);  
                                            easyFlashText_addText(&globalFlashTextManager, "Added Search Bouy");
                                         }
                                    }
                                } else {
                                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                                        bool shiftDown = isDown(gameKeyStates.gameButtons, BUTTON_SHIFT);
                                        bool addedNewNode = easyAi_pushNode(e->aiController, hitP, e->aiController->boardHash, !shiftDown);
                                        if(addedNewNode) {
                                            easyFlashText_addText(&globalFlashTextManager, "Added Ai Tile");
                                        } else {
                                            EasyAi_Node *found = easyAi_removeNode(e->aiController, hitP, e->aiController->boardHash);
                                            assert(found);
                                            easyFlashText_addText(&globalFlashTextManager, "Removed Ai Tile");
                                        }
                                    }
                                }
                            }
                        } break;
                        case EDITOR_CREATE_TILE_MODE: {
                            if(editorState->tileOption == EDITOR_TILE_SINGLE) {
                                if(isDown(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {

                                    V3 createP;
                                    float planeOrigin = {};
                                    {
                                        EasyPlane plane = {};

                                        plane.origin = v3(0, 0, 0);
                                        plane.normal = v3(0, 0, -1);

                                        if(rotationType == WORLD_TILE_ROTATION_UP) {
                                            //start at the bottom of the tile in the y coordinate
                                            plane.origin = v3_minus(editorState->lastcreatedTileP_onGround, v3(0, 0.5f, 0));
                                            
                                            plane.origin.x = 0;
                                            plane.origin.z = 0;

                                            planeOrigin = plane.origin.y;

                                            plane.normal = v3(0, -SIN45, -SIN45);       
                                        }

                                        float tAt;

                                        if(easyMath_castRayAgainstPlane(mouseP_worldRay, plane, &createP, &tAt)) {
                                            if(rotationType == WORLD_TILE_ROTATION_FLAT) {
                                                createP = roundToGridBoard(createP, GLOBAL_WORLD_TILE_SIZE);
                                                editorState->lastcreatedTileP_onGround = createP;
                                                // easyConsole_pushV3(DEBUG_globalEasyConsole, editorState->lastcreatedTileP_onGround);
                                            } else {

                                                createP = v3_minus(createP, plane.origin);

                                                V3 xAxis = v3(1, 0, 0);
                                                V3 yAxis = normalizeV3(v3(0, SIN45, -SIN45));

                                                // V3 otherXAxis = v3_crossProduct(yAxis, plane.normal);
                                                // we get the x & y positions local to the plane
                                                V3 localP = {};
                                                localP.x = dotV3(createP, xAxis);
                                                localP.y = dotV3(createP, yAxis);
                                                localP.z = 0;

                                                localP = roundToGridBoard(localP, GLOBAL_WORLD_TILE_SIZE);

                                                createP = v3_plus(plane.origin, v3_plus(v3_scale(localP.x, xAxis), v3_scale(localP.y, yAxis)));
                                            }
                                        }
                                    }

                                    if(!addWorldTile(gameState, createP.x, createP.y, createP.z, planeOrigin, editorState->tileType, rotationType)) {
                                        easyFlashText_addText(&globalFlashTextManager, "Tile Array Full. Make bigger!");
                                    }
                                }
                            } else if(editorState->tileOption == EDITOR_TILE_DRAG) {
                                if(wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                                    editorState->topLeftCornerOfTile = hitP.xy;
                                }

                                if(isDown(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                                    //draw the outline
                                    V2 diff = v2_minus(hitP.xy, editorState->topLeftCornerOfTile);


                                    outlineTransform.pos.xy = v2_plus(editorState->topLeftCornerOfTile, v2_scale(0.5f, diff));
                                    outlineTransform.scale.xy = diff; 

                                    Matrix4 outlineT = easyTransform_getTransform(&outlineTransform);
                                    setModelTransform(globalRenderGroup, outlineT);
                                    renderDrawSprite(globalRenderGroup, outlineSprite, COLOR_WHITE);

                                }



                                bool ranOutOfTileRoom = false;
                                if(wasReleased(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE)) {
                                    V2 diff = v2_minus(hitP.xy, editorState->topLeftCornerOfTile);

                                    int w = (int)absVal(diff.x);
                                    int h = (int)absVal(diff.y);
                                    for(int i = 0; i < h && !ranOutOfTileRoom; i++) {
                                        for(int j = 0; j < w && !ranOutOfTileRoom; j++) {
                                            ranOutOfTileRoom = !addWorldTile(gameState, j + editorState->topLeftCornerOfTile.x, editorState->topLeftCornerOfTile.y - i, 0, 0, editorState->tileType, WORLD_TILE_ROTATION_FLAT);
                                        }    
                                    }

                                    char str[256];
                                    sprintf(str, "%d", w*h);
                                    easyFlashText_addText(&globalFlashTextManager, str);
                                   
                                }

                                if(ranOutOfTileRoom) {
                                     easyFlashText_addText(&globalFlashTextManager, "Tile Array Full. Make bigger!");
                                }   
                            }
                        } break;
                        case EDITOR_CREATE_BLOCK_TO_PUSH: {
                            if(pressed) {
                                editorState->entitySelected = initPushRock(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                                justCreatedEntity = true;
                            }
                        } break;
                        case EDITOR_CREATE_LAMP_POST: {
                            if(pressed) {
                                Animation *animation = 0;
                                if(animationOn > 0) {
                                    animation = ((Animation **)(gameState->splatAnimations.memory))[animationOn - 1];
                                }

                                editorState->entitySelected = initLampPost(gameState, manager, hitP, splatTexture, animation);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                                justCreatedEntity = true;
                            }
                        } break;
                        // case EDITOR_CREATE_3D_MODEL: {
                        //     if(pressed) {
                        //         editorState->entitySelected = init3dModel(gameState, manager, hitP, modelSelected);
                        //         editorState->entityIndex = manager->lastEntityIndex;
                        //         assert(editorState->entitySelected);
                        //         justCreatedEntity = true;
                        //     }
                        // } break;
                        // case EDITOR_CREATE_TERRAIN: {
                        //     if(pressed) {
                        //         editorState->entitySelected = initTerrain(gameState, manager, hitP);
                        //         gameState->currentTerrainEntity = editorState->entitySelected; 
                        //         editorState->entityIndex = manager->lastEntityIndex;
                        //         assert(editorState->entitySelected);
                        //         justCreatedEntity = true;
                        //     }
                        // } break;
                        // case EDITOR_CREATE_ENTITY_AI_STATE: {
                        //     if(pressed) {
                        //         editorState->entitySelected = initAiAnimation(gameState, manager, hitP);
                        //         editorState->entityIndex = manager->lastEntityIndex;
                        //         assert(editorState->entitySelected);
                        //         justCreatedEntity = true;
                        //     }
                        // } break;
                        case EDITOR_CREATE_SHOOT_TRIGGER: {
                            if(pressed) {
                                editorState->entitySelected = initShootTrigger(gameState, manager, hitP, splatTexture);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                                justCreatedEntity = true;
                            }
                        } break;
                        case EDITOR_CREATE_ENTITY_FOG: {
                            if(pressed) {
                                editorState->entitySelected = initFog(gameState, manager, hitP, splatTexture);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);
                                justCreatedEntity = true;
                            }
                        } break;
                        case EDITOR_CREATE_SCENERY: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, hitP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, false);
                                editorState->entityIndex = manager->lastEntityIndex;

                                if(animationOn > 0) {
                                    Animation *animation = ((Animation **)(gameState->splatAnimations.memory))[animationOn - 1];

                                    Entity *e = (Entity *)(editorState->entitySelected);
                                    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, animation, e->animationRate);  

                                    e->sprite = 0;
                                }
                                justCreatedEntity = true;
                            }

                        } break;
                        case EDITOR_CREATE_TRIGGER_WITH_RIGID_BODY: {
                            if(pressed) {
                                editorState->entitySelected = initEmptyTriggerWithRigidBody(gameState, manager, hitP, splatTexture);
                                editorState->entityIndex = manager->lastEntityIndex;
                                assert(editorState->entitySelected);

                                if(animationOn > 0) {
                                    Animation *animation = ((Animation **)(gameState->splatAnimations.memory))[animationOn - 1];

                                    Entity *e = (Entity *)(editorState->entitySelected);
                                    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, animation, e->animationRate);  

                                    e->sprite = 0;
                                }

                                justCreatedEntity = true;
                            }
                        } break;
                        case EDITOR_CREATE_SCENERY_RIGID_BODY: {
                            if(pressed) {
                                editorState->entitySelected = initEntity(manager, 0, hitP, v2(1, 1), v2(1, 1), gameState, ENTITY_SCENERY, 0, splatTexture, COLOR_WHITE, -1, true);
                                editorState->entityIndex = manager->lastEntityIndex;
                                justCreatedEntity = true;

                            }
                        } break;   
                        case EDITOR_CREATE_ENEMY: {
                            if(pressed) {
                                editorState->entitySelected = initEnemy(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;
                                justCreatedEntity = true;

                            }
                        } break;
                        case EDITOR_CREATE_CHEST: {
                            if(pressed) {
                                editorState->entitySelected = initChest(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;
                                justCreatedEntity = true;

                            }
                        } break;
                        case EDITOR_CREATE_SIGN: {
                            if(pressed) {
                                editorState->entitySelected = initSign(gameState, manager, hitP, splatTexture);
                                editorState->entityIndex = manager->lastEntityIndex;

                                if(animationOn > 0) {
                                    Animation *animation = ((Animation **)(gameState->splatAnimations.memory))[animationOn - 1];

                                    Entity *e = (Entity *)(editorState->entitySelected);
                                    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationFreeList, animation, e->animationRate);  

                                    e->sprite = 0;
                                }

                                justCreatedEntity = true;

                            }
                        } break;
                        case EDITOR_CREATE_ENTITY_CREATOR: {
                            if(pressed) {
                                editorState->entitySelected = initEntityCreator(gameState, manager, hitP);
                                editorState->entityIndex = manager->lastEntityIndex;
                                justCreatedEntity = true;

                            }
                        } break;
                    }

                    lerpToSelectedEntity(editorState, camera.pos.xy);
                    
                }
                
                easyEditor_pushCheckBox(appInfo->editor, "Bounds", &DEBUG_DRAW_COLLISION_BOUNDS);
                easyEditor_pushCheckBox(appInfo->editor, "Bounds Triggers", &DEBUG_DRAW_COLLISION_BOUNDS_TRIGGERS);
                easyEditor_pushCheckBox(appInfo->editor, "Terrain", &DEBUG_DRAW_TERRAIN);
                easyEditor_pushCheckBox(appInfo->editor, "Ortho Camera", &DEBUG_USE_ORTHO_MATRIX);
                easyEditor_pushCheckBox(appInfo->editor, "Ai Grid", &DEBUG_AI_BOARD_FOR_ENTITY);
                easyEditor_pushCheckBox(appInfo->editor, "Lock position", &DEBUG_LOCK_POSITION_TO_GRID);
                easyEditor_pushCheckBox(appInfo->editor, "Angle Entity", &DEBUG_ANGLE_ENTITY_ON_CREATION);
                easyEditor_pushCheckBox(appInfo->editor, "Scenery", &DEBUG_DRAW_SCENERY_TEXTURES);

                if(easyEditor_pushButton(appInfo->editor, "Zero Pos Player")) {
                    ((Entity *)(manager->player))->T.pos.xy = v2(0, 0);
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
                    easyEditor_pushSlider(appInfo->editor, "Z Position: ", &e->T.pos.z, -3.0f, 3.0f);
                    easyEditor_pushFloat3(appInfo->editor, "Scale: ", &e->T.scale.x, &e->T.scale.y, &e->T.scale.z);

                    if(e->type == ENTITY_ENTITY_CREATOR) {
                        easyEditor_alterListIndex_withIds(appInfo->editor, (int)e->typeToCreate, e->T.id, gameState->currentSceneName); e->typeToCreate = (EntityType)easyEditor_pushList_withIds(appInfo->editor, "Create Entity Type: ", MyEntity_EntityTypeStrings, arrayCount(MyEntity_EntityTypeStrings), e->T.id, gameState->currentSceneName);
                        easyEditor_pushFloat1(appInfo->editor, "RateOfCreation: ", &e->rateOfCreation);
                    }

                    if(&e->animationController.parent != e->animationController.parent.next) {
                        easyEditor_pushSlider(appInfo->editor, "Animation Rate: ", &e->animationRate, 0.001f, 3.0f);
                        // easyEditor_pushFloat1(appInfo->editor, "Animation Rate: ", &e->animationRate);

                        EasyAnimation_ListItem *nextAnimation = e->animationController.parent.next;
                        while(&e->animationController.parent != nextAnimation) {

                            nextAnimation->timerPeriod = e->animationRate;
                            nextAnimation = nextAnimation->next;
                        }
                    }

                    if(e->type == ENTITY_ENEMY || e->type == ENTITY_TRIGGER) {
                        EntityEnemyType lastType = e->enemyType;
                        easyEditor_alterListIndex_withIds(appInfo->editor, (int)e->enemyType, e->T.id, gameState->currentSceneName); e->enemyType = (EntityEnemyType)easyEditor_pushList_withIds(appInfo->editor, "Enemy Type: ", MyEntity_EnemyTypeStrings, arrayCount(MyEntity_EnemyTypeStrings), e->T.id, gameState->currentSceneName);
                        easyEditor_pushSlider(appInfo->editor, "Move Speed: ", &e->enemyMoveSpeed, 1, 10);

                        if(lastType != e->enemyType) {
                            assignAnimalAttribs(e, gameState, e->enemyType);

                            if(e->enemyType == ENEMY_BUG) {
                                e->T.scale.xy = getEnemyScale(e->enemyType);
                                if(!getEnemyRotate(e->enemyType)) {
                                    e->T.Q = identityQuaternion();
                                }
                            }
                        }
                    }

                    if(e->type == ENTITY_CHEST || e->type == ENTITY_TRIGGER_WITH_RIGID_BODY) {
                        easyEditor_alterListIndex_withIds(appInfo->editor, (int)e->chestType, e->T.id, gameState->currentSceneName); e->chestType = (ChestType)easyEditor_pushList_withIds(appInfo->editor, "Chest Type: ", MyEntity_ChestTypeStrings, arrayCount(MyEntity_ChestTypeStrings), e->T.id, gameState->currentSceneName);
                    }

                    if(e->type == ENTITY_ENEMY) {
                        bool value = (bool)(e->subEntityType & ENEMY_IGNORE_PLAYER);
                        easyEditor_pushCheckBox(appInfo->editor, "Ingore Player", &value);

                        if(!value) {
                           e->subEntityType &= ~ENEMY_IGNORE_PLAYER;
                        } else {
                            e->subEntityType |= ENEMY_IGNORE_PLAYER;
                        }
                        
                    }



                    bool value = (bool)(e->flags & ENTITY_SHOULD_NOT_RENDER);
                    easyEditor_pushCheckBox(appInfo->editor, "Should Not Render", &value);

                    if(!value) {
                       e->flags &= ~ENTITY_SHOULD_NOT_RENDER;
                    } else {
                        e->flags |= ENTITY_SHOULD_NOT_RENDER;
                    }
                        

                    if(e->currentSound) {
                        easyEditor_pushSlider(appInfo->editor, "volume: ", &e->currentSound->volume, 0, 3);
                        easyEditor_pushSlider(appInfo->editor, "innerRadius: ", &e->currentSound->innerRadius , 0.1, 2);
                        easyEditor_pushSlider(appInfo->editor, "outerRadius: ", &e->currentSound->outerRadius, 0.5, 20);
                    }


                        
                    
                    if(e->type == ENTITY_LAMP_POST || e->triggerType == ENTITY_TRIGGER_SAVE_BY_FIRE || e->triggerType == ENTITY_TRIGGER_FIRE_POST) {
                        V4 lColor = {};
                        lColor.xyz = e->lightColor;
                        lColor.w = 1;
                        easyEditor_pushFloat1(appInfo->editor, "LightPower: ", &e->lightIntensity);
                        easyEditor_pushColor_(appInfo->editor, "Light Color: ", &lColor, e->T.id, gameState->currentSceneName);

                        e->lightColor = lColor.xyz;
                    }

                    if(e->sprite && easyEditor_pushButton(appInfo->editor, "Snap Aspect Ratio")) {
                        e->T.scale.y = e->sprite->aspectRatio_h_over_w*e->T.scale.x;
                    }


                    if(e->type == ENTITY_TRIGGER || e->type == ENTITY_TRIGGER_WITH_RIGID_BODY) {

                        EntityTriggerType entityTriggerType = e->triggerType; 
                        easyEditor_alterListIndex_withIds(appInfo->editor, (int)e->triggerType, e->T.id, gameState->currentSceneName); e->triggerType = (EntityTriggerType)easyEditor_pushList_withIds(appInfo->editor, "Trigger Type: ", MyEntity_TriggerTypeStrings, arrayCount(MyEntity_TriggerTypeStrings), e->T.id, gameState->currentSceneName);
                        
                        if(entityTriggerType != e->triggerType && e->triggerType == ENTITY_TRIGGER_OPEN_DOOR_WITH_BUTTON_WITH_TRIGGER_CLOSE) {
                            e->isActivated = false; 
                            
                            // Move the door down ready to be opened
                            e->T.pos.z = 1.f;
                            e->chestIsOpen = true;
                            e->collider->isActive = false;
                        }
                        

                        easyEditor_alterListIndex_withIds(appInfo->editor, (int)e->locationSoundType, e->T.id, gameState->currentSceneName); e->locationSoundType = (EntityLocationSoundType)easyEditor_pushList_withIds(appInfo->editor, "Sound Type: ", MyEntity_LocationSoundTypeStrings, arrayCount(MyEntity_LocationSoundTypeStrings), e->T.id, gameState->currentSceneName);
                    
                        if(e->levelToLoad) {
                            e->levelToLoad = easyEditor_pushTextBox_withIds(appInfo->editor, "Scene to load:", e->levelToLoad, e->T.id, gameState->currentSceneName);
                        }

                        easyEditor_pushInt1(appInfo->editor, "PartnerId", &e->partnerId);
                        easyEditor_pushFloat2(appInfo->editor, "Move Direction for Partner: ", &e->moveDirection.x, &e->moveDirection.y);
                    }

                   if(e->type == ENTITY_HOUSE) {
                       e->levelToLoad = easyEditor_pushTextBox_withIds(appInfo->editor, "Scene to load:", e->levelToLoad, e->T.id, gameState->currentSceneName);
                   }


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

                    easyEditor_pushColor_(appInfo->editor, "Color: ", &e->colorTint, e->T.id, gameState->currentSceneName);
                    easyEditor_pushInt1(appInfo->editor, "MaxHealth: ", &e->maxHealth);
                    easyEditor_pushFloat1(appInfo->editor, "MaxStamina: ", &e->maxStamina);

                    // if(easyEditor_pushButton(appInfo->editor, e->renderFirstPass ? "first pass Off" : "first pass Off")) {
                    //     e->renderFirstPass = !e->renderFirstPass;
                    //     // manager->player->collider1->offset.y = 0;
                    // }


                    if(e->collider) {
                        easyEditor_pushFloat2(appInfo->editor, "Collider: ", &e->collider->dim2f.x, &e->collider->dim2f.y); 
                        easyEditor_pushFloat2(appInfo->editor, "Col offset: ", &e->collider->offset.x, &e->collider->offset.y);
                        // easyConsole_pushFloat(DEBUG_globalEasyConsole, e->collider->offset.x);
                    }

                    if(e->collider1) {
                        easyEditor_pushFloat2(appInfo->editor, "Collider 1: ", &e->collider1->dim2f.x, &e->collider1->dim2f.y);
                        easyEditor_pushFloat2(appInfo->editor, "Collider1 offset: ", &e->collider1->offset.x, &e->collider1->offset.y); 
                    }

                    // if(e->type == ENTITY_CHEST) {
                    //     easyEditor_alterListIndex(appInfo->editor, (int)e->chestContains); EntityType chestContains = (EntityType)easyEditor_pushList(appInfo->editor, "Chest Contains: ", MyEntity_EntityTypeStrings, arrayCount(MyEntity_EntityTypeStrings));
                    //     easyEditor_pushInt1(appInfo->editor, "Chest ItemCount: ", &e->itemCount);;
                    // }

                    if(e->type == ENTITY_SIGN) {

                        //These have to be on the same line since the editor uses __LINE__ as the uuid.
                        easyEditor_alterListIndex_withIds(appInfo->editor, (int)e->dialogType, e->T.id, gameState->currentSceneName); DialogInfoType dialogType = (DialogInfoType)easyEditor_pushList_withIds(appInfo->editor, "Dialog: ", MyDialog_DialogTypeStrings, arrayCount(MyDialog_DialogTypeStrings), e->T.id, gameState->currentSceneName); 

                        e->dialogType = dialogType;
                    }
                    

                    // easyEditor_pushInt1(appInfo->editor, "Health: ", &e->health);
                    // easyEditor_pushInt1(appInfo->editor, "Max Health: ", &e->maxHealth);

                    //wasPressed(appInfo->keyStates.gameButtons, BUTTON_DELETE) || wasPressed(appInfo->keyStates.gameButtons, BUTTON_BACKSPACE)
                    if(e->type != ENTITY_WIZARD && (easyEditor_pushButton(appInfo->editor, "Delete Entity") || (wasPressed(gameKeyStates.gameButtons, BUTTON_BACKSPACE) && !easyEditor_isInteracting(appInfo->editor)))) {

                        //NOTE: Add entity to be deleted off disk when we save the scene, so we add to array ready to commit the delete
                        {
                            ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteOnSave);
                            EntityToDeleteOnSave *entityToDelete = (EntityToDeleteOnSave *)arrayInfo.elm;
                            entityToDelete->id = e->T.id;
                            entityToDelete->sceneName = gameState->currentSceneName;
                        }
                        ///////////////////////////////////


                        ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&manager->entitiesToDeleteForFrame);
                        int *indexToAdd = (int *)arrayInfo.elm;
                        *indexToAdd = editorState->entityIndex;
                        if(editorState->entitySelected == gameState->currentTerrainEntity) {
                            gameState->currentTerrainEntity = 0;
                        }
                        letGoOfSelectedEntity(editorState);
                        
                    }

                    if(easyEditor_pushButton(appInfo->editor, "Let Go")) {
                        letGoOfSelectedEntity(editorState);
                    }

                    if(easyEditor_pushButton(appInfo->editor, "Flip")) {
                        e->isFlipped = !e->isFlipped;

                        if(e->isFlipped) {
                            if(e->collider) {
                                e->collider->offset.x *= -1;
                            }
                            if(e->collider1) {
                                e->collider1->offset.x *= -1;
                            }
                        }
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
                            case ENTITY_HEALTH_POTION_1: {
                                // assert(false);
                                easyFlashText_addText(&globalFlashTextManager, "Can't copy");
                            } break;
                            case ENITY_AUDIO_CHECKPOINT: {
                                newEntity = initAudioCheckPoint(gameState, manager, position);
                            } break;
                            case ENTITY_SIGN: {
                                newEntity = initSign(gameState, manager, position, e->sprite);
                            } break;
                            case ENTITY_SHOOT_TRIGGER: {
                                newEntity = initShootTrigger(gameState, manager, position, e->sprite);
                            } break;
                            case ENTITY_CHEST: {
                                newEntity = initChest(gameState, manager, position);
                                newEntity->chestType = e->chestType;
                            } break;
                            case ENITY_CHECKPOINT: {
                                newEntity = initCheckPoint(gameState, manager, position);
                            } break;
                            case ENTITY_LAMP_POST: {
                                Animation *animation = 0;
                                if(e->animationController.parent.next != &e->animationController.parent) {
                                    animation = e->animationController.parent.next->animation;    
                                }
                                
                                newEntity = initLampPost(gameState, manager, position, e->sprite, animation);
                            } break;
                            default: {
                                easyFlashText_addText(&globalFlashTextManager, "Can't duplicate yet, please add in editor");
                            }
                        }

                        if(newEntity) {
                            if(newEntity->collider) {
                                newEntity->collider->dim2f = e->collider->dim2f; 
                                newEntity->collider->offset = e->collider->offset;
                            }


                            if(newEntity->collider1) {
                                newEntity->collider1->dim2f = e->collider1->dim2f; 
                                newEntity->collider1->offset = e->collider1->offset;  
                            }

                            newEntity->maxHealth = e->maxHealth;
                            newEntity->T.Q = e->T.Q;
                            newEntity->T.scale = e->T.scale;
                            newEntity->colorTint = e->colorTint;

                            newEntity->animationRate = e->animationRate;

                            if(&e->animationController.parent != e->animationController.parent.next) {
                                EasyAnimation_ListItem *nextAnimation = e->animationController.parent.next;
                                while(&e->animationController.parent != nextAnimation) {

                                    easyAnimation_addAnimationToController(&newEntity->animationController, &gameState->animationFreeList, nextAnimation->animation, nextAnimation->timerPeriod);
                                    nextAnimation = nextAnimation->next;
                                }
                            }



                            newEntity->audioFile = e->audioFile;
                            newEntity->dialogType = e->dialogType;
                        }

                    }

                    easyEditor_endWindow(appInfo->editor); //might not actuall need this

                    if(wasPressed(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && !appInfo->editor->isHovering && !insideEntity && editorState->gizmoSelect == EDITOR_GIZMO_NONE && !justCreatedEntity && editorState->createMode != EDITOR_CREATE_ENTITY_BOARD_MODE) {
                        
                        letGoOfSelectedEntity(editorState);
                        
                    }

                    //Update the entity moving
                    if(editorState->entitySelected && isDown(gameKeyStates.gameButtons, BUTTON_LEFT_MOUSE) && !appInfo->editor->isHovering && editorState->gizmoSelect == EDITOR_GIZMO_NONE && editorState->createMode != EDITOR_CREATE_ENTITY_BOARD_MODE) {
                        Entity *e = ((Entity *)editorState->entitySelected);
                            
                        EasyPlane floor = {};

                        floor.origin = v3(0, 0, 0);
                        floor.normal = v3(0, 0, -1);

                        V3 hitP;
                        float tAt;

                        if(easyMath_castRayAgainstPlane(mouseP_worldRay, floor, &hitP, &tAt)) {
                            
                            hitP.z = 0;

                            e->T.pos.xy = hitP.xy;//v3_plus(hitP, editorState->grabOffset).xy;

                            //NOTE: Lock entities to integer coordinates - grid base gameplay
                            if(DEBUG_LOCK_POSITION_TO_GRID) {
                                e->T.pos.xy = roundToGridBoard(e->T.pos, 1).xy;
                            }
                        }
                    }
                }
            }


           //  ////////////////////////////////////////////////////////////////////

            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            /////////////////////////////////////////////////////
             //NOTE: Draw what the player is holding
            if(gameState->gameModeType != GAME_MODE_CRAFTING){

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

                outputTextNoBacking(gameFont, itemPosition1.x - 40, gameState->fuaxResolution.y - itemPosition1.y + 40, 0.1f, gameState->fuaxResolution, "Z", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_WHITE, 1, true, 1);
                
                if(gameState->playerHolding[0] && gameState->playerHolding[0]->type != ENTITY_NULL) {

                    float x = itemPosition1.x + xOffset;
                    float y = itemPosition1.y + yOffset;
                    setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(item_T, v3(itemScale*canVal0, itemScale*canVal0, 0)), v3(x, y, 0.1f)));

                    Texture *t = getInvetoryTexture(gameState->playerHolding[0]->type);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);

                    if(gameState->playerHolding[0]->isDisposable) {
                        float offset = 50;
                        char *str = easy_createString_printf(&globalPerFrameArena, "%d", gameState->playerHolding[0]->count);
                        outputTextNoBacking(gameFont, x + offset, gameState->fuaxResolution.y - y + offset, 0.1f, gameState->fuaxResolution, str, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_WHITE, 1.3f, true, 1);
                    }
                }

                setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(T_1, v3(canVal1, canVal1, 0)), itemPosition2));
                renderDrawSprite(globalRenderGroup, t_square, COLOR_WHITE);

                outputTextNoBacking(gameFont, itemPosition2.x - 40, gameState->fuaxResolution.y - itemPosition2.y + 40, 0.1f, gameState->fuaxResolution, "X", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_WHITE, 1, true, 1);

                if(gameState->playerHolding[1] && gameState->playerHolding[1]->type != ENTITY_NULL) {

                    float x = itemPosition2.x + xOffset;
                    float y = itemPosition2.y + yOffset;

                    setModelTransform(globalRenderGroup, Matrix4_translate(Matrix4_scale(item_T, v3(itemScale*canVal1, itemScale*canVal1, 0)), v3(x, y, 0.1f)));

                    Texture *t = getInvetoryTexture(gameState->playerHolding[1]->type);
                    renderDrawSprite(globalRenderGroup, t, COLOR_WHITE);

                    if(gameState->playerHolding[1]->isDisposable) {
                        float offset = 50;
                        char *str = easy_createString_printf(&globalPerFrameArena, "%d", gameState->playerHolding[1]->count);
                        outputTextNoBacking(gameFont, x + offset, gameState->fuaxResolution.y - y + offset, 0.1f, gameState->fuaxResolution, str, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_WHITE, 1.3f, true, 1);
                    }

                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));
            }
            ///////

            //NOTE: Draw the shop if in the shop
            if(gameState->gameModeType == GAME_MODE_SHOP) {
                //Make sure game is paused
                gameState->gameIsPaused = true;

                updateShop(gameState->current_shop, gameState, (Entity *)manager->player, gameFont, appInfo);

                //NOTE: Exit the shop
                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_ESCAPE)) {
                    gameState->gameModeType = GAME_MODE_PLAY;
                    gameState->gameIsPaused = false;
                    gameState->current_shop = 0;
                }
            }

            //NOTE: Draw the shop if in the shop
            if(gameState->gameModeType == GAME_MODE_CRAFTING) {
                //Make sure game is paused
                gameState->gameIsPaused = true;

                updateCrafting(gameState->crafting, gameState, (Entity *)manager->player, gameFont, appInfo);

                //NOTE: Exit the shop
                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_ESCAPE)) {
                    gameState->gameModeType = GAME_MODE_PLAY;
                    gameState->gameIsPaused = false;
                    gameState->current_shop = 0;
                }
            }
            
            //NOTE(ollie): Make sure the transition is on top
            renderClearDepthBuffer(mainFrameBuffer.bufferId);

            FrameBuffer *endBuffer = &mainFrameBuffer;

           //  ////////////////// Draw the Pause menu //////////////////////
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

                //Draw backing 
                {
                    Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth, fuaxHeight, 0)), v3(0, 0, 0.6f));
                    setModelTransform(globalRenderGroup, T);
                    renderDrawQuad(globalRenderGroup, v4(0, 0, 0, 0.7f));
    
                }
                

                if(gameState->pauseMenu_subType == GAME_PAUSE_MENU_MAIN) {

                    char *pauseMenuItems[] = { "Continue", "Save", "Settings", "Exit" };

                    float yT = fuaxHeight / (float)(arrayCount(pauseMenuItems) + 1);

                    float fonty = yT;


                    if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_DOWN)) {
                        gameState->currentMenuIndex++;

                       if(gameState->currentMenuIndex >= arrayCount(pauseMenuItems)) {
                           gameState->currentMenuIndex = arrayCount(pauseMenuItems) - 1;
                       } else {
                        playMenuSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_FOREGROUND);
                       }
                    }

                    if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_UP)) {
                        gameState->currentMenuIndex--;

                        if(gameState->currentMenuIndex < 0) {
                            gameState->currentMenuIndex = 0;
                        } else {
                            playMenuSound(&globalLongTermArena, gameState->clickSound, 0, AUDIO_FOREGROUND);
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
                                    gameState->pauseMenu_subType = GAME_PAUSE_MENU_MAIN;

                                }

                                //NOTE: Save game
                                if(gameState->currentMenuIndex == 1) {
                                    playMenuSound(&globalLongTermArena, gameState->saveSuccessSound, 0, AUDIO_FOREGROUND);
                                    playerGameState_saveProgress(gameState, manager, appInfo->saveFolderLocation);
                                }

                                if(gameState->currentMenuIndex == 2) { //settings
                                     gameState->pauseMenu_subType = GAME_PAUSE_MENU_SETTINGS;
                                }

                                if(gameState->currentMenuIndex == 3) { //exit
                                    appInfo->running = false;
                                }
                            }
                        }

                        //Draw the text
                        V2 size = getBounds(title, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 2, gameState->fuaxResolution, 1);
                         
                        float fontx = -0.5f*size.x + 0.5f*fuaxWidth; 
                        

                        

                        outputTextNoBacking(gameFont, fontx, fonty, 0.1f, gameState->fuaxResolution, title, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), color, 2, true, 1);
                        /////////////////////////

                        fonty += yT;
                    }
                } else if(gameState->pauseMenu_subType == GAME_PAUSE_MENU_SETTINGS) {
                    Texture *controllerSheetT = findTextureAsset("controller_sheet.jpg");

                    ///Color background yellow
                    Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth,fuaxHeight, 0)), v3(0, 0, 0.5f));
                    setModelTransform(globalRenderGroup, T);
                    renderDrawQuad(globalRenderGroup, settingsYellowColor);    
                    ///////////////////////

                    float h = 0.9f*fuaxHeight;
                    //Draw prompt button to continue
                    float aspectRatio = 1.0f / controllerSheetT->aspectRatio_h_over_w;
                    T = Matrix4_translate(Matrix4_scale(mat4(), v3(h*aspectRatio, h, 0)), v3(0, 0, 0.4f));
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, controllerSheetT, COLOR_WHITE);
                    ///////

                    if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_ESCAPE)) {
                        gameState->pauseMenu_subType = GAME_PAUSE_MENU_MAIN;
                    }   
                }

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

                renderClearDepthBuffer(mainFrameBuffer.bufferId);

                easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);  
            }

           //  /////////////// Draw the text ///////////////////

            if(gameState->gameModeType == GAME_MODE_ITEM_COLLECT) {

               //Make sure game is paused
               gameState->gameIsPaused = true;
               //DRAW THE PLAYER HUD

               //DRAW THE PARTICLE SYSTEM
               {
                 renderSetShader(globalRenderGroup, &textureProgram);

                 Entity *collectChest = (Entity *)gameState->entityChestToDisplay; 
                 V3 worldP = easyTransform_getWorldPos(&collectChest->T);
                 drawAndUpdateParticleSystem(globalRenderGroup, &gameState->collectParticleSystem, worldP, appInfo->dt, v3(0, 0, 0), COLOR_GOLD, true); 
               }
               ///////////////////////////////////////////

              bool finished = drawAndUpdateMessageSystem(appInfo, gameState, manager, weatherState, appInfo->transitionState, tweakY, tweakWidth, tweakSpacing, gameFont, &mainFrameBuffer);


              //Exit back to game
              if(finished) {
                   
                   assert(gameState->gameModeType == GAME_MODE_PLAY);
                   gameState->gameIsPaused = false;
                   gameState->entityChestToDisplay = 0;

                   easyAnimation_emptyAnimationContoller(&((Entity *)(manager->player))->animationController, &gameState->animationFreeList);
                   easyAnimation_addAnimationToController(&((Entity *)(manager->player))->animationController, &gameState->animationFreeList, gameState->wizardIdleForward, ((Entity *)(manager->player))->animationRate);   
                   
               }
            }

            if(gameState->gameModeType == GAME_MODE_CUT_SCENE) {

                //NOTE: Update the black bars


                //NOTE: Draw the black bars
                float fuaxWidth = 1920.0f;
                float fuaxHeight = appInfo->aspectRatio_yOverX*fuaxWidth;
                        
                float height_of_cutscene = 0.25f;
                float height_of_cutscene_half = 0.5f - (0.5*height_of_cutscene);

                setViewTransform(globalRenderGroup, mat4());
                setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

                Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth, height_of_cutscene*fuaxHeight, 0)), v3(0, height_of_cutscene_half*fuaxHeight, 0.5f));
                setModelTransform(globalRenderGroup, T);
                renderDrawQuad(globalRenderGroup, COLOR_BLACK);

                T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth, height_of_cutscene*fuaxHeight, 0)), v3(0, -height_of_cutscene_half*fuaxHeight, 0.5f));
                setModelTransform(globalRenderGroup, T);
                renderDrawQuad(globalRenderGroup, COLOR_BLACK);
            }

            if(gameState->gameModeType == GAME_MODE_READING_TEXT) {

                //Make sure game is paused
                gameState->gameIsPaused = true;
                //DRAW THE PLAYER HUD

                //////////////NOTE: Was blurring when in read test mode
                // easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                // endBuffer = &bloomFrameBuffer;
                // renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);
                ////////////////////////////

               drawAndUpdateMessageSystem(appInfo, gameState, manager, weatherState, appInfo->transitionState, tweakY, tweakWidth, tweakSpacing, gameFont, &mainFrameBuffer);
            }
           //  ///////////////////////////

            
            if(gameState->isLookingAtItems) {

                EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);

                gameState->gameIsPaused = true;
                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;

                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);

                setViewTransform(globalRenderGroup, mat4());

                float fuaxWidth = 1920.0f;
                float fuaxHeight = appInfo->aspectRatio_yOverX*fuaxWidth;
                
                setProjectionTransform(globalRenderGroup, OrthoMatrixToScreen(fuaxWidth, fuaxWidth*appInfo->aspectRatio_yOverX));

                switch(gameState->inventoryMenuType) {
                    case GAME_INVENTORY_MENU_MAP: {
                        Texture *mapTex = findTextureAsset("map.jpg");

                        ///Color background yellow
                        Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth,fuaxHeight, 0)), v3(0, 0, 0.5f));
                        setModelTransform(globalRenderGroup, T);
                        renderDrawQuad(globalRenderGroup, settingsYellowColor);    
                        ///////////////////////

                        float h = 0.9f*fuaxHeight;
                        //Draw prompt button to continue
                        float aspectRatio = 1.0f / mapTex->aspectRatio_h_over_w;
                        T = Matrix4_translate(Matrix4_scale(mat4(), v3(h*aspectRatio, h, 0)), v3(0, 0, 0.4f));
                        setModelTransform(globalRenderGroup, T);
                        renderDrawSprite(globalRenderGroup, mapTex, COLOR_WHITE);
                        ///////


                    } break;
                    case GAME_INVENTORY_MENU_ITEMS: {

                    } break;
                    case GAME_INVENTORY_MENU_MAIN: {
                        V2 size = getBounds("ITEMS", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 2, gameState->fuaxResolution, 1);
                        outputTextNoBacking(gameFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 100, 0.1f, gameState->fuaxResolution, "ITEMS", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 2, true, 1);
                        

                        //Update animation timer
                        // gameState->lookingAt_animTimer.current = lerp(gameState->lookingAt_animTimer.current, 20.0f*clamp01(appInfo->dt), gameState->lookingAt_animTimer.target);
                        ////


                        //DRAW BACKGROUND

                        Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(fuaxWidth, fuaxHeight, 0)), v3(0, 0, 0.4f));
                            

                        V4 c = COLOR_GOLD;
                        c.w = 0.5f;
                        setModelTransform(globalRenderGroup, T);
                        renderDrawQuad(globalRenderGroup, c);
                                                
                        //DRAW PLAYER
                        Texture *playerPoseTexture = findTextureAsset("player_pose.png");

                        renderSetShader(globalRenderGroup, &pixelArtProgramPlain);

                        float pWidth = 0.3f*fuaxWidth;
                        T = Matrix4_translate(Matrix4_scale(mat4(), v3(pWidth, playerPoseTexture->aspectRatio_h_over_w*pWidth, 0)), v3(-220, 0, 0.3f));
                        
                        setModelTransform(globalRenderGroup, T);
                        renderDrawSprite(globalRenderGroup, playerPoseTexture, COLOR_WHITE);

                        renderSetShader(globalRenderGroup, &glossProgram);
                        
                        //////////////////////////////////////////////////////////////
                        Texture *t = findTextureAsset("inventory_spot.png");
                        Texture *hover_t = findTextureAsset("targeting2.png");
                        float xAt = 0;
                        float yAt = 0;

                        float spacing = 180;
                        float circleSize = 150;

                        for(int i = 1; i < arrayCount(gameState->itemSpots) + 1; ++i) {   

                            Texture *t_todraw = t;

                             ItemInfo *itemI = &gameState->itemSpots[i - 1];

                            if(gameState->indexInItems == (i - 1)) {
                                gameState->inventoryBreathSelector += appInfo->dt;

                                if(gameState->inventoryBreathSelector > 1.0f) {
                                    gameState->inventoryBreathSelector -= 1.0f;
                                }

                                renderSetShader(globalRenderGroup, &pixelArtProgramPlain);

                                float s = smoothStep00(1.3f*circleSize, gameState->inventoryBreathSelector, 1.6f*circleSize);

                                T = Matrix4_translate(Matrix4_scale(mat4(), v3(s, s, 0)), v3(0.1f*fuaxWidth + xAt, -0.15f*fuaxHeight + yAt, 0.3f));
                                                    
                                setModelTransform(globalRenderGroup, T);
                                renderDrawSprite(globalRenderGroup, hover_t, COLOR_WHITE);


                                if(itemI->type != ENTITY_NULL) {
                                    char *s = getInventoryString(itemI->type);
                                    float descripX = 0.5f*gameState->fuaxResolution.x;
                                    outputTextNoBacking(gameFont, descripX, gameState->fuaxResolution.y - 200, 0.1f, gameState->fuaxResolution, s, rect2fMinMax(descripX, 0, 0.9f*gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(1, 1, 1, 1), 1.4f, true, 1);
                                }
                                

                                t_todraw = findTextureAsset("inventory_spot1.png");

                                //UPDATE PLAYER EQUIPING ITEM
                                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_Z)) {
                                    
                                    //NOTE: Equip item sound
                                    playGameSound(&globalLongTermArena, gameState->equipItemSound, 0, AUDIO_BACKGROUND);

                                    gameState->playerHolding[0] = &gameState->itemSpots[i - 1];

                                    gameState->animationItemTimersHUD[0] = 0.0f;

                                }

                                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_X)) {
                                    //NOTE: Equip item sound
                                    playGameSound(&globalLongTermArena, gameState->equipItemSound, 0, AUDIO_BACKGROUND);

                                    gameState->playerHolding[1] = &gameState->itemSpots[i - 1];

                                    gameState->animationItemTimersHUD[1] = 0.0f;

                                }
                                //////////////////////
                            }


                            float x = 0.1f*fuaxWidth + xAt;
                            float y = -0.15f*fuaxHeight + yAt;
                            T = Matrix4_translate(Matrix4_scale(mat4(), v3(circleSize, circleSize, 0)), v3(x, y, 0.4f));
                            
                            setModelTransform(globalRenderGroup, T);
                            renderDrawSprite(globalRenderGroup, t_todraw, COLOR_WHITE);

                           
                            if(itemI->type != ENTITY_NULL) {
                                renderSetShader(globalRenderGroup, &pixelArtProgramPlain);
                                Texture *inventoryTexture = getInvetoryTexture(itemI->type);

                                float tWidth = circleSize;
                                float tHeight = circleSize*inventoryTexture->aspectRatio_h_over_w;

                                if(tWidth < tHeight) {
                                    tWidth = circleSize/inventoryTexture->aspectRatio_h_over_w;
                                    tHeight = circleSize;
                                }

                                 T = Matrix4_translate(Matrix4_scale(mat4(), v3(tWidth, tHeight, 0)), v3(x, y, 0.2f));
                                 setModelTransform(globalRenderGroup, T);
                                renderDrawSprite(globalRenderGroup, inventoryTexture, COLOR_WHITE);
                                renderSetShader(globalRenderGroup, mainShader);

                                if(itemI->isDisposable) {
                                    float offset = 50;
                                    char *str = easy_createString_printf(&globalPerFrameArena, "%d", itemI->count);
                                    outputTextNoBacking(gameFont, x + 0.5f*fuaxWidth + offset, 0.5f*fuaxHeight - y + offset, 0.1f, gameState->fuaxResolution, str, rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), COLOR_BLACK, 1.3f, true, 1);
                                }
                                
                                
                            }
                            

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
                    } break;
                }
                
                easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

                ////////////////////////////////////////////////////////////////////////////


                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            } else if(gameState->gameModeType == GAME_MODE_GAME_OVER) {
                

                gameState->gameIsPaused = true;
                easyRender_blurBuffer_cachedBuffer(&mainFrameBuffer, &bloomFrameBuffer, &cachedFrameBuffer, 0);
                endBuffer = &bloomFrameBuffer;
                renderSetFrameBuffer(endBuffer->bufferId, globalRenderGroup);
                setViewTransform(globalRenderGroup, cameraLookAt_straight);


                V2 size = getBounds("GAME OVER", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), gameFont, 2, gameState->fuaxResolution, 1);
                outputTextNoBacking(gameFont, 0.5f*gameState->fuaxResolution.x - 0.5f*size.x, 0.5*gameState->fuaxResolution.y, 0.1f, gameState->fuaxResolution, "GAME OVER", rect2fMinMax(0, 0, gameState->fuaxResolution.x, gameState->fuaxResolution.y), v4(0, 0, 0, 1), 2, true, 1);
    
                
                //Draw prompt button to continue
                {

                    EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);

                    float fuaxWidth = 1920.0f;
                    float fuaxHeight = fuaxWidth*appInfo->aspectRatio_yOverX;

                   
                    setViewTransform(globalRenderGroup, mat4());
                    Matrix4 projection = OrthoMatrixToScreen(fuaxWidth, fuaxHeight);
                    setProjectionTransform(globalRenderGroup, projection);

                    renderSetShader(globalRenderGroup, &textureProgram);
                    gameState->gameOverHoverScale += 0.3f*appInfo->dt;

                    if(gameState->gameOverHoverScale > 1.0f) {
                        gameState->gameOverHoverScale = 1.0f - gameState->gameOverHoverScale;
                    }

                    float hoverScale = smoothStep00(1.3f, gameState->gameOverHoverScale, 1.5f);


                    //draw backing
                    Matrix4 T = Matrix4_translate(Matrix4_scale(mat4(), v3(800, 500, 0)), v3(0, 0, 0.5f));
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, findTextureAsset("quill.png"), COLOR_WHITE);

                    T = Matrix4_translate(Matrix4_scale(mat4(), v3(hoverScale*100, hoverScale*100*gameState->spacePrompt->aspectRatio_h_over_w, 0)), v3(0, -100, 0.4f));
                    setModelTransform(globalRenderGroup, T);
                    renderDrawSprite(globalRenderGroup, gameState->spacePrompt, COLOR_WHITE);


                    easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

                }
                ///////////////////////////////////////////////

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

                if(wasPressed(appInfo->keyStates.gameButtons, BUTTON_SPACE)) {
                    EntityResetOnDieData *playerResetData = (EntityResetOnDieData *)easyPlatform_allocateMemory(sizeof(EntityResetOnDieData), EASY_PLATFORM_MEMORY_ZERO);;

                    playerResetData->gameState = gameState;

                    Entity *fireEntity = findEntityById(manager, gameState->playerSaveProgress.playerInfo.lastCheckPointId);

                    // assert(fireEntity);
                    if(fireEntity) {
                        
                        playerResetData->playerStartP = v3_plus(fireEntity->T.pos, v3(0, -0.5, 0));
                        playerResetData->player = (Entity *)manager->player;

                        //Add transition 
                        EasySceneTransition *transition = EasyTransition_PushTransition(appInfo->transitionState, playerDiedReset, playerResetData, EASY_TRANSITION_FADE);
            
                    } else {
                        //NOTE: If no fire to restart at, get last position in save file
                        entityManager_emptyEntityManager(manager, &gameState->physicsWorld);
                        letGoOfSelectedEntity(editorState);
                        gameScene_loadScene(gameState, manager, gameState->currentSceneName, weatherState);

                        gameState->gameModeType = GAME_MODE_PLAY;
                        gameState->gameIsPaused = false;
                    }

                }
            }

            
            //@MEMORY
           //  //NOTE(ollie): Update the console
            if(easyConsole_update(&appInfo->console, &consoleKeyStates, appInfo->dt, (resolution.y / resolution.x))) {
                EasyToken token = easyConsole_getNextToken(&appInfo->console);
                
                bool noMatch = true;
                if(token.type == TOKEN_WORD) {
                    bool responseYes = (token.size == 1 && stringsMatchNullN("y", token.at, token.size)) || (token.size == 3 && stringsMatchNullN("yes", token.at, token.size));
                    

                    if(appInfo->console.askingQuestion  && responseYes) {
                        appInfo->console.askingQuestion = false;
                         noMatch = false;

                        if(appInfo->console.questionId == 1) { //Save Scene Id
                            gameScene_saveScene(gameState, manager, gameState->sceneFileNameTryingToSave);
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

                    if(stringsMatchNullN("level", token.at, token.size)) {
                        noMatch = false;
                        easyConsole_addToStream(DEBUG_globalEasyConsole, gameState->currentSceneName);
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
                                gameScene_saveScene(gameState, manager, sceneFileName);
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
                                letGoOfSelectedEntity(editorState);
                                //NOTE: MEMEORY LEAK HERE. We don' free the scene file name when we load a new scene
                                gameScene_loadScene(gameState, manager, nullTerminateArena(token.at, token.size, &globalLongTermArena), weatherState);
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

            //clear 2d lights from render group
            globalRenderGroup->light2dCountForFrame = 0;
            entitiesRenderGroup->light2dCountForFrame = 0;

            /* //display shadow map 
            {

                EasyRender_ShaderAndTransformState state = easyRender_saveShaderAndTransformState(globalRenderGroup);

                Texture shadowTex = {};
                shadowTex.width = resolution.x;
                shadowTex.height = resolution.y;
                shadowTex.uvCoords = rect2f(0, 0, 1, 1);
                shadowTex.aspectRatio_h_over_w = easyRender_getTextureAspectRatio_HOverW(&shadowTex);
                shadowTex.name = "shadowMap Texture";
                shadowTex.id = shadowMapBuffer.depthId;

                setViewTransform(globalRenderGroup, mat4());
                Matrix4 projection = OrthoMatrixToScreen_BottomLeft(resolution.x, resolution.y);
                setProjectionTransform(globalRenderGroup, projection);

                EasyTransform T;
                easyTransform_initTransform_withScale(&T, v3(200, 200, 0.4f), v3(400, 400*appInfo->aspectRatio_yOverX, 1), EASY_TRANSFORM_NO_ID); 

                setModelTransform(globalRenderGroup, easyTransform_getTransform(&T));
                
                renderSetShader(globalRenderGroup, &displayShadowMapProgram);
                renderDrawSprite(globalRenderGroup, &shadowTex, COLOR_WHITE);

                drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

                easyRender_restoreShaderAndTransformState(globalRenderGroup, &state);

            }
            */

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
