#ifndef EASY_ANIMATION_2D_H
#define EASY_ANIMATION_2D_H


#ifndef EASY_ANIMATION_2D_IMPLEMENTATION
#define EASY_ANIMATION_2D_IMPLEMENTATION 0
#endif

#ifndef EASY_HEADERS_ASSERT
#define EASY_HEADERS_ASSERT(statement) if(!(statement)) { int *i_ptr = 0; *(i_ptr) = 0; }
#endif

#ifndef EASY_HEADERS_ALLOC
#include <stdlib.h>
#define EASY_HEADERS_ALLOC(size) malloc(size)
#endif

#ifndef EASY_HEADERS_FREE
#include <stdlib.h>
#define EASY_HEADERS_FREE(ptr) free(ptr)
#endif

typedef struct {
    char *frames[256];
    int frameCount;
    char *name;
}  Animation;

typedef struct EasyAnimation_ListItem EasyAnimation_ListItem;
typedef struct EasyAnimation_ListItem {
    float timerAt;
    float timerPeriod;

    int frameIndex;
    
    Animation *animation;

    
    
    EasyAnimation_ListItem *prev;
    EasyAnimation_ListItem *next;
} EasyAnimation_ListItem;

typedef struct {
    EasyAnimation_ListItem parent;

    Animation *lastAnimationOn;
    bool finishedAnimationLastUpdate; //goes to true if it finished an animation in the last update so you can change it if you need to
    unsigned int currentLoopCount; //NOTE: How many times the current animation has looped for
} EasyAnimation_Controller;

///////////////////////************ Header definitions start here *************////////////////////

//Constructor functions
void easyAnimation_initController(EasyAnimation_Controller *controller);
void easyAnimation_initAnimation(Animation *animation, char **FileNames, int FileNameCount, char *name);


//Two workhorse functions
void easyAnimation_addAnimationToController(EasyAnimation_Controller *controller, EasyAnimation_ListItem **AnimationItemFreeListPtr, Animation *animation, float period);
char *easyAnimation_updateAnimation(EasyAnimation_Controller *controller, EasyAnimation_ListItem **AnimationItemFreeListPtr, float dt, Animation *NextAnimation, float period);

//Get the animation the controller is currently on
char *easyAnimation_getFrameOn(EasyAnimation_ListItem *AnimationListSentintel);

//Helper functions
//Find an animation in a list
Animation *easyAnimation_findAnimation(Animation *Animations, int AnimationsCount, char *name);

//See if there are any current animations 
int easyAnimation_isControllerEmpty(EasyAnimation_Controller *c);
//Empty the animation controller
void easyAnimation_emptyAnimationContoller(EasyAnimation_Controller *controller, EasyAnimation_ListItem **AnimationItemFreeListPtr);

//Get the direction in radians of velocity. This could be used to find the correct animation
float easyAnimation_getDirectionInRadians(float x, float y);

#if EASY_ANIMATION_2D_IMPLEMENTATION

char *easyAnimation2d_copyString(char *str) {

    int sizeOfString = 0;
    char *at = (char *)str;
    while(*at) {
        sizeOfString++;
        at++;
    }

    char *result = (char *)EASY_HEADERS_ALLOC(sizeOfString + 1);
    result[sizeOfString] = '\0';

    //Now copy the string
    char *src = (char *)str;
    char *dest = (char *)result;
    while(*src) {
        *dest = *src;
        src++;
        dest++;
    }

    return result;
}

void easyAnimation_initController(EasyAnimation_Controller *controller) {
    controller->parent.next = controller->parent.prev = &controller->parent;
    controller->finishedAnimationLastUpdate = false;
    controller->lastAnimationOn = 0;
    controller->currentLoopCount = 0;
}

void easyAnimation_initAnimation_withFrames(Animation *animation, char **FileNames, int FileNameCount, char *name) {
    animation->name = name;
    animation->frameCount = 0;

    for(int i = 0; i < FileNameCount; ++i) {
        EASY_HEADERS_ASSERT(animation->frameCount < arrayCount(animation->frames));
        animation->frames[animation->frameCount++] = easyAnimation2d_copyString(FileNames[i]);
    }
}

void easyAnimation_initAnimation(Animation *animation, char *name) {
    animation->name = name;
    animation->frameCount = 0;
}

void easyAnimation_pushFrame(Animation *animation, char *assetName) {
    EASY_HEADERS_ASSERT(animation->frameCount < arrayCount(animation->frames));
    animation->frames[animation->frameCount++] = easyAnimation2d_copyString(assetName);
}

// static Animation *easyAnimation_findAnimationWithId(Animation *animations, int AnimationsCount, int id) {
//     Animation *Result = 0;
//     for(int i = 0; i < AnimationsCount; i++) {
//         Animation *Anim = animations + i;
//         if(Anim->animationId == id) {
//             Result = Anim;
//             break;
//         }
//     }
    
