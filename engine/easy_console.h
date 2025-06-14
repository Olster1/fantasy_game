/*
	How to use: 
	
	//NOTE(ollie): Check if the console is open
	easyConsole_isOpen(&console)
*/

typedef enum {
	EASY_CONSOLE_CLOSED,
	EASY_CONSOLE_OPEN_MID,
	EASY_CONSOLE_OPEN_LARGE,

	///
	EASY_CONSOLE_COUNT
} EasyConsoleState;

typedef struct {
	ButtonType hotkey;
	EasyConsoleState state;

	Timer expandTimer;
	Timer focusGlowTimer;

	InputBuffer buffer;

	//NOTE(ollie): Since the buffer doesn't resize for the input, it's ok if we just have a static array here
	char lastString[EASY_STRING_INPUT_BUFFER_SIZE_INCREMENT];

	int streamSize;
	int streamAt;
	char *bufferStream; //allocated

	bool isInFocus;

	//For console to keep state across multiple queries, so you can ask questions. 
	int questionId;
	bool askingQuestion;

	EasyTokenizer tokenizer;
} EasyConsole;

static inline bool easyConsole_isConsoleOpen(EasyConsole *c) {
	return (c->state != EASY_CONSOLE_CLOSED);
}

static EasyConsole *DEBUG_globalEasyConsole;

inline void easyConsole_initConsole(EasyConsole *c, ButtonType hotkey) {
	DEBUG_TIME_BLOCK()
	c->hotkey = hotkey;
	c->state = EASY_CONSOLE_CLOSED;
	c->expandTimer = initTimer(0.2f, false);
	turnTimerOff(&c->expandTimer);

	c->focusGlowTimer = initTimer(1.2f, false);
	turnTimerOff(&c->focusGlowTimer);
	

	easyString_initInputBuffer(&c->buffer, false);

	c->tokenizer.parsing = false;

	c->questionId = 0;
	c->askingQuestion = false;

	c->streamAt = 0;
	c->streamSize = 1000;
	c->bufferStream = pushArray(&globalLongTermArena, c->streamSize, char);
	c->bufferStream[0] = '\0';
}


#define easyConsole_addToStream(c, toAdd) easyConsole_addToStream_(c, toAdd, __LINE__, __FILE__)

inline void easyConsole_addToStream_(EasyConsole *c, char *toAdd, int lineNumber, char *file) {
	DEBUG_TIME_BLOCK()
	MemoryArenaMark perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);

	char *at = easy_createString_printf(&globalPerFrameArena, "%d %.2fs: %s ... %s", lineNumber, globalTimeSinceStart, toAdd, getFileLastPortionWithArena(file, &globalPerFrameArena));

	// [0]1|[1]2|[2]3|[3]4|[X]

	//TODO(ollie): Make new string length function that differentiates between string size in bytes, and string unicode character length
	u32 newSize = c->streamAt + easyString_getSizeInBytes_utf8(toAdd);
	//NOTE(ollie): We reached maximum capacity, so just nuke the whole buffer
	if(newSize > (c->streamSize - 2)) { //NOTE(ollie): -2 for Null terminator space & newline
		c->streamAt = 0;	
		c->bufferStream[0] = '\0';
	}

	while(*at) {
		char *prevPtr = at;
		easyUnicode_utf8_codepoint_To_Utf32_codepoint((char **)&at, true);
		s32 sizeInBytesOfUnicode = (s32)(at - prevPtr);
		assert(sizeInBytesOfUnicode >= 0);

		//NOTE(ollie): This is to make sure we keep all unicode bytes together
		if((c->streamAt + sizeInBytesOfUnicode) <= (c->streamSize - 2)) { //NOTE(ollie): minus 2 for newline as well
			easyPlatform_copyMemory(&c->bufferStream[c->streamAt], prevPtr, sizeInBytesOfUnicode);
			c->streamAt += sizeInBytesOfUnicode;		
		}
	}

	assert(c->streamAt <= (c->streamSize - 2));
	//NOTE(ollie): Add null terminator now
	c->bufferStream[c->streamAt++] = '\n';
	assert(c->streamAt < c->streamSize);
	c->bufferStream[c->streamAt] = '\0';

	releaseMemoryMark(&perFrameArenaMark);
}


