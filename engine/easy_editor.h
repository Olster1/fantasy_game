
/*
Use example::


easyEditor_initEditor(EasyEditor *e, RenderGroup *group, Font *font, float aspectRatio_yOverX, AppKeyStates *keyStates);

while(running) {

	if(!easyEditor_isInteracting(editor)) {
		Do something
	}

	if(!editor.isHovering) {
		do something else when not hovering over editor
	}

	easyEditor_startWindow(e)
	
	static V3 v = {};
	//blah blah
	easyEditor_pushFloat1(e, &v);
	easyEditor_pushFloat2(e, &v, &v);
	easyEditor_pushFloat3(e, &v, &v, &v);

	easyEditor_endWindow(e); //might not actuall need this
	

	easyEditor_endEditorForFrame(e)
}


*/


#define EASY_EDITOR_MARGIN 10
#define EASY_EDITOR_MIN_TEXT_FIELD_WIDTH 100
#define EASY_EDITOR_SCALE 1.5f //Bigger this number, the smaller the editor ui

typedef enum {
	EASY_EDITOR_DOCK_NONE,
	EASY_EDITOR_DOCK_TOP_RIGHT,
	EASY_EDITOR_DOCK_BOTTOM_RIGHT,
	EASY_EDITOR_DOCK_TOP_LEFT,
	EASY_EDITOR_DOCK_BOTTOM_LEFT,
} EasyEditor_WindowDockType;

typedef struct {
	char *fileName;
	int lineNumber;
	int index;
	int id1;
	char *id2;
} EasyEditorId;

typedef enum  {
		EASY_EDITOR_INTERACT_NULL,
		EASY_EDITOR_INTERACT_WINDOW,
		EASY_EDITOR_INTERACT_TEXT_FIELD,
		EASY_EDITOR_INTERACT_COLOR_WHEEL,
		EASY_EDITOR_INTERACT_SLIDER,
		EASY_EDITOR_INTERACT_BUTTON,
		EASY_EDITOR_INTERACT_DROP_DOWN_LIST,
		EASY_EDITOR_INTERACT_CHECK_BOX,
		EASY_EDITOR_INTERACT_WINDOW_SCROLL_BAR,
} EasyEditorInteractType;

typedef struct {
	EasyEditorId id;

	EasyEditorInteractType type;

	bool isOpen; //used by color wheel & drop down list & window itself

	union {
		struct { //window state
			char *name;
			Rect2f margin;

			V2 topCorner_;
			V2 topCorner;
			V2 at;
			float maxX;

			float yOffset; //NOTE: From the scroll

			//NOTE(ollie): To see if it docks anywhere
			EasyEditor_WindowDockType dockType; 

			bool drawnYet;
		};
		struct { //color selection state
			
			V2 coord; //from the center
			float value; //side bar of black to white
		};
		struct { //text field state
			InputBuffer buffer;	
		};
		struct { //range 
			float min;
			float max;
		};
		struct { //drop down list index
			int dropDownIndex;
			float scrollAt;
			bool holdingScrollBar;
		};
	};
} EasyEditorState;


typedef struct {
	EasyEditorId id;

	EasyEditorInteractType type;

	EasyEditorState *item;
	
	bool visitedThisFrame;

	V2 dragOffset; //for dragging interactions
} EasyEditorInteraction;

typedef struct {
	RenderGroup *group;

	int stateCount;
	EasyEditorState states[128];	

	EasyEditorState *currentWindow;

	EasyFont_Font *font;

	EasyEditorInteraction interactingWith;

	Matrix4 orthoMatrix;
	V2 fuaxResolution;

	float screenRelSize;

	EasyEditorState bogusItem; //for interactions that have nothing that needs storing

	//NOTE(ollie): Hovering booleans - we keep the isHovering around for one extra frame
	//             So things before the editor can use it in the next frame. Then at the end of the
	//			   We assign the new one. 
	bool isHovering;
	bool isHovering_unstable; //shouldn't be used outside of the editor  

	AppKeyStates *keyStates;

	V2 viewportDim;
} EasyEditor;

static inline bool easyEditor_isInteracting(EasyEditor *e) {
	return (e->interactingWith.item);
}


static inline EasyEditorId easyEditor_getNullId() {
	DEBUG_TIME_BLOCK()
	EasyEditorId result = {};
	result.lineNumber = -1;
	result.index = -1;
	result.fileName = 0;
	return result;
}

static inline void easyEditor_stopInteracting(EasyEditor *e) {
	DEBUG_TIME_BLOCK()
	if(e->interactingWith.type == EASY_EDITOR_INTERACT_TEXT_FIELD) {
		EasyEditorState *field = e->interactingWith.item;

		field->buffer.length = 0;
		field->buffer.cursorAt = 0;
	}
	e->interactingWith.item = 0;
	e->interactingWith.type = EASY_EDITOR_INTERACT_NULL;
	e->interactingWith.id = easyEditor_getNullId();
}

static inline void easyEditor_initEditor(EasyEditor *e, RenderGroup *group, EasyFont_Font *font, float aspectRatio_yOverX, AppKeyStates *keyStates, V2 viewportDim) {
	e->group = group;

	e->stateCount = 0;
	e->currentWindow = 0; //used for the local positions of the windows

	e->font = font;

	e->viewportDim = viewportDim;

	easyEditor_stopInteracting(e); //null the interacting with

	//NOTE(ollie): This is a default width, so the editor scales idependently of what the game resolution is
	float fuaxWidth = EASY_EDITOR_SCALE*1920.0f;

	e->fuaxResolution = v2(fuaxWidth, fuaxWidth*aspectRatio_yOverX);

	e->orthoMatrix = OrthoMatrixToScreen_BottomLeft(e->fuaxResolution.x, e->fuaxResolution.y); 
	
	e->screenRelSize = 1;
	e->keyStates = keyStates;
	e->isHovering = false;
	e->isHovering_unstable = false;
}

static inline void easyEditor_startInteracting(EasyEditor *e, EasyEditorState *item, int lineNumber, char *fileName, int index, EasyEditorInteractType type) {
	e->interactingWith.item = item;
	e->interactingWith.id.lineNumber = lineNumber;
	e->interactingWith.id.fileName = fileName;
	e->interactingWith.id.index = index;
	e->interactingWith.visitedThisFrame = true;
	e->interactingWith.type = type;
}

static inline bool easyEditor_idEqual(EasyEditorId id, int lineNumber, char *fileName, int index, int id1 = 0, char *id2 = 0) {
	return (lineNumber == id.lineNumber && cmpStrNull(fileName, id.fileName) && id.index == index && id1 == id.id1 && cmpStrNull(id2, id.id2));
}

static inline EasyEditorState *easyEditor_hasState(EasyEditor *e, int lineNumber, char *fileName, int index, EasyEditorInteractType type, int id1 = 0, char *id2 = 0) {
	EasyEditorState *result = 0;
	for(int i = 0; i < e->stateCount && !result; ++i) {
		EasyEditorState *w = e->states + i;
		if(easyEditor_idEqual(w->id, lineNumber, fileName, index, id1, id2) && type == w->type) {
			result = w;
			assert(result->type == type);
			break;
		}
	}
	return result;
}

static inline EasyEditorState *addState(EasyEditor *e, int lineNumber, char *fileName, int index, EasyEditorInteractType type, int id1 = 0, char *id2 = 0) {
	assert(e->stateCount < arrayCount(e->states));
	EasyEditorState *t = &e->states[e->stateCount++];
	t->id.lineNumber = lineNumber;
	t->id.fileName = fileName;
	t->id.index = index;
	t->id.id1 = id1;
	t->id.id2 = id2;
	
	t->type = type;

	return t;
}

