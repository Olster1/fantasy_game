#Example of a program:
```
#include "defines.h"
#include "easy_headers.h"

//NOTE: All your custom header files


int main(int argc, char *args[]) {
    
    EASY_ENGINE_ENTRY_SETUP()
    	
    //NOTE: These are defined in your defines.h file
    V2 screenDim = v2(DEFINES_WINDOW_SIZE_X, DEFINES_WINDOW_SIZE_Y); 
    V2 resolution = v2(DEFINES_RESOLUTION_X, DEFINES_RESOLUTION_Y);
    	
    OSAppInfo *appInfo = easyOS_createApp("Easy Engine", &screenDim, false);
    
    if(appInfo->valid) {

        easyOS_setupApp(appInfo, &resolution, RESOURCE_PATH_EXTENSION);

        //NOTE: example of how you might create frame buffers
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

        //NOTE: Load all images in this folder into your Assets so you can access them in your program. This is a relative path name
        loadAndAddImagesToAssets("img/engine_icons/");

        //NOTE: can use this version for an absolute name
        loadAndAddImagesToAssets_(concatInArena(appInfo->saveFolderLocation, "/", &globalPerFrameArena));
        	
        //NOTE: create a camera. You can create as many cameras as you like in your program
        EasyCamera camera;
        easy3d_initCamera(&camera, v3(0, 0, 0));
        
        EasyTransform sunTransform;
        easyTransform_initTransform(&sunTransform, v3(0, -10, 0), EASY_TRANSFORM_TRANSIENT_ID);
        
        EasyLight *light = easy_makeLight(&sunTransform, EASY_LIGHT_DIRECTIONAL, 1.0f, v3(1, 1, 1));
        easy_addLight(globalRenderGroup, light);
        

        ///////////*********** This is now your main program loop *************/////////////////

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

            ///////////////////////*********** See if the user dragged any files into the program **************////////////////////

            //NOTE(ollie): File dropped something onto the program
            if(appInfo->keyStates.droppedFilePath) {
                //NOTE(ollie): copy the file to a new folder

                //NOTE(ollie): Check if a folder exists, if not make one
                if(!platformDoesDirectoryExist(appInfo->saveFolderLocation)) {
                    platformCreateDirectory(appInfo->saveFolderLocation);
                }
                ////////////////////////////////////////////////////////////////////

                ///////////////////////************ Check if assets folder exists *************////////////////////

                char *programFolderAssetName = concatInArena(appInfo->saveFolderLocation, "/Assets", &globalPerFrameArena);
                
                //NOTE(ollie): Check if a folder exists, if not make one
                if(!platformDoesDirectoryExist(programFolderAssetName)) {
                    platformCreateDirectory(programFolderAssetName);
                }

                ///////////////////////************ Copy the file over to the folder*************////////////////////

                char *fileNameToCopy = appInfo->keyStates.droppedFilePath;

                char *shortName = getFileLastPortionWithArena(fileNameToCopy, &globalPerFrameArena);

                char *fullName = concatInArena(concatInArena(programFolderAssetName, "/", &globalPerFrameArena), shortName, &globalPerFrameArena);

                FileContents fileToCopyContents = platformReadEntireFile(fileNameToCopy, false);

                assert(fileToCopyContents.valid);

                game_file_handle handle = platformBeginFileWrite(fullName);

                if(!handle.HasErrors) {
                    platformWriteFile(&handle, fileToCopyContents.memory, fileToCopyContents.fileSize, 0);
                } else {
                    assert(false);
                }
                
                platformEndFile(handle);

                easyFile_endFileContents(&fileToCopyContents);


                ////////////////////////////////////////////////////////////////////
            }

            //////////////////////////////// Updating the camera ////////////////////////////////////
            
            
            EasyCamera_MoveType camMove = (EasyCamera_MoveType)(EASY_CAMERA_MOVE | EASY_CAMERA_ROTATE | EASY_CAMERA_ZOOM);
                
            easy3d_updateCamera(&camera, &appInfo->keyStates, 1, 100.0f, appInfo->dt, camMove);

            easy_setEyePosition(globalRenderGroup, camera.pos);
            
            // update camera first
            Matrix4 viewMatrix = easy3d_getWorldToView(&camera);
            Matrix4 perspectiveMatrix = projectionMatrixFOV(camera.zoom, resolution.x/resolution.y);

            ///////////////////////************* You have to make sure you draw your render group once you have finished using it ************////////////////////


            drawRenderGroup(globalRenderGroup, (RenderDrawSettings)(RENDER_DRAW_SORT));

            ////////////////////////////////////////////////////////////////////

            //NOTE: Can clear the depth buffer as well if you need to make sure something is on top
            renderClearDepthBuffer(toneMappedBuffer.bufferId);
            
            
            //NOTE(ollie): Make sure the transition is on top
            renderClearDepthBuffer(mainFrameBuffer.bufferId);
            
            //NOTE: You can add custom console commands here
            if(easyConsole_update(&appInfo->console, &appInfo->keyStates, appInfo->dt, (resolution.y / resolution.x))) {
                EasyToken token = easyConsole_getNextToken(&appInfo->console);
                if(token.type == TOKEN_WORD) {
                    // if(stringsMatchNullN("your custom command", token.at, token.size)) {
                    // }
                } else {
                    easyConsole_parseDefault(&appInfo->console, token);
                }
            }

            //////////////////////////////////////////////////////////////////////////////////////////////

            easyOS_endFrame(resolution, screenDim, mainFrameBuffer.bufferId, appInfo, appInfo->hasBlackBars);
            DEBUG_TIME_BLOCK_FOR_FRAME_END(beginFrame, wasPressed(appInfo->keyStates.gameButtons, BUTTON_F4))
            DEBUG_TIME_BLOCK_FOR_FRAME_START(beginFrame, "Per frame")
            easyOS_endKeyState(&appInfo->keyStates);
        }
        easyOS_endProgram(appInfo);
    }
    
    return(0);
}


```

