#ifndef _WIN32_HANDMADE_
#define _WIN32_HANDMADE_
#include "build_opts.h"

#define internal static
#define global   static

typedef struct _WIN32OffscreenBuffer
{
    BITMAPINFO  info;
    void        *memory;
    int         width;
    int         height;
    int         bytesPerPixel;
    int         pitch;
} WIN32OffscreenBuffer;

typedef struct _WIN32Dims
{
    int width;
    int height;
} WIN32Dims;


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


#endif /* _WIN32_HANDMADE_ */