static inline EasyEditorState *easyEditor_addTextField(EasyEditor *e, int lineNumber, char *fileName, int index, int id1, char *id2) {
	EasyEditorState *t = easyEditor_hasState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_TEXT_FIELD, id1, id2);
	if(!t) {
		t = addState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_TEXT_FIELD, id1, id2);

		t->buffer.length = 0;
		t->buffer.cursorAt = 0;

	}
	return t;
}

static inline EasyEditorState *easyEditor_addColorSelector(EasyEditor *e, V4 color, int lineNumber, char *fileName, int index) {
	EasyEditorState *t = easyEditor_hasState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_COLOR_WHEEL);
	if(!t) {
		t = addState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_COLOR_WHEEL);

		t->isOpen = false;
	

		//NOTE(ollie): Set the original color		
		EasyColor_HSV hsv = easyColor_rgbToHsv(color.x, color.y, color.z);

		float rads = easyMath_degreesToRadians(hsv.h);
		t->coord = v2_scale(hsv.s, v2(cos(rads), sin(rads)));
		t->value = hsv.v;
	}
	return t;
}



static inline EasyEditorState *easyEditor_addDropDownSpriteList(EasyEditor *e, int lineNumber, char *fileName, int index, int id1, char *id2) {
	EasyEditorState *t = easyEditor_hasState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_DROP_DOWN_LIST, id1, id2);
	if(!t) {
		t = addState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_DROP_DOWN_LIST, id1, id2);

		t->isOpen = false;
		t->dropDownIndex = 0;
		t->scrollAt = 0;
		t->holdingScrollBar = false;
	}
	return t;
}


static inline EasyEditorState *easyEditor_addDropDownList(EasyEditor *e, int lineNumber, char *fileName, int index, int id1, char *id2) {
	EasyEditorState *t = easyEditor_hasState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_DROP_DOWN_LIST, id1, id2);
	if(!t) {
		t = addState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_DROP_DOWN_LIST, id1, id2);

		t->isOpen = false;
		t->dropDownIndex = 0;
		t->scrollAt = 0;
		t->holdingScrollBar = false;
	}
	return t;
}


typedef enum {
	EASY_SLIDER_HORIZONTAL, 
	EASY_SLIDER_VERTICAL, 
} EasyEditor_SliderType;

static inline EasyEditorState *easyEditor_addSlider(EasyEditor *e, int lineNumber, char *fileName, int index, float min, float max) {
	EasyEditorState *t = easyEditor_hasState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_SLIDER);
	if(!t) {
		t = addState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_SLIDER);

		t->min = min;
		t->max = max;
	}
	return t;
}

#define easyEditor_startWindow(e, name, x, y) easyEditor_startWindow_(e, name, x, y, EASY_EDITOR_DOCK_NONE, __LINE__, __FILE__)
#define easyEditor_startDockedWindow(e, name, dockType) easyEditor_startWindow_(e, name, 0, 0, dockType, __LINE__, __FILE__)

static inline void easyEditor_startWindow_(EasyEditor *e, char *name, float x, float y, EasyEditor_WindowDockType dockType, int lineNumber, char *fileName) {
	EasyEditorState *w = easyEditor_hasState(e, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_WINDOW);
	if(!w) {
		w = addState(e, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_WINDOW);

		w->margin = InfinityRect2f();
		w->topCorner_ = w->topCorner = v2(x, y); //NOTE: Default top corner
		w->isOpen = true;
		w->yOffset = 0;
		
		if(dockType != EASY_EDITOR_DOCK_NONE) {
			easyConsole_addToStream(DEBUG_globalEasyConsole, "NOT NOW");
			//just default the position to out of the screen since we calculate the position at the end of the window
			//NOTE: Couldn't use INFINITY_VALUE since the margin for the text output is using these values and the span of that react was 
			//		between -INFINITY & INFINITY & Y was clamped to onscreen values.  
			w->topCorner = w->topCorner_ = v2(3000.0f, 3000.0f);

		} else {	
			easyConsole_addToStream(DEBUG_globalEasyConsole, "YEA");
		}
		w->drawnYet = false;
	} else {
		w->drawnYet = true;
	}

	assert(w);

	//NOTE: Only reset the values when the window is open
	if(w->isOpen) {
		w->name = name;

		w->dockType = dockType;
		
		w->topCorner.y = w->topCorner_.y - w->yOffset;

		w->at = w->topCorner; 

		w->maxX = w->topCorner.x;
		w->at.y += EASY_EDITOR_MARGIN;
	}
	e->currentWindow = w; 
}



