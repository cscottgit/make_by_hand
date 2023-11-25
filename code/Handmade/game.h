#ifndef _GAME_
#define _GAME_
#include "build_opts.h"

typedef struct _Keyboard_input_state
{
	bool up;
	bool down;
	bool left;
	bool right;
	bool space;
	bool escape;
} Keyboard_input_state;

typedef struct _Gamepad_input_state
{
	bool up;
	bool down;
	bool left;
	bool right;
	bool start;
	bool back;
	bool leftThumb;
	bool rightThumb;
	bool leftShoulder;
	bool rightShoulder;
	bool aButton;
	bool bButton;
	bool xButton;
	bool yButton;

	int16_t stickX;
	int16_t stickY;
} Gamepad_input_state;

typedef struct _Input_state
{
	Keyboard_input_state	keyboard;
	Gamepad_input_state		gamepad;
} Input_state;


#endif /* _GAME_ */