//     return Result;
// }

Animation *easyAnimation_findAnimation(Animation *Animations, int AnimationsCount, char *name) {
    Animation *Result = 0;
    for(int i = 0; i < AnimationsCount; ++i) {
        Animation *Anim = Animations + i;
        if(easyString_stringsMatch_nullTerminated(Anim->name, name)) {
            Result = Anim;
            break;
        }
    }
    
    return Result;
}


void easyAnimation_addAnimationToController(EasyAnimation_Controller *controller, EasyAnimation_ListItem **AnimationItemFreeListPtr, Animation *animation, float period) {
    EasyAnimation_ListItem *AnimationItemFreeList = *AnimationItemFreeListPtr;
    EasyAnimation_ListItem *Item = 0;
    if(AnimationItemFreeList) {
        Item = AnimationItemFreeList;
        *AnimationItemFreeListPtr = AnimationItemFreeList->next;
    } else {
        Item = (EasyAnimation_ListItem *)EASY_HEADERS_ALLOC(sizeof(EasyAnimation_ListItem));
    }
    
    EASY_HEADERS_ASSERT(Item);
    
    Item->timerAt = 0;
    Item->timerPeriod = period;
    
    Item->frameIndex = 0;
    
    Item->animation = animation;
        
    EasyAnimation_ListItem *AnimationListSentintel = &controller->parent;

    //Add animation to end of list;
    Item->next = AnimationListSentintel;
    Item->prev = AnimationListSentintel->prev;
    
    AnimationListSentintel->prev->next = Item;
    AnimationListSentintel->prev = Item;
    
}

void easyAnimation_emptyAnimationContoller(EasyAnimation_Controller *controller, EasyAnimation_ListItem **AnimationItemFreeListPtr) {
    //NOTE(ollie): While still things on the list
    EasyAnimation_ListItem *AnimationListSentintel = &controller->parent;

    while(AnimationListSentintel->next != AnimationListSentintel) {
        EasyAnimation_ListItem *Item = AnimationListSentintel->next;
        //Remove from linked list
        AnimationListSentintel->next = Item->next;
        Item->next->prev = AnimationListSentintel;
        
        //Add to free list
        Item->next = *AnimationItemFreeListPtr;
        *AnimationItemFreeListPtr = Item;
    }

    controller->currentLoopCount = 0;
    
}

char *easyAnimation_updateAnimation(EasyAnimation_Controller *controller, EasyAnimation_ListItem **AnimationItemFreeListPtr, float dt) {
    EasyAnimation_ListItem *AnimationListSentintel = &controller->parent;

    controller->finishedAnimationLastUpdate = false; //clear out the value

    EasyAnimation_ListItem *Item = AnimationListSentintel->next;
    EASY_HEADERS_ASSERT(Item != AnimationListSentintel);
    
    EASY_HEADERS_ASSERT(Item->timerAt >= 0);

    controller->lastAnimationOn = Item->animation;

    Item->timerAt += dt;    

    char *result = Item->animation->frames[Item->frameIndex];

    if(Item->timerAt >= Item->timerPeriod) {
        Item->frameIndex++;
        Item->timerAt = 0;
        
        if(Item->frameIndex >= Item->animation->frameCount)
        { 
            //finished animation
            Item->frameIndex = 0;

            if(Item->next != AnimationListSentintel) {
                //NOTE: There are more than one animation on the list, so go to the next one
                //Remove from linked list
                AnimationListSentintel->next = Item->next;
                Item->next->prev = AnimationListSentintel;
                
                //Add to free list
                Item->next = *AnimationItemFreeListPtr;
                *AnimationItemFreeListPtr = Item;

                controller->currentLoopCount = 0;
            } else {
                controller->currentLoopCount++;
            }

            controller->finishedAnimationLastUpdate = true;

        }
        
    }

    
    return result;
}

inline int easyAnimation_isControllerEmpty(EasyAnimation_Controller *c) {
    int Result = c->parent.next == &c->parent;
    return Result;
}

inline float easyAnimation_getDirectionInRadians(float x, float y) {
    float DirectionValue = 0;
    if(x != 0 || y != 0) {
        //V2 EntityVelocity = normalizeV2(dp);
        DirectionValue = ATan2_0toTau(y, x);
    }
    return DirectionValue;
}

inline char *easyAnimation_getFrameOn(EasyAnimation_ListItem *AnimationListSentintel) {
    char *currentFrame = AnimationListSentintel->next->animation->frames[AnimationListSentintel->next->frameIndex];
    return currentFrame;
}

inline bool easyAnimation_getCurrentAnimation(EasyAnimation_Controller *c, Animation *a) {
    return (c->parent.next->animation == a);

}

#endif // END OF IMPLEMENTATION
#endif // END OF HEADER INCLUDE


/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