static inline void easyEditor_endWindow(EasyEditor *e) {
	assert(e->currentWindow);
	EasyEditorState *w = e->currentWindow;

	
	int lineNumber = w->id.lineNumber;
	char *fileName = w->id.fileName;

	//update the handle

	Rect2f rect = outputTextNoBacking(e->font, w->topCorner_.x, w->topCorner_.y - e->font->fontHeight - 2*EASY_EDITOR_MARGIN, 1, e->fuaxResolution, w->name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);

		
		//explicilty set the top handle to the right size 
		rect.maxY = w->topCorner_.y - e->font->fontHeight;
		rect.minX = w->topCorner_.x - EASY_EDITOR_MARGIN;
		if(w->isOpen) {
			w->at.x += getDim(rect).x;
		}

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}

		rect.maxX = w->maxX + EASY_EDITOR_MARGIN;

		Rect2f bounds = {};
		
		if(w->isOpen) {
		///////////////////////************** We set the position depending on the dock type ***********////////////////////
		float windowHeight = w->at.y - w->topCorner_.y;
		float windowWidth = w->maxX - rect.minX;

		float maxExtent = w->maxX + EASY_EDITOR_MARGIN;

		switch(w->dockType) {
			case EASY_EDITOR_DOCK_BOTTOM_LEFT: {
				w->topCorner = w->topCorner_ = v2(0, e->fuaxResolution.y - windowHeight);
			} break;
			case EASY_EDITOR_DOCK_BOTTOM_RIGHT: {
				w->topCorner_ = w->topCorner = v2(e->fuaxResolution.x - windowWidth, e->fuaxResolution.y - windowHeight);
				maxExtent = e->fuaxResolution.x;
			} break;
			case EASY_EDITOR_DOCK_TOP_LEFT: {
				w->topCorner_ = w->topCorner = v2(0, 2*e->font->fontHeight);
			} break;
			case EASY_EDITOR_DOCK_TOP_RIGHT: {
				w->topCorner_ = w->topCorner = v2(e->fuaxResolution.x - windowWidth, 2.0f*e->font->fontHeight + 0.4f*EASY_EDITOR_MARGIN);
				maxExtent = e->fuaxResolution.x;
			} break;
			default: {
				//do nothing
			}
		} //END SWITCH STATEMENT



		float totalHeightOfWindow = w->at.y - w->topCorner_.y + 2*EASY_EDITOR_MARGIN + e->font->fontHeight + w->yOffset;
		if(totalHeightOfWindow > e->fuaxResolution.y) 
		{ //bigger than the screen
			EasyEditorState *sh = easyEditor_hasState(e, w->id.lineNumber, w->id.fileName, 0, EASY_EDITOR_INTERACT_WINDOW_SCROLL_BAR);
			if(!sh) {
				sh = addState(e, w->id.lineNumber, w->id.fileName, 0, EASY_EDITOR_INTERACT_WINDOW_SCROLL_BAR);
			}
			float scrollWidth = 50;
			Rect2f scrollLaneWay = {};
			//NOTE: Draw the handle
			if(w->dockType == EASY_EDITOR_DOCK_BOTTOM_LEFT || w->dockType == EASY_EDITOR_DOCK_TOP_LEFT) {
				//NOTE: Draw the handle on the right hand side
				scrollLaneWay = rect2fMinDim(maxExtent, rect.minY, scrollWidth, e->fuaxResolution.y - rect.minY); 
			} else {
				//NOTE: Put it on the right hand size for all the others
				scrollLaneWay = rect2fMinDim(rect.minX - scrollWidth, rect.minY, scrollWidth, e->fuaxResolution.y - rect.minY); 
			}

			renderDrawRect(scrollLaneWay, 10, COLOR_GOLD, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);


			float overhang = totalHeightOfWindow - e->fuaxResolution.y;
			float scrollHeight = e->fuaxResolution.y - rect.minY;

			Rect2f scrollHandle = scrollLaneWay;
			scrollHandle.minY = ((sh->value * overhang) + rect.minY);
			scrollHandle.maxY = scrollHandle.minY + (scrollHeight - overhang);

			// easyConsole_pushFloat(DEBUG_globalEasyConsole, w->at.y);
			// easyConsole_pushFloat(DEBUG_globalEasyConsole, totalHeightOfWindow);
			// easyConsole_pushFloat(DEBUG_globalEasyConsole, overhang);

			//NOTE: Draw the handle
			V4 color = COLOR_BLUE;

			if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), scrollHandle, BOUNDS_RECT) && !e->interactingWith.item) {
				color = COLOR_YELLOW;
				e->isHovering_unstable = true;
				if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					easyEditor_startInteracting(e, w, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_WINDOW_SCROLL_BAR);

					e->interactingWith.dragOffset = v2_minus(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), scrollHandle.min);
				} 
			}


			if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0) && e->interactingWith.type == EASY_EDITOR_INTERACT_WINDOW_SCROLL_BAR) {
				e->interactingWith.visitedThisFrame = true;
				color = COLOR_GREEN;

				if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					easyEditor_stopInteracting(e);
				} else {
					//NPTE: Move the scroll bar
					sh->value = (v2_minus(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), e->interactingWith.dragOffset).y -  rect.minY) / overhang;
					sh->value = clamp01(sh->value);
					assert(sh->value >= 0.0f && sh->value <= 1.0f);
					w->yOffset = sh->value*overhang; 


				}
			}


			if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), scrollHandle, BOUNDS_RECT)) {
				e->isHovering_unstable = true;
			} 

			renderDrawRect(scrollHandle, 1, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);	
		}

	
		//NOTE(ollie): Draw the backing
		bounds = rect2fMinMax(w->topCorner_.x - EASY_EDITOR_MARGIN, w->topCorner_.y - e->font->fontHeight, maxExtent, w->at.y); 
		if(w->drawnYet) {
			renderDrawRect(bounds, 10, COLOR_BLACK, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);
		}

	}
		V4 color = COLOR_BLUE;

		if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), rect, BOUNDS_RECT) && !e->interactingWith.item && w->dockType == EASY_EDITOR_DOCK_NONE) {
			color = COLOR_YELLOW;
			e->isHovering_unstable = true;
			if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				easyEditor_startInteracting(e, w, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_WINDOW);

				e->interactingWith.dragOffset = v2_minus(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), w->topCorner_);
			} 
		}


		if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0) &&  e->interactingWith.type == EASY_EDITOR_INTERACT_WINDOW) {
			e->interactingWith.visitedThisFrame = true;
			color = COLOR_GREEN;

			if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				easyEditor_stopInteracting(e);
			} else {
				w->topCorner_ = w->topCorner = v2_minus(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), e->interactingWith.dragOffset);
			}
		}


		if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT) || inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), rect, BOUNDS_RECT)) {
			e->isHovering_unstable = true;
		} 

		if(w->drawnYet) {
			renderDrawRect(rect, 3, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);	
		}

	//////

	////////////////////******* Draw & Update the minimise button on the window

	float minimiseBtn_size = getDim(rect).y;

	Rect2f minimiseRect = rect2fMinDim(rect.maxX - minimiseBtn_size, rect.minY, minimiseBtn_size, minimiseBtn_size);

	V4 minimiseBtn_color = COLOR_GREEN;

	if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), minimiseRect, BOUNDS_RECT) && !e->interactingWith.item) {
		minimiseBtn_color = COLOR_GOLD;
		e->isHovering_unstable = true;
		if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
			w->isOpen = !w->isOpen;
		} 
	}
	renderDrawRect(minimiseRect, 1, minimiseBtn_color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

	//NOTE(ollie): Make sure user has to use a beginWindow call, otherwise it will crash
	e->currentWindow = 0;

	//NOTE(ollie): Draw the render group so things don't overlap the things
	//TODO(ollie): We should have another frame buffer to draw the editor stuff to to stop things overlapping
	// drawRenderGroup(e->group, RENDER_DRAW_SORT);
	//Clear depth buffer aswell 
}

#define easyEditor_alterListIndex(e, newIndex) easyEditor_alterListIndex_(e, newIndex, __LINE__, __FILE__, __LINE__, __FILE__)
#define easyEditor_alterListIndex_withIds(e, newIndex, id1, id2) easyEditor_alterListIndex_(e,  newIndex, __LINE__, __FILE__, id1, id2)
static inline void easyEditor_alterListIndex_(EasyEditor *e, int newIndex, int lineNumber, char *fileName, int id1, char *id2) {
	assert(e->currentWindow);
	EasyEditorState *w = e->currentWindow;
	EasyEditorState *state = easyEditor_addDropDownList(e, lineNumber, fileName, 0, id1, id2);

	state->dropDownIndex = newIndex;
}


