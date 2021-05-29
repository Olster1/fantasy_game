typedef struct {
    GameButton buttons[BUTTON_COUNT];
    float xAxis;
    float yAxis;    
} EasyGameController;

#define EASY_CONTROLLER_MAX_CONTROLLERS 4

typedef struct {
    SDL_GameController *controllerHandles[EASY_CONTROLLER_MAX_CONTROLLERS];
    EasyGameController controllerButtons[EASY_CONTROLLER_MAX_CONTROLLERS];
} EasyGameControllerState;


static EasyGameControllerState *easyControllers_initState() {
    EasyGameControllerState *state = pushStruct(&globalLongTermArena, EasyGameControllerState);


    int maxJoysticks = SDL_NumJoysticks();
    int controllerIndex = 0;

    for(int JoystickIndex = 0; JoystickIndex < maxJoysticks; ++JoystickIndex)
    {
        if (!SDL_IsGameController(JoystickIndex)) {
            continue;
        }
        if (controllerIndex >= EASY_CONTROLLER_MAX_CONTROLLERS)
        {
            break;
        }
        state->controllerHandles[controllerIndex] = SDL_GameControllerOpen(JoystickIndex);
        controllerIndex++;
    }

    return state;
}

static void easyControllers_closeControllers(EasyGameControllerState *state) {
    for(int ControllerIndex = 0; ControllerIndex < EASY_CONTROLLER_MAX_CONTROLLERS; ++ControllerIndex) {
       if (state->controllerHandles[ControllerIndex]) {
           SDL_GameControllerClose(state->controllerHandles[ControllerIndex]);
       }
    }
}

static bool easyControllers_pollControllers(EasyGameControllerState *state, AppKeyStates *keyStates) { //returns controllers available
    bool controllerConnected = false;
    int availableControllers = 0;
    for (int ControllerIndex = 0;
         ControllerIndex < EASY_CONTROLLER_MAX_CONTROLLERS;
         ++ControllerIndex)
    {
        SDL_GameController *sdl_controller  = state->controllerHandles[ControllerIndex];

        EasyGameController *controller = &state->controllerButtons[ControllerIndex];

        //clear the transition count
        for(int i = 0; i < BUTTON_COUNT; i++) {
            controller->buttons[i].transitionCount = 0;
        }

        if(sdl_controller != 0 && SDL_GameControllerGetAttached(sdl_controller))
        {
            
            bool Up = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            bool Down = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            bool Left = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            bool Right = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            bool Start = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_START);
            bool Back = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_BACK);
            bool LeftShoulder = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            bool RightShoulder = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            bool AButton = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_A);
            bool BButton = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_B);
            bool XButton = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_X);
            bool YButton = SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_Y);

            short StickX = SDL_GameControllerGetAxis(sdl_controller, SDL_CONTROLLER_AXIS_LEFTX);
            short StickY = SDL_GameControllerGetAxis(sdl_controller, SDL_CONTROLLER_AXIS_LEFTY);

            float xAxis = inverse_lerp(-32768, StickX, 32767);
            float yAxis = inverse_lerp(-32768, StickY, 32767);

            sdlProcessGameKey(&controller->buttons[BUTTON_LEFT], Left, isDown(controller->buttons, BUTTON_LEFT) && Left);
            sdlProcessGameKey(&controller->buttons[BUTTON_RIGHT], Right, isDown(controller->buttons, BUTTON_RIGHT) && Right);
            sdlProcessGameKey(&controller->buttons[BUTTON_UP], Up, isDown(controller->buttons, BUTTON_UP) && Up);
            sdlProcessGameKey(&controller->buttons[BUTTON_DOWN], Down, isDown(controller->buttons, BUTTON_DOWN) && Down);
            sdlProcessGameKey(&controller->buttons[BUTTON_SPACE], AButton, isDown(controller->buttons, BUTTON_SPACE) && AButton);
            sdlProcessGameKey(&controller->buttons[BUTTON_SHIFT], LeftShoulder, isDown(controller->buttons, BUTTON_SHIFT) && LeftShoulder);
            sdlProcessGameKey(&controller->buttons[BUTTON_I], Start, isDown(controller->buttons, BUTTON_I) && Start);
            sdlProcessGameKey(&controller->buttons[BUTTON_ENTER], BButton, isDown(controller->buttons, BUTTON_ENTER) && BButton);
            sdlProcessGameKey(&controller->buttons[BUTTON_ESCAPE], Back, isDown(controller->buttons, BUTTON_ESCAPE) && Back);
            sdlProcessGameKey(&controller->buttons[BUTTON_X], XButton, isDown(controller->buttons, BUTTON_X) && XButton);
            sdlProcessGameKey(&controller->buttons[BUTTON_Z], YButton, isDown(controller->buttons, BUTTON_Z) && YButton);

            controller->xAxis = xAxis;
            controller->yAxis = yAxis;

            availableControllers++;

            controllerConnected = true;

            if(ControllerIndex == 0) { //player 1 controller
                keyStates->gameButtons[BUTTON_LEFT] = controller->buttons[BUTTON_LEFT];
                keyStates->gameButtons[BUTTON_RIGHT] = controller->buttons[BUTTON_RIGHT];
                keyStates->gameButtons[BUTTON_UP] = controller->buttons[BUTTON_UP];
                keyStates->gameButtons[BUTTON_DOWN] = controller->buttons[BUTTON_DOWN];
                keyStates->gameButtons[BUTTON_SPACE] = controller->buttons[BUTTON_SPACE];
                keyStates->gameButtons[BUTTON_X] = controller->buttons[BUTTON_X];
                keyStates->gameButtons[BUTTON_Z] = controller->buttons[BUTTON_Z];
                keyStates->gameButtons[BUTTON_ENTER] = controller->buttons[BUTTON_ENTER];
                keyStates->gameButtons[BUTTON_SHIFT] = controller->buttons[BUTTON_SHIFT];
                keyStates->gameButtons[BUTTON_I] = controller->buttons[BUTTON_I];

                // keyStates->joystick = v2(1, 1);

            }
        }
        else
        {
            // TODO: This controller is note plugged in.
        }
    }
    return controllerConnected;
}