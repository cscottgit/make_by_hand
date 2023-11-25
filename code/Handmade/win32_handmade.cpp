#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#include "win32_handmade.h"
#include "game.h"

//#include <winuser.h>
//#include <iostream>

global bool sRunning = true;

global WIN32OffscreenBuffer globalOffscreenBuffer;
global Input_state globalInputState;


#if defined(LINK_DIRECTLY_WITH_XINPUT_LIB) && (LINK_DIRECTLY_WITH_XINPUT_LIB == FALSE)
typedef DWORD (WINAPI *XInputGetState_T)(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD (WINAPI *XInputSetState_T)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

internal XInputGetState_T localXInputGetState;
internal XInputSetState_T localXInputSetState;

#define XInputGetState localXInputGetState
#define XInputSetState localXInputSetState
#endif

internal void Win32LoadXInput(void)
{
#if defined(LINK_DIRECTLY_WITH_XINPUT_LIB) && (LINK_DIRECTLY_WITH_XINPUT_LIB == FALSE)
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (XInputLibrary)
	{
		localXInputGetState = (XInputGetState_T)GetProcAddress(XInputLibrary, "XInputGetState");
		localXInputSetState = (XInputSetState_T)GetProcAddress(XInputLibrary, "XInputSetState");
	}
#endif
}


internal WIN32Dims Win32GetWindowDims(HWND hwnd)
{
    WIN32Dims dims;
    RECT rect;
    GetClientRect(hwnd, &rect);
    dims.height = rect.bottom - rect.top;
    dims.width = rect.right - rect.left;
    return dims;
}

internal void Win32HandleXInput(Input_state* inputState)
{
	DWORD dwResult;
	for (DWORD controllerIdx = 0;
		controllerIdx < XUSER_MAX_COUNT;
		controllerIdx++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(controllerIdx, &state);

		if (dwResult == ERROR_SUCCESS)
		{
			XINPUT_GAMEPAD* Gamepad = &state.Gamepad;
			//
			//typedef struct _XINPUT_STATE {
			//DWORD          dwPacketNumber;
			//XINPUT_GAMEPAD Gamepad;
			//} XINPUT_STATE, *PXINPUT_STATE;
			Gamepad_input_state* gpInputState = &inputState->gamepad;
			gpInputState->up = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			gpInputState->down = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			gpInputState->left = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			gpInputState->right = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
			gpInputState->start = (Gamepad->wButtons & XINPUT_GAMEPAD_START);
			gpInputState->back = (Gamepad->wButtons & XINPUT_GAMEPAD_BACK);
			gpInputState->leftThumb = (Gamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
			gpInputState->rightThumb = (Gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
			gpInputState->leftShoulder = (Gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			gpInputState->rightShoulder = (Gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			gpInputState->aButton = (Gamepad->wButtons & XINPUT_GAMEPAD_A);
			gpInputState->bButton = (Gamepad->wButtons & XINPUT_GAMEPAD_B);
			gpInputState->xButton = (Gamepad->wButtons & XINPUT_GAMEPAD_X);
			gpInputState->yButton = (Gamepad->wButtons & XINPUT_GAMEPAD_Y);
			gpInputState->stickX = Gamepad->sThumbLX;
			gpInputState->stickY = Gamepad->sThumbLY;
		}
		else
		{
			// Controller is not connected
		}
	}
}

internal void Win32HandleKeyBoardInput(WPARAM wParam, LPARAM lParam, Input_state* inputState)
{
	Keyboard_input_state* kbInputState = &inputState->keyboard;
	uint32_t vKeyCode = wParam;
	uint32_t wasDownBefore = ((lParam & (1 << 30)) != 0);
	uint32_t isDown = ((lParam & (1 << 31)) == 0);
	uint32_t isUp = ((lParam & (1 << 31)) == 1);
	if (vKeyCode == 'W')
	{
		kbInputState->up = isDown;
	}
	if (vKeyCode == 'A')
	{
		kbInputState->left = isDown;
	}
	if (vKeyCode == 'S')
	{
		kbInputState->down = isDown;
	}
	if (vKeyCode == 'D')
	{
		kbInputState->right = isDown;
	}
	if (vKeyCode == 'Q')
	{
	}
	if (vKeyCode == 'E')
	{
	}
	if (vKeyCode == VK_UP)
	{
		kbInputState->up = isDown;
	}
	if (vKeyCode == VK_DOWN)
	{
		kbInputState->down = isDown;
	}
	if (vKeyCode == VK_LEFT)
	{
		kbInputState->left = isDown;
	}
	if (vKeyCode == VK_RIGHT)
	{
		kbInputState->right = isDown;
	}
	if (vKeyCode == VK_ESCAPE)
	{
	}
	if (vKeyCode == VK_SPACE)
	{
	}
}

internal void Render(WIN32OffscreenBuffer *buffer, int xOffset, int yOffset)
{
    //static_assert(buffer);
    
	uint8_t *row = (uint8_t *)buffer->memory;

	for (int y = 0; y < buffer->height; y++)
	{
		uint32_t *pixel = (uint32_t *)row;
		for (int x = 0; x < buffer->width; x++)
		{
			uint8_t red = 0;
			uint8_t green = (y + yOffset);
			uint8_t blue = (x + xOffset);
			uint8_t blank = 0;
			// windows is glue green red
			*pixel = blue | (green << 8) | (red  << 16) | (blank << 24);
			pixel++;
		}
		row += buffer->pitch;
	}
}

internal void ResizeDIBSection(WIN32OffscreenBuffer *buffer, int width, int height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, NULL, MEM_RELEASE);
	}
	
    buffer->width = width;
    buffer->height = height;
    buffer->bytesPerPixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height; // negative so it is bottom down...
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32; //rgb but 8 bits of padding
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->info.bmiHeader.biSizeImage = 0;
    buffer->info.bmiHeader.biXPelsPerMeter = 0;
    buffer->info.bmiHeader.biYPelsPerMeter = 0;
    buffer->info.bmiHeader.biClrUsed = 0;
    buffer->info.bmiHeader.biClrImportant = 0;
	
    
	int bitMapMemorySize = buffer->bytesPerPixel * (buffer->width * buffer->height);
    buffer->memory = VirtualAlloc(NULL, bitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    buffer->pitch = buffer->bytesPerPixel * (buffer->width);


}

internal void Win32UpdateWindow(HDC hdc,
                            WIN32OffscreenBuffer *buffer,
                            int width, int height)
{

    StretchDIBits(hdc,
		          //x, y, width, height,
		          //x, y, width, height,
				  0, 0, width, height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory,
		          &buffer->info,
		          DIB_RGB_COLORS,
				  SRCCOPY);
}

/*********
* Windows callback handler
*********/
internal LRESULT MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	switch (message)
	{
		case WM_SIZE:
		{
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			//OutputDebugStringA("WM_PAINT\n");

			// BEGIN
			hdc = BeginPaint(window, &ps);
			//TextOut(hdc, 0, 0, "Hello, Windows!", 15);
			//windowHeight = ps.rcPaint.bottom - ps.rcPaint.top;
			//windowWidth = ps.rcPaint.right - ps.rcPaint.left;
			//int x = ps.rcPaint.left;
			//int y = ps.rcPaint.top;

            WIN32Dims dims = Win32GetWindowDims(window);
			Win32UpdateWindow(hdc,
                              &globalOffscreenBuffer,
                              dims.width, dims.height);

			//PatBlt(hdc, x, y, width, height, rop);
			
			// END
			EndPaint(window, &ps);
		}
		break;

		case WM_CLOSE:
		{
			sRunning = false;
			//PostQuitMessage(0);
			OutputDebugStringA("WM_CLOSE\n");
		}
		break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Win32HandleKeyBoardInput(wParam, lParam, &globalInputState);
		}
		break;

		case WM_DESTROY:
		{
			sRunning = false;
			OutputDebugStringA("WM_DESTROY\n");
		}
		break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		}
		break;

		default:
		{
			//OutputDebugStringA("default \n");
			ret = DefWindowProc(window, message, wParam, lParam);
		}
		break;
	}
	return ret;
}


/*
* Main Entry point for Windown Subsystem
*/
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	Win32LoadXInput();
	WNDCLASSA windowClass = { 0 };

    ResizeDIBSection(&globalOffscreenBuffer, 1280, 720);
    
    windowClass.style = CS_VREDRAW | CS_HREDRAW; // | CS_OWNDC;
	windowClass.hInstance = hInst;
	windowClass.lpfnWndProc = (WNDPROC)MainWindowCallback;
	windowClass.lpszClassName = "HandMadeHeroWindowClass";

	if (RegisterClassA(&windowClass))
	{
		HWND hwnd = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInst,
			0);

		if (hwnd)
		{
			
			int	xOffset = 0;
			int	yOffset = 0;
			MSG in_message;

			ShowWindow(hwnd, cmdshow);
			UpdateWindow(hwnd);
			sRunning = true;


			while(sRunning)
			{
				while(PeekMessage(&in_message,0,0,0,PM_REMOVE))
				{
					if(in_message.message == WM_QUIT)
						sRunning = false;

					TranslateMessage(&in_message); 
					DispatchMessageA(&in_message);
				}

				Win32HandleXInput(&globalInputState);
				
				if (globalInputState.gamepad.down || 
					globalInputState.keyboard.down)
				{
					yOffset++;
				}
				if (globalInputState.gamepad.up ||
					globalInputState.keyboard.up)
				{
					yOffset--;
				}
				if (globalInputState.gamepad.right ||
					globalInputState.keyboard.right)
				{
					xOffset++;
				}
				if (globalInputState.gamepad.left ||
					globalInputState.keyboard.left)
				{
					xOffset--;
				}
				if (yOffset % 101 == 0)
				{
					//XINPUT_VIBRATION vib;
					//vib.wLeftMotorSpeed = 10;
					//XInputSetState(controllerIdx, &vib);
				}
				else
				{
					//XINPUT_VIBRATION vib;
					//vib.wLeftMotorSpeed = 10;
					//XInputSetState(controllerIdx, &vib);
				}

				HDC	hdc = GetDC(hwnd);
				Render(&globalOffscreenBuffer, xOffset, yOffset);

                WIN32Dims dims = Win32GetWindowDims(hwnd);
				Win32UpdateWindow(hdc,
                                  &globalOffscreenBuffer,
                                  dims.width, dims.height);

                ReleaseDC(hwnd, hdc);
				//yOffset++;
				//xOffset++;
			}
		}
		else
		{
			// could not create window
		}
	}
	else
	{
		// could not create class
	}
	return 0;
	
	//MessageBox(0, "Message Box!!","Scott", MB_OK | MB_ICONINFORMATION);

	//printf("Hello\n");
	//std::cout << "Hey" << std::endl;
	
//    return MessageBox(NULL, "hello, world", "caption", 0);
}