#define easyEditor_pushList(e, name, options, optionLength) easyEditor_pushList_(e, name, options, optionLength, __LINE__, __FILE__, __LINE__, __FILE__)
#define easyEditor_pushList_withIds(e, name, options, optionLength, id1, id2) easyEditor_pushList_(e, name, options, optionLength, __LINE__, __FILE__, id1, id2)
static inline int easyEditor_pushList_(EasyEditor *e, char *name, char **options, int optionLength, int lineNumber, char *fileName, int id1, char *id2) {
	assert(e->currentWindow);
	EasyEditorState *w = e->currentWindow;

	w->at.y += EASY_EDITOR_MARGIN;

	V4 color = COLOR_WHITE;

	EasyEditorState *state = easyEditor_addDropDownList(e, lineNumber, fileName, 0, id1, id2);

	assert(optionLength > 0);

	if(state->dropDownIndex >= optionLength) {
		//NOTE(ollie): Should only get in here if the user changes the list between frames. 
		//			   And the one they had selected doesn't exist anymore
		state->dropDownIndex = 0;
	}

	if(w->isOpen) {

		///////////////////////************ Draw the name of the list*************////////////////////
		Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
		w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;


		if(!state->isOpen) {
			///////////////////////************ draw & update to enter the list *************////////////////////
			Rect2f bounds = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, options[state->dropDownIndex], w->margin, COLOR_BLACK, 1, true, e->screenRelSize);
			bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN));

			bool hover = inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT);
			if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0)) {
				e->interactingWith.visitedThisFrame = true;
				color = COLOR_GREEN;

				if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					if(hover) {
						state->isOpen = !state->isOpen; //close or open the menu
						easyEditor_stopInteracting(e);	
					} else {
						easyEditor_stopInteracting(e);	
					}
				}
			} else if(hover) {
				if(!e->interactingWith.item) {
					color = COLOR_YELLOW;	
				}
				
				if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					if(e->interactingWith.item) {
						//NOTE: Clicking on a different cell will swap it
						easyEditor_stopInteracting(e);	
					}
					easyEditor_startInteracting(e, &e->bogusItem, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_DROP_DOWN_LIST);
				} 
			}

			renderDrawRect(bounds, 2, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

			float xAt = (w->at.x + getDim(bounds).x) + EASY_EDITOR_MARGIN;
			if(w->maxX < xAt) {
				w->maxX = xAt;
			}

			w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
		} else {

			float handleWidth = 30;
			float handleHeight = 50;

			float clippedHeight = 0.3f*e->viewportDim.y - handleHeight;
			
			float startY = w->at.y;

			///////////////////////************* Find the total height of the scroll window ************////////////////////
			for(int optionIndex = 0; optionIndex < optionLength; ++optionIndex) {
				char *option = options[optionIndex];

				color = COLOR_WHITE;

				Rect2f bounds = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, options[optionIndex], w->margin, COLOR_BLACK, 1, false, e->screenRelSize);
				bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN));

				w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
			}
			////////////////////////////////////////////////////////////////////

			//NOTE(ollie): Find the height of the window
			float acutalHeight = w->at.y - startY;

			//NOTE(ollie): Reset the y height
			w->at.y = startY; 

			bool neededScroll = acutalHeight > clippedHeight;

			

			///////////////////////************ Needed scroll *************////////////////////
			if(neededScroll) {
				V2 handlePos = v2(w->at.x - handleWidth - EASY_EDITOR_MARGIN, (startY - e->font->fontHeight) + (clippedHeight - handleHeight)*state->scrollAt);

				Rect2f scrollHandle = rect2fMinDim(handlePos.x, handlePos.y, handleWidth, handleHeight);
				
				V4 handleColor = COLOR_GOLD;

				if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), scrollHandle, BOUNDS_RECT)) {
					handleColor = COLOR_YELLOW;
					 if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
						//NOTE(ollie): grab the handle
						state->holdingScrollBar = true;									 	
					 }
					
				}

				if(state->holdingScrollBar) {
					handleColor = COLOR_GREEN;

					///////////////////////************ Update the handle position *************////////////////////
					float goalPos = easyInput_mouseToResolution(e->keyStates, e->fuaxResolution).y - startY;
					//update position
					state->scrollAt = lerp(clippedHeight*state->scrollAt, 0.4f, goalPos) / (clippedHeight - handleHeight);
					state->scrollAt = clamp01(state->scrollAt);

					////////////////////////////////////////////////////////////////////

					//NOTE(ollie): Let go of the handle
					if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
						state->holdingScrollBar = false;
					}
				}
				//NOTE(ollie): Reclculate the scroll handle position
				scrollHandle = rect2fMinDim(handlePos.x, handlePos.y, handleWidth, handleHeight);

				//NOTE(ollie): Draw the scroll handle
				renderDrawRect(scrollHandle, NEAR_CLIP_PLANE, handleColor, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);
			}
			////////////////////////////////////////////////////////////////////

			//NOTE(ollie): Push the scissors on the stack
			float scrollOffset = 0;

			Rect2f scrollWindow = rect2fMinDim(w->at.x, startY - e->font->fontHeight, e->fuaxResolution.x, clippedHeight);

			if(neededScroll) {
				easyRender_pushScissors(globalRenderGroup, scrollWindow, NEAR_CLIP_PLANE, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix, e->viewportDim);
				scrollOffset = state->scrollAt*(acutalHeight - clippedHeight);
			}

			
			///////////////////////************ Draw & Update the list *************////////////////////
			for(int optionIndex = 0; optionIndex < optionLength; ++optionIndex) {
				char *option = options[optionIndex];

				float yAt = w->at.y - scrollOffset;
				color = COLOR_WHITE;

				bool shouldDraw = (!neededScroll || (yAt >= (startY -  e->font->fontHeight) && yAt < (startY + clippedHeight + 1.2f*e->font->fontHeight)));

				Rect2f bounds = outputTextNoBacking(e->font, w->at.x, yAt, 1, e->fuaxResolution, options[optionIndex], w->margin, COLOR_BLACK, 1, shouldDraw, e->screenRelSize);
				bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN));


				if(shouldDraw) {
					
					bool hover = inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT);
					if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, optionIndex + 1)) { //+ 1 because 0 is taken by the box that can open to display the list 
						e->interactingWith.visitedThisFrame = true;
						color = COLOR_GREEN;

						if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
							if(hover) {
								//actually comiting to this option
								state->dropDownIndex = optionIndex;
								state->isOpen = false;
							} else {
								easyEditor_stopInteracting(e);	
							}
						}
					} else if(hover) {
						if(!e->interactingWith.item) {
							color = COLOR_YELLOW;	
						}
						
						if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
							if(e->interactingWith.item) {
								//NOTE: Clicking on a different cell will swap it
								easyEditor_stopInteracting(e);	
							}
							easyEditor_startInteracting(e, &e->bogusItem, lineNumber, fileName, optionIndex + 1, EASY_EDITOR_INTERACT_DROP_DOWN_LIST);
						} 
					}
					
				}
				if(shouldDraw) {
					renderDrawRect(bounds, 2, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);
				}

				w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;

				float xAt = (w->at.x + getDim(bounds).x) + EASY_EDITOR_MARGIN;
				if(w->maxX < xAt) {
					w->maxX = xAt;
				}
			}
			///////////////////////************ Pop the scissors*************////////////////////
			if(neededScroll) {
				easyRender_disableScissors(globalRenderGroup);
			
				w->at.y = startY + clippedHeight + EASY_EDITOR_MARGIN;
			}



		}


		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}

		w->at.x = w->topCorner.x;
	}

	return state->dropDownIndex;

}



///////////////////////// LIST TEXTURES /////////////////////////////////////