static inline void easyConsole_pushFloat(EasyConsole *c, float i) {
	DEBUG_TIME_BLOCK()
	char *buf = easy_createString_printf(&globalPerFrameArena, "%f", i);
	easyConsole_addToStream(c, buf);
}


static inline void easyConsole_pushV2(EasyConsole *c, V2 i) {
	DEBUG_TIME_BLOCK()
	char *buf = easy_createString_printf(&globalPerFrameArena, "%f, %f", i.x, i.y);
	easyConsole_addToStream(c, buf);
}

static inline void easyConsole_pushV3(EasyConsole *c, V3 i) {
	DEBUG_TIME_BLOCK()
	char *buf = easy_createString_printf(&globalPerFrameArena, "%f, %f, %f", i.x, i.y, i.z);
	easyConsole_addToStream(c, buf);
}


static inline void easyConsole_pushInt(EasyConsole *c, int i) {
	DEBUG_TIME_BLOCK()
	char *buf = easy_createString_printf(&globalPerFrameArena, "%d", i);
	easyConsole_addToStream(c, buf);
}

inline void easyConsole_setHotKey(EasyConsole *c, ButtonType hotkey) {
	c->hotkey = hotkey;
}

inline bool easyConsole_isOpen(EasyConsole *c) {
	return (c->state != EASY_CONSOLE_CLOSED);
	
}