#define easyEditor_pushSpriteList(e, name, options, optionLength) easyEditor_pushSpriteList_(e, name, options, optionLength, __LINE__, __FILE__, __LINE__, __FILE__)
#define easyEditor_pushSpriteList_withIds(e, name, options, optionLength, id1, id2) easyEditor_pushSpriteList_(e, name, options, optionLength, __LINE__, __FILE__, id1, id2)
static inline int easyEditor_pushSpriteList_(EasyEditor *e, char *name, Texture **options, int optionLength, int lineNumber, char *fileName, int id1, char *id2) {
	assert(e->currentWindow);
	EasyEditorState *w = e->currentWindow;

	w->at.y += EASY_EDITOR_MARGIN;

	V4 color = COLOR_WHITE;

	Rect2f spriteTotalBounds = {};

	EasyEditorState *state = easyEditor_addDropDownSpriteList(e, lineNumber, fileName, 0, id1, id2);

	assert(optionLength > 0);

	if(state->dropDownIndex >= optionLength) {
		//NOTE(ollie): Should only get in here if the user changes the list between frames. 
		//			   And the one they had selected doesn't exist anymore
		state->dropDownIndex = 0;
	}



	{

		///////////////////////************ Draw the name of the list*************////////////////////
		Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
		w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;

		float handleWidth = 30;
		float handleHeight = 50;

		//all textures have width 50 then the height will change based on the aspect ratio of the texture
		float textureWidth = 80;	

		float clippedHeight = 0.3f*e->viewportDim.y - handleHeight;
			
		float startY = w->at.y;

		spriteTotalBounds.minY = startY - EASY_EDITOR_MARGIN;
		spriteTotalBounds.minX = w->at.x;

		{
			//This is so each 'channel' has a seperate y descending down
			float yAt_low[3] = { w->at.y, w->at.y, w->at.y };

			///////////////////////************* Find the total height of the scroll window ************////////////////////
			for(int optionIndex = 0; optionIndex < optionLength; ++optionIndex) {
				Texture *option = options[optionIndex];

				int modVal = optionIndex % 3; //three for three channels

				yAt_low[modVal] += textureWidth*option->aspectRatio_h_over_w + EASY_EDITOR_MARGIN;
			}
			////////////////////////////////////////////////////////////////////

			float maxY = 0;

			for(int i = 0; i < arrayCount(yAt_low); ++i) {
				 if(yAt_low[i] > maxY) {
				 	maxY = yAt_low[i];
				 }
			}

			w->at.y = maxY;
		}



		//NOTE(ollie): Find the height of the window
		float acutalHeight = w->at.y - startY;

		//NOTE(ollie): Reset the y height
		w->at.y = startY; 

		bool neededScroll = acutalHeight > clippedHeight;
		float maxX = 0;

		///////////////////////************ Needed scroll *************////////////////////
		if(neededScroll) {
			V2 handlePos = v2(w->at.x - handleWidth - EASY_EDITOR_MARGIN, (startY) + (clippedHeight - handleHeight)*state->scrollAt);

			Rect2f scrollHandle = rect2fMinDim(handlePos.x, handlePos.y, handleWidth, handleHeight);
			
			V4 handleColor = COLOR_GOLD;

			if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), scrollHandle, BOUNDS_RECT)) {
				handleColor = COLOR_YELLOW;
				 if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					//NOTE(ollie): grab the handle
					state->holdingScrollBar = true;									 	
				 }
				
			}

			if(state->holdingScrollBar) {
				handleColor = COLOR_GREEN;

				///////////////////////************ Update the handle position *************////////////////////
				float goalPos = easyInput_mouseToResolution(e->keyStates, e->fuaxResolution).y - startY;
				//update position
				state->scrollAt = lerp(clippedHeight*state->scrollAt, 0.4f, goalPos) / (clippedHeight - handleHeight);
				state->scrollAt = clamp01(state->scrollAt);

				////////////////////////////////////////////////////////////////////

				//NOTE(ollie): Let go of the handle
				if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					state->holdingScrollBar = false;
				}
			}
			//NOTE(ollie): Reclculate the scroll handle position
			scrollHandle = rect2fMinDim(handlePos.x, handlePos.y, handleWidth, handleHeight);

			//NOTE(ollie): Draw the scroll handle
			renderDrawRect(scrollHandle, NEAR_CLIP_PLANE, handleColor, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);
		}
		////////////////////////////////////////////////////////////////////

		//NOTE(ollie): Push the scissors on the stack
		float scrollOffset = 0;

		Rect2f scrollWindow = rect2fMinDim(w->at.x, startY - EASY_EDITOR_MARGIN, e->fuaxResolution.x, clippedHeight);

		if(neededScroll) {
			easyRender_pushScissors(globalRenderGroup, scrollWindow, NEAR_CLIP_PLANE, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix, e->viewportDim);
			scrollOffset = state->scrollAt*(acutalHeight - clippedHeight);
		}

		//This is so each 'channel' has a seperate y descending down
		float yAt_low[3] = { w->at.y - scrollOffset, w->at.y - scrollOffset, w->at.y - scrollOffset};
		
		///////////////////////************ Draw & Update the list *************////////////////////
		for(int optionIndex = 0; optionIndex < optionLength; ++optionIndex) {
			Texture *option = options[optionIndex];

			int modVal = optionIndex % 3; 

			float yAt = yAt_low[modVal];
			color = COLOR_WHITE;

			float xAt = w->at.x + modVal*textureWidth + modVal*EASY_EDITOR_MARGIN;
			float textureHeight = textureWidth*option->aspectRatio_h_over_w;

			bool shouldDraw = (!neededScroll || ((yAt + textureHeight) >= (startY - textureHeight) && (yAt + textureHeight) < (startY + clippedHeight + textureHeight)));

			Rect2f bounds = rect2fMinDim(xAt, yAt, textureWidth, textureHeight);
			bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN));


			if(shouldDraw) {
				
				bool hover = inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT);
				if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, optionIndex + 1)) { //+ 1 because 0 is taken by the box that can open to display the list 
					e->interactingWith.visitedThisFrame = true;
					color = COLOR_GREEN;

					if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
						if(hover) {
							//actually comiting to this option
							state->dropDownIndex = optionIndex;
							state->isOpen = false;
						} else {
							easyEditor_stopInteracting(e);	
						}
					}
				} else if(hover) {
					if(!e->interactingWith.item) {
						color = COLOR_YELLOW;	
					}
					
					if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
						if(e->interactingWith.item) {
							//NOTE: Clicking on a different cell will swap it
							easyEditor_stopInteracting(e);	
						}
						easyEditor_startInteracting(e, &e->bogusItem, lineNumber, fileName, optionIndex + 1, EASY_EDITOR_INTERACT_DROP_DOWN_LIST);
					} 
				}
				
			}
			if(shouldDraw) {
				// renderDrawRect(bounds, 2, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

				renderTextureCentreDim(option, v3(xAt + 0.5f*textureWidth, yAt + 0.5f*textureHeight, 0.3f), v2(textureWidth, textureHeight), COLOR_WHITE, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y),  mat4(),  e->orthoMatrix);
				renderDrawRectOutlineCenterDim_(v3(xAt + 0.5f*textureWidth, yAt + 0.5f*textureHeight, 0.3f), v2(textureWidth, textureHeight), color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y),   e->orthoMatrix, 5);

			}

			yAt_low[modVal] += textureHeight + EASY_EDITOR_MARGIN;	
			
			if(w->maxX < xAt) {
				w->maxX = xAt;
			}

			float mX = (xAt + textureWidth + EASY_EDITOR_MARGIN);
			if(mX > maxX) {
				maxX = mX;
			}
		}

		float maxY = 0;

		for(int i = 0; i < arrayCount(yAt_low); ++i) {
			 if(yAt_low[i] > maxY) {
			 	maxY = yAt_low[i];
			 }
		}

		// renderDrawRectCenterDim(v3(, 0.4f), v2(textureWidth, textureHeight), COLOR_GREY, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y),   e->orthoMatrix);

		w->at.y = maxY + EASY_EDITOR_MARGIN + e->font->fontHeight;

		float yValue = w->at.y;
		///////////////////////************ Pop the scissors*************////////////////////
		if(neededScroll) {
			easyRender_disableScissors(globalRenderGroup);
		
			w->at.y = startY + clippedHeight + EASY_EDITOR_MARGIN + e->font->fontHeight;
			
			yValue = scrollWindow.maxY; 
		}

		spriteTotalBounds.maxX = maxX;
		spriteTotalBounds.maxY = yValue;
		

		renderDrawRect(spriteTotalBounds, 0.5f, COLOR_GREY, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

		if(neededScroll) {
			if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), spriteTotalBounds, BOUNDS_RECT)) {
				state->scrollAt = clamp01(state->scrollAt + -0.02f*e->keyStates->scrollWheelY);
			}
		}

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}

		w->at.x = w->topCorner.x;
	}

	return state->dropDownIndex;

}

//////////////////////////////////////////////////


#define easyEditor_pushCheckBox(e, name, value) easyEditor_pushCheckBox_(e, name, value, __LINE__, __FILE__)
static inline bool easyEditor_pushCheckBox_(EasyEditor *e, char *name, bool *value, int lineNumber, char *fileName) {
	assert(e->currentWindow);
	bool result = false;

	EasyEditorState *w = e->currentWindow;

	if(w->isOpen) {

		w->at.y += EASY_EDITOR_MARGIN;

		//Draw the checkbox
		Rect2f bounds = rect2fMinDim(w->at.x + EASY_EDITOR_MARGIN, w->at.y - 25 - EASY_EDITOR_MARGIN, 50, 50);
		
		////// handle the user interaction

		V4 color = COLOR_WHITE;

		bool hover = inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT);
		if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0)) {
			e->interactingWith.visitedThisFrame = true;
			color = COLOR_GREEN;

			if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				if(hover) {
					*value = !(*value);
				}
				
				easyEditor_stopInteracting(e);	
			}
		} else if(hover) {
			if(!e->interactingWith.item) {
				color = COLOR_YELLOW;	
			}
			
			if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				if(e->interactingWith.item) {
					//NOTE: Clicking on a different cell will swap it
					easyEditor_stopInteracting(e);	
				}
				easyEditor_startInteracting(e, &e->bogusItem, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_CHECK_BOX);
			} 
		}

		renderDrawRect(bounds, 2, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

		if(*value) {
			Texture *t = findTextureAsset("easy_engine_tick.png");
			V2 pos_ = getCenter(bounds);
			V3 pos = v3(pos_.x, pos_.y, 0.3f); 
			renderTextureCentreDim(t, pos, getDim(bounds), COLOR_WHITE, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix, mat4());
		} 

		//NOTE(ollie): Increment the bounds 
		w->at.x = bounds.minX + getDim(bounds).x + EASY_EDITOR_MARGIN;



		///////////////////////////////////

		

		bounds = outputTextNoBacking(e->font, w->at.x + EASY_EDITOR_MARGIN, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
		bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN));
		// bounds.minX = w->topCorner.x + EASY_EDITOR_MARGIN;
		
		//NOTE(ollie): Increment the bounds 
		w->at.x += getDim(bounds).x;

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}
		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
	}

	return result;

}


//////////////////////////////////////////////////


#define easyEditor_pushButton(e, name) easyEditor_pushButton_(e, name, __LINE__, __FILE__)
static inline bool easyEditor_pushButton_(EasyEditor *e, char *name, int lineNumber, char *fileName) {
	assert(e->currentWindow);
	bool result = false;

	EasyEditorState *w = e->currentWindow;

	if(w->isOpen) {

		w->at.y += EASY_EDITOR_MARGIN;

		V4 color = COLOR_WHITE;

		Rect2f bounds = outputTextNoBacking(e->font, w->at.x + EASY_EDITOR_MARGIN, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_BLACK, 1, true, e->screenRelSize);
		bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN));
		// bounds.minX = w->topCorner.x + EASY_EDITOR_MARGIN;
		
		//NOTE(ollie): Increment the bounds 
		w->at.x += getDim(bounds).x;


		bool hover = inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT);
		if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0)) {
			e->interactingWith.visitedThisFrame = true;
			color = COLOR_GREEN;

			if(wasReleased(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				if(hover) {
					result = true;
				}
				
				easyEditor_stopInteracting(e);	
			}
		} else if(hover) {
			if(!e->interactingWith.item) {
				color = COLOR_YELLOW;	
			}
			
			if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				if(e->interactingWith.item) {
					//NOTE: Clicking on a different cell will swap it
					easyEditor_stopInteracting(e);	
				}
				easyEditor_startInteracting(e, &e->bogusItem, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_BUTTON);
			} 
		}

		renderDrawRect(bounds, 2, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}
		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
	}

	return result;

}

typedef enum {
	EASY_EDITOR_FLOAT_BOX,
	EASY_EDITOR_INT_BOX,
	EASY_EDITOR_TEXT_BOX,
} EasyEditor_TextBoxType;

typedef struct {
	Rect2f bounds;
	char *string;
} EasyEditor_TextBoxEditInfo;

static inline EasyEditor_TextBoxEditInfo easyEditor_renderBoxWithCharacters_(EasyEditor *e, float *f, int *intVal, char *startStr, int lineNumber, char *fileName, int index, EasyEditor_TextBoxType boxType, int id1 = 0, char *id2 = 0) {
	EasyEditor_TextBoxEditInfo result = {};

	assert(e->currentWindow);
	EasyEditorState *w = e->currentWindow;

	if(w->isOpen) {

		V4 color = COLOR_WHITE;

		EasyEditorState *hasState = easyEditor_hasState(e, lineNumber, fileName, index, EASY_EDITOR_INTERACT_TEXT_FIELD, id1, id2);
		EasyEditorState *field = easyEditor_addTextField(e, lineNumber, fileName, index, id1, id2);

		if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, index)) {
			e->interactingWith.visitedThisFrame = true;
			assert(field == (EasyEditorState *)e->interactingWith.item);
			if(wasPressed(e->keyStates->gameButtons, BUTTON_ENTER)) {
				if(boxType == EASY_EDITOR_FLOAT_BOX) {
					float newValue = atof(field->buffer.chars);
					*f = newValue;	
				} 

				if(boxType == EASY_EDITOR_INT_BOX) {
					float newValue = atoi(field->buffer.chars);
					*intVal = newValue;	
				} 

				easyEditor_stopInteracting(e);
			} else {
				color = COLOR_GREEN;

				//NOTE: update buffer input
				if(e->keyStates->inputString) {
					splice(&field->buffer, e->keyStates->inputString, true);
				}

				if(wasPressed(e->keyStates->gameButtons, BUTTON_BACKSPACE)) {
					splice(&field->buffer, "1", false);
				}

				if(wasPressed(e->keyStates->gameButtons, BUTTON_BOARD_LEFT) && field->buffer.cursorAt > 0) {
					field->buffer.cursorAt--;
				}
				if(wasPressed(e->keyStates->gameButtons, BUTTON_BOARD_RIGHT) && field->buffer.cursorAt < field->buffer.length) {
					field->buffer.cursorAt++;
				}
				////
			}
		} else {
			//Where we first put the string in the buffer 
			if(boxType == EASY_EDITOR_FLOAT_BOX) {
				//Empty the buffer
				easyString_emptyInputBuffer(&field->buffer);

				char floatStr[512];
				sprintf(floatStr, "%f", *f);

				splice(&field->buffer, floatStr, true);

			} else if(boxType == EASY_EDITOR_INT_BOX) {
				//Empty the buffer
				easyString_emptyInputBuffer(&field->buffer);

				char floatStr[512];
				sprintf(floatStr, "%d", *intVal);

				splice(&field->buffer, floatStr, true);
			} else if(boxType == EASY_EDITOR_TEXT_BOX && !hasState) {
				splice(&field->buffer, startStr, true);
			}
		} 

		char *string = field->buffer.chars;
		bool displayString = true;
		if(strlen(string) == 0) {
			string = "Null";
			displayString = false;
		}

		result.string = string;

		Rect2f bounds = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, string, w->margin, COLOR_BLACK, 1, displayString, e->screenRelSize);

		float fontHeight = e->font->fontHeight;

		if(getDim(bounds).x < EASY_EDITOR_MIN_TEXT_FIELD_WIDTH) {
			bounds.maxX = bounds.minX + EASY_EDITOR_MIN_TEXT_FIELD_WIDTH;
		}
		
		if(getDim(bounds).y < fontHeight) {
			bounds.maxY = bounds.minY + fontHeight;
		}


		bounds = expandRectf(bounds, v2(EASY_EDITOR_MARGIN, EASY_EDITOR_MARGIN)); 
		if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT)) {
			if(!e->interactingWith.item) {
				color = COLOR_YELLOW;	
			}
			
			if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				if(e->interactingWith.item) {
					//NOTE: Clicking on a different cell will swap it
					easyEditor_stopInteracting(e);	
				}
				easyEditor_startInteracting(e, field, lineNumber, fileName, index, EASY_EDITOR_INTERACT_TEXT_FIELD);
			} 
		}

		renderDrawRect(bounds, 2.0f, color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

		result.bounds = bounds;
	}

	return result;
}