inline bool easyConsole_update(EasyConsole *c, AppKeyStates *keyStates, float dt, float aspectRatio_yOverX) {
	DEBUG_TIME_BLOCK()
	float fuaxWidth = 1920;
	V2 fuaxResolution = v2(fuaxWidth, fuaxWidth*aspectRatio_yOverX);

	bool wasPressed = false;
	if(wasPressed(keyStates->gameButtons, c->hotkey)) {
		c->state = (EasyConsoleState)((int)c->state + 1);
		if(c->state >= (int)EASY_CONSOLE_COUNT) {
			c->state = EASY_CONSOLE_CLOSED;
			c->isInFocus = false;
			//Stop asking any questions
			c->askingQuestion = false;
			turnTimerOff(&c->focusGlowTimer);
		} else {
			c->isInFocus = true;
			turnTimerOn(&c->focusGlowTimer);
		}
		turnTimerOn(&c->expandTimer);
		wasPressed = true;
	}

	//NOTE: Toggle Focus
	if(wasPressed(keyStates->gameButtons, BUTTON_ESCAPE) && c->state != EASY_CONSOLE_CLOSED) {
		c->isInFocus = !c->isInFocus;
	}

	float v = 1.0f;
	if(isOn(&c->expandTimer)) {
		TimerReturnInfo info = updateTimer(&c->expandTimer, dt);
		v = info.canonicalVal;
		if(info.finished) {
			turnTimerOff(&c->expandTimer);
		}
	}
	float height = 0;//
	switch(c->state) {
		case EASY_CONSOLE_CLOSED: {
			height = smoothStep01(0.8f*fuaxResolution.y, v, 0);
		} break;
		case EASY_CONSOLE_OPEN_MID: {
			height = smoothStep01(0, v, 0.3f*fuaxResolution.y);
		} break;
		case EASY_CONSOLE_OPEN_LARGE: {
			height = smoothStep01(0.3f*fuaxResolution.y, v, 0.8f*fuaxResolution.y);
		} break;
		default: {
			assert(!"shouldn't be here");
		} break;
	}

	bool result = false;

	if(c->state != EASY_CONSOLE_CLOSED) {
		result = wasPressed(keyStates->gameButtons, BUTTON_ENTER);
		if(height != 0) {
			if(!result) {
				if(keyStates->inputString && !wasPressed) {
					splice(&c->buffer, keyStates->inputString, true);
				}

				if(wasPressed(keyStates->gameButtons, BUTTON_BACKSPACE)) {
					splice(&c->buffer, "1", false);
				}

				if(wasPressed(keyStates->gameButtons, BUTTON_BOARD_LEFT) && c->buffer.cursorAt > 0) {
					c->buffer.cursorAt--;
				}
				if(wasPressed(keyStates->gameButtons, BUTTON_BOARD_RIGHT) && c->buffer.cursorAt < c->buffer.length) {
					c->buffer.cursorAt++;
				}
			}

			V4 inputBackingColor = COLOR_LIGHT_GREY;

			if(c->isInFocus) {
				if(isOn(&c->focusGlowTimer)) {
					TimerReturnInfo info = updateTimer(&c->focusGlowTimer, dt);
					float v = info.canonicalVal;

					inputBackingColor.x = smoothStep00(0, v, 1);

					if(info.finished) {
						turnTimerOn(&c->focusGlowTimer);
					}
				}
			}

			// renderEnableDepthTest(RenderGroup *group);
			renderDrawRectCenterDim(v3(0.5f*fuaxResolution.x, 0.5f*(height - globalDebugFont->fontHeight), NEAR_CLIP_PLANE + 0.3f), v2(fuaxResolution.x, height - globalDebugFont->fontHeight), COLOR_GREY, 0, mat4TopLeftToBottomLeft(fuaxResolution.y), OrthoMatrixToScreen_BottomLeft(fuaxResolution.x, fuaxResolution.y));
			renderDrawRectCenterDim(v3(0.5f*fuaxResolution.x, height - 0.5f*globalDebugFont->fontHeight, NEAR_CLIP_PLANE + 0.2f), v2(fuaxResolution.x, globalDebugFont->fontHeight), inputBackingColor, 0, mat4TopLeftToBottomLeft(fuaxResolution.y), OrthoMatrixToScreen_BottomLeft(fuaxResolution.x, fuaxResolution.y));
		 	outputText_with_cursor(globalDebugFont, 0, height, NEAR_CLIP_PLANE + 0.1f, fuaxResolution, c->buffer.chars, rect2fMinMax(0, height - globalDebugFont->fontHeight, fuaxResolution.x, height + 0.4f*globalDebugFont->fontHeight), COLOR_BLACK, 1, c->buffer.cursorAt, COLOR_YELLOW, true, 1);
			
			V2 bounds = getBounds(c->bufferStream, rect2fMinMax(0, 0, fuaxResolution.x, height - globalDebugFont->fontHeight), globalDebugFont, 1, fuaxResolution, 1);
			outputTextNoBacking(globalDebugFont, 0, height - globalDebugFont->fontHeight - bounds.y, NEAR_CLIP_PLANE + 0.1f, fuaxResolution, c->bufferStream, rect2fMinMax(0, 0, fuaxResolution.x, height), COLOR_WHITE, 1, true, 1);
		}
	}

	if(result) {
		//NOTE(ollie): Move the current buffer into the saved buffer to tokenize
		nullTerminateBuffer(c->lastString, c->buffer.chars, c->buffer.length);

		//NOTE(ollie): Clear the input buffer out
		c->buffer.length = 0;
		c->buffer.cursorAt = 0;
		c->buffer.chars[0] = '\0';

		c->tokenizer = lexBeginParsing(c->lastString, (EasyLexOptions)(EASY_LEX_OPTION_EAT_WHITE_SPACE | EASY_LEX_DONT_EAT_SLASH_COMMENTS));
	}
	return result;
}

inline EasyToken easyConsole_getNextToken(EasyConsole *console) {
	return lexGetNextToken(&console->tokenizer);
}

inline void easyConsole_clearOutputConsole(EasyConsole *c) {
	c->streamAt = 0;
	c->bufferStream[0] = '\0';
}

inline EasyToken easyConsole_seeNextToken(EasyConsole *console) {
	return lexSeeNextToken(&console->tokenizer);
}

/*
	
	Short cuts: 

	fly - fly the camera around

	camMoveXY - only move in the XY coordinates

	camRotate - toggle camera rotating off/on
*/