#define easyEditor_preloadName(e, str) easyEditor_preloadName_(e, str, __LINE__, __FILE__, __LINE__, __FILE__)
static inline void easyEditor_preloadName_(EasyEditor *e, char *str, int lineNumber, char *fileName, int id1, char *id2) {

	if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0, id1, id2)) {

	} else {
		EasyEditorState *hasState = easyEditor_hasState(e, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_TEXT_FIELD);
		if(hasState) {
			EasyEditorState *field = easyEditor_addTextField(e, lineNumber, fileName, 0, id1, id2);
			field->buffer.length = 0;
			field->buffer.cursorAt = 0;
			splice(&field->buffer, str, true);

		}
	}
}

#define easyEditor_pushTextBox(e, name, startStr) easyEditor_pushTextBox_(e, name, startStr, __LINE__, __FILE__, __LINE__, __FILE__)
#define easyEditor_pushTextBox_withIds(e, name, startStr, id1, id2) easyEditor_pushTextBox_(e, name, startStr, __LINE__, __FILE__, id1, id2)
static inline char *easyEditor_pushTextBox_(EasyEditor *e, char *name, char *startStr, int lineNumber, char *fileName, int id1, char *id2) {
	EasyEditorState *w = e->currentWindow;
	char *result = "empty";
	assert(w);
	if(w->isOpen) {
		if(name && strlen(name) > 0) {
			Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		EasyEditor_TextBoxEditInfo textInfo = easyEditor_renderBoxWithCharacters_(e, 0, 0, startStr, lineNumber, fileName, 0, EASY_EDITOR_TEXT_BOX, id1, id2);
		Rect2f bounds = textInfo.bounds;
		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;


		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}
		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;

		result = textInfo.string;
	}
	return result;
}

#define easyEditor_pushInt1(e, name, f0) easyEditor_pushInt1_(e, name, f0, __LINE__, __FILE__)
static inline void easyEditor_pushInt1_(EasyEditor *e, char *name, int *f, int lineNumber, char *fileName) {
	EasyEditorState *w = e->currentWindow;
	assert(w);

	if(w->isOpen) {
		if(name && strlen(name) > 0) {
			Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		Rect2f bounds = easyEditor_renderBoxWithCharacters_(e, 0, f, 0, lineNumber, fileName, 0, EASY_EDITOR_INT_BOX).bounds;
		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}
		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
	}

}

#define easyEditor_pushFloat1(e, name, f0) easyEditor_pushFloat1_(e, name, f0, __LINE__, __FILE__)
static inline void easyEditor_pushFloat1_(EasyEditor *e, char *name, float *f, int lineNumber, char *fileName) {
	EasyEditorState *w = e->currentWindow;
	assert(w);
	if(w->isOpen) {
		if(name && strlen(name) > 0) {
			Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		Rect2f bounds = easyEditor_renderBoxWithCharacters_(e, f, 0, 0, lineNumber, fileName, 0, EASY_EDITOR_FLOAT_BOX).bounds;
		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}
		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
	}
}

#define easyEditor_pushFloat2(e, name, f0, f1) easyEditor_pushFloat2_(e, name, f0, f1, __LINE__, __FILE__)
static inline void easyEditor_pushFloat2_(EasyEditor *e, char *name, float *f0, float *f1, int lineNumber, char *fileName) {
	EasyEditorState *w = e->currentWindow;
	assert(w);
	if(w->isOpen) {

		if(name && strlen(name) > 0) {
			Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		Rect2f bounds = easyEditor_renderBoxWithCharacters_(e, f0, 0, 0, lineNumber, fileName, 0, EASY_EDITOR_FLOAT_BOX).bounds;

		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;
		bounds = easyEditor_renderBoxWithCharacters_(e, f1, 0, 0, lineNumber, fileName, 1, EASY_EDITOR_FLOAT_BOX).bounds;
		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}
		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
	}
}


#define easyEditor_pushFloat3(e, name, f0, f1, f2) easyEditor_pushFloat3_(e, name, f0, f1, f2, __LINE__, __FILE__)
static inline void easyEditor_pushFloat3_(EasyEditor *e, char *name, float *f0, float *f1, float *f2, int lineNumber, char *fileName) {
	EasyEditorState *w = e->currentWindow;
	assert(w);
	if(w->isOpen) {
		if(name && strlen(name) > 0) {
			Rect2f rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		Rect2f bounds = easyEditor_renderBoxWithCharacters_(e, f0, 0, 0, lineNumber, fileName, 0, EASY_EDITOR_FLOAT_BOX).bounds;

		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;
		bounds = easyEditor_renderBoxWithCharacters_(e, f1, 0, 0, lineNumber, fileName, 1, EASY_EDITOR_FLOAT_BOX).bounds;
		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;
		bounds = easyEditor_renderBoxWithCharacters_(e, f2, 0, 0, lineNumber, fileName, 2, EASY_EDITOR_FLOAT_BOX).bounds;
		w->at.x += getDim(bounds).x + EASY_EDITOR_MARGIN;

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}

		w->at.x = w->topCorner.x;
		w->at.y += getDim(bounds).y + EASY_EDITOR_MARGIN;
	}
}

#define easyEditor_pushBool(e, name, b0) easyEditor_pushBool_(e, name, b0, __LINE__, __FILE__)
static inline void easyEditor_pushBool_(EasyEditor *e, char *name, bool *b, int lineNumber, char *fileName) {


	
}

static inline void easyEditor_endEditorForFrame(EasyEditor *e) {
	if(!e->interactingWith.visitedThisFrame) {
		easyEditor_stopInteracting(e);
	}

	e->isHovering = e->isHovering_unstable;
	//NOTE(ollie): Clear the unstable one to false
	e->isHovering_unstable = false;

	e->interactingWith.visitedThisFrame = false;
	
}

#define easyEditor_pushSlider(e, name, value, min, max) easyEditor_pushSlider_(e, name, value, min, max, __LINE__, __FILE__, 0)
static inline void easyEditor_pushSlider_(EasyEditor *e, char *name, float *value, float min, float max, int lineNumber, char *fileName, int index) {
	EasyEditorState *w = e->currentWindow;
	assert(w);
	if(w->isOpen) {

	///////////////////////*************Add the slider state************////////////////////
		EasyEditorState *field = easyEditor_addSlider(e, lineNumber, fileName, index, min, max);

		Rect2f rect = rect2f(0, 0, 0, e->font->fontHeight);
		if(name && strlen(name) > 0) {
			rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		float sliderX = 300;
		float sliderY = 20;
		Rect2f sliderRect = rect2fMinDim(w->at.x, w->at.y - sliderY, sliderX, sliderY);

		float greyValue = 0.2f;
		renderDrawRect(sliderRect, 2, v4(greyValue, greyValue, greyValue, 1.0f), 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);


		float handleX = 40;
		float handleY = 40;

		float percent = ((*value - min) / (max - min));
		if(percent < 0.0f)  percent = 0; 
		if(percent > 1.0f)  percent = 1; 

		float offset = w->at.x + sliderX*percent;
		
		Rect2f handleRect = rect2fCenterDim(offset, w->at.y - 0.5f*sliderY, handleX, handleY);

		V4 handleColor = COLOR_WHITE;

	///////////////////////***********check if were interacting with it**************////////////////////
		if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, index)) {
			e->interactingWith.visitedThisFrame = true;

			if(!isDown(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				easyEditor_stopInteracting(e);
			} else {
				float tVal = (easyInput_mouseToResolution(e->keyStates, e->fuaxResolution).x - w->at.x) / sliderX;
				*value = clamp(min, lerp(min, tVal, max), max);
			}

			handleColor = COLOR_GREY;
		} else {
			if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), handleRect, BOUNDS_RECT)) {
				if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					if(e->interactingWith.item) {
						easyEditor_stopInteracting(e);	
					}
					easyEditor_startInteracting(e, field, lineNumber, fileName, index, EASY_EDITOR_INTERACT_SLIDER);
				} 
				handleColor = COLOR_YELLOW;
			}
		}

		
		
		V3 pos = {};
		pos.xy = getCenter(handleRect);
		pos.z = 1;
		renderTextureCentreDim(findTextureAsset("easy_engine_blank_circle.png"), pos, getDim(handleRect), handleColor, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix, mat4());

	///////////////////////*************advance the cursor ************////////////////////

		w->at.x += sliderX + EASY_EDITOR_MARGIN;


		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}

		w->at.x = w->topCorner.x;
		w->at.y += getDim(rect).y + EASY_EDITOR_MARGIN;
	}

}

#define easyEditor_pushColor(e, name, color) easyEditor_pushColor_(e, name, color, __LINE__, __FILE__)
static inline void easyEditor_pushColor_(EasyEditor *e, char *name, V4 *color, int lineNumber, char *fileName) {
	EasyEditorState *w = e->currentWindow;
	assert(w);

	if(w->isOpen) {

		//NOTE(ol): Take snapshot of state
		BlendFuncType tempType = globalRenderGroup->blendFuncType;
		//

		EasyEditorState *field = easyEditor_addColorSelector(e, *color, lineNumber, fileName, 0);

		Rect2f rect = rect2f(0, 0, 30, 10);
		if(name && strlen(name) > 0) {
			rect = outputTextNoBacking(e->font, w->at.x, w->at.y, 1, e->fuaxResolution, name, w->margin, COLOR_WHITE, 1, true, e->screenRelSize);
			w->at.x += getDim(rect).x + 3*EASY_EDITOR_MARGIN;
		}

		V2 at = v2_minus(w->at, v2(0, getDim(rect).y));
		Rect2f bounds = rect2fMinDimV2(at, getDim(rect));

		//change to other type of blend
		setBlendFuncType(globalRenderGroup, BLEND_FUNC_STANDARD_NO_PREMULTIPLED_ALPHA);

		renderDrawRect(bounds, 2, *color, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

		//restore the state
		setBlendFuncType(globalRenderGroup, tempType);
		//
		w->at.x += getDim(bounds).x;

		w->at.y += getDim(bounds).y;


		float advanceY = 0;
		if(field->isOpen) {

			if(wasPressed(e->keyStates->gameButtons, BUTTON_BACKSPACE)) {
				if(e->interactingWith.item) {
					easyEditor_stopInteracting(e);	
				}
				field->coord = v2(0, 0);
				*color = COLOR_WHITE;
			}
	/////////////************ DRAW THE COLOR WHEEL ***********///////////////
			V2 dim = v2(150, 150);
			float radius = 0.5f*dim.x;
			V3 center = v3(w->at.x + 0.5f*dim.x, w->at.y + 0.5f*dim.y, 1);

			advanceY = dim.y + e->font->fontHeight;
	///////////////////////////////////////////////////////////////////////////

			Rect2f colorSelectorBounds = rect2fCenterDimV2(v2_plus(v2_scale(radius, field->coord), center.xy), v2(15, 15));
			V4 selectorColor = COLOR_BLACK;

			///// IF INTERACTING WITH THE COLOR SELECTOR
			if(e->interactingWith.item && easyEditor_idEqual(e->interactingWith.id, lineNumber, fileName, 0)) {
				e->interactingWith.visitedThisFrame = true;
				assert(field == e->interactingWith.item);

				if(!isDown(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
					easyEditor_stopInteracting(e);
				}

				selectorColor = *color;

	///////////////////////***********SET POSITION OF SELECTOR**************////////////////////

				field->coord = v2_minus(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), center.xy);
				float length = getLength(field->coord) / radius;

				if(length > 1) { length = 1; }

				field->coord = v2_scale(length, normalizeV2(field->coord));
				
			} else {
				//NOTE: Check whether we are hovering over the button
				if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), colorSelectorBounds, BOUNDS_RECT)) {
					selectorColor = COLOR_YELLOW;	
					
					if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
						if(e->interactingWith.item) {
							easyEditor_stopInteracting(e);	
						}
						easyEditor_startInteracting(e, field, lineNumber, fileName, 0, EASY_EDITOR_INTERACT_COLOR_WHEEL);
					} 
				}
			}

			//change to other type of blend
			setBlendFuncType(globalRenderGroup, BLEND_FUNC_STANDARD_NO_PREMULTIPLED_ALPHA);
			//

			EasyRender_ColorWheel_DataPacket *packet = pushStruct(&globalPerFrameArena, EasyRender_ColorWheel_DataPacket);
			
			packet->value = field->value;

			renderColorWheel(center, dim, packet, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix, mat4());
			
			//restore the state
			setBlendFuncType(globalRenderGroup, tempType);
			//
			renderDrawRect(colorSelectorBounds, 1, selectorColor, 0, mat4TopLeftToBottomLeft(e->fuaxResolution.y), e->orthoMatrix);

			w->at.y += advanceY + EASY_EDITOR_MARGIN;

			easyEditor_pushSlider_(e, "", &field->value, 0, 1, lineNumber, fileName, 1);

	///////////////////////*************WORK OUT THE NEW COLOR & SET IT ************////////////////////
			float hue = 0;
			float saturation = getLength(field->coord);
			if(field->coord.x != 0.0f) {
				float flippedY = field->coord.y;
				float flippedX = -1*field->coord.x;


				hue = atan2(flippedY, flippedX);
				//assert(hue >= -PI32 && hue <= PI32);
				hue /= PI32;
				assert(hue >= -1.0f && hue <= 1.0f);
				hue += 1.0f;
				hue /= 2.0f;
				assert(hue >= 0.0f && hue <= 1.0f);
				hue *= 360.0f; 
				assert(hue >= 0 && hue <= 360);
			}

			*color = easyColor_hsvToRgb(hue, saturation, field->value);

		} else {
			w->at.y += advanceY + EASY_EDITOR_MARGIN;
		}


		if(inBounds(easyInput_mouseToResolution(e->keyStates, e->fuaxResolution), bounds, BOUNDS_RECT)) {
			if(wasPressed(e->keyStates->gameButtons, BUTTON_LEFT_MOUSE)) {
				field->isOpen = !field->isOpen;
			} 
		}

		

		if(w->maxX < w->at.x) {
			w->maxX = w->at.x;
		}

		w->at.x = w->topCorner.x;
	}

	
}