inline void easyConsole_parseDefault(EasyConsole *c, EasyToken token) {
	DEBUG_TIME_BLOCK()
	if(stringsMatchNullN("fly", token.at, token.size)) {
        DEBUG_global_IsFlyMode = !DEBUG_global_IsFlyMode;
    } else if(stringsMatchNullN("camMoveXY", token.at, token.size)) {
        DEBUG_global_CameraMoveXY = !DEBUG_global_CameraMoveXY;
    } else if(stringsMatchNullN("pause", token.at, token.size)) {
        DEBUG_global_PauseGame = !DEBUG_global_PauseGame;
    } else if(stringsMatchNullN("bounds", token.at, token.size)) {
        DEBUG_drawBounds = !DEBUG_drawBounds;
    }  else if(stringsMatchNullN("framerate", token.at, token.size)) {
        DEBUG_global_DrawFrameRate = !DEBUG_global_DrawFrameRate;
    } else if(stringsMatchNullN("camRotate", token.at, token.size)) {
        DEBUG_global_CamNoRotate = !DEBUG_global_CamNoRotate;
    } else if(stringsMatchNullN("help", token.at, token.size)) {
        easyConsole_addToStream(c, "camMoveXY");
        easyConsole_addToStream(c, "fly");
        easyConsole_addToStream(c, "camRotate");
        easyConsole_addToStream(c, "pause");
        easyConsole_addToStream(c, "framerate");
    } else if(stringsMatchNullN("command", token.at, token.size)) {
    } else if(stringsMatchNullN("clear", token.at, token.size)) {
    	easyConsole_clearOutputConsole(c);
    	
    } else if(stringsMatchNullN("vsync", token.at, token.size)) {
    	token = easyConsole_getNextToken(c);
    	if(stringsMatchNullN("on", token.at, token.size)) {
    	    DEBUG_global_VsyncIsOn = true;
    	    if(SDL_GL_SetSwapInterval(1) != -1) {
    	    	//TODO(ollie): This seems to be a bug. Swap to wglSetSwapInterval win32 
    	    	const char *error = SDL_GetError();
    	    	easyLogger_addLog("Tried setting vsync, but not supported.");
    	    }
    	} else if(stringsMatchNullN("off", token.at, token.size)) {
    	    DEBUG_global_VsyncIsOn = false;
    		if(SDL_GL_SetSwapInterval(0) != -1) {
    			//TODO(ollie): This seems to be a bug. Swap to wglSetSwapInterval win32 
    			const char *error = SDL_GetError();
    			easyLogger_addLog("Tried unsetting vsync, but not supported.");

    		}
    	} else {
			char helpString[512];
			snprintf(helpString, arrayCount(helpString), "Vsync is %d\n", DEBUG_global_VsyncIsOn); 
		    easyConsole_addToStream(c, helpString);
    	}
    } else if(stringsMatchNullN("guessframerate", token.at, token.size)) {
    	token = easyConsole_getNextToken(c);
    	if(stringsMatchNullN("on", token.at, token.size)) {
    	    DEBUG_global_GuessFramerateFromVsync = true;
    	} else if(stringsMatchNullN("off", token.at, token.size)) {
    	    DEBUG_global_GuessFramerateFromVsync = false;
    	} else {
    		char helpString[512];
    		snprintf(helpString, arrayCount(helpString), "Guess Rate is %d\n", DEBUG_global_GuessFramerateFromVsync); 
    	    easyConsole_addToStream(c, helpString);
    	}
    	
    } else if(stringsMatchNullN("outsideConsole", token.at, token.size)) {
    	// STARTUPINFO startUpInfo = {};
    	// _PROCESS_INFORMATION lpProcessInformation;

    	// if(!CreateProcessA(
    	//    "c:\\windows\\system32\\cmd.exe",
    	//   "/c mkdir",
    	//   NULL,
    	//   NULL,
    	//   FALSE,
    	//   NORMAL_PRIORITY_CLASS,
    	//   NULL,
    	//   "C://",
    	//   &startUpInfo,
    	//   &lpProcessInformation
    	// )) {
    	// 	easyConsole_addToStream(c, "parameter not understood");
    	// }
    } else if(stringsMatchNullN("colliders", token.at, token.size)) {
    	DEBUG_global_ViewColliders = !DEBUG_global_ViewColliders;
    } else {
    	easyConsole_addToStream(c, "parameter not understood");
    } 
	
}

