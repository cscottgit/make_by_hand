#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#include "win32_handmade.h"

//#include <winuser.h>
//#include <iostream>

global bool s_running = true;

global WIN32OffscreenBuffer globalOffscreenBuffer;



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

internal void Win32HandleKeyBoardInput(WPARAM wParam, LPARAM lParam)
{
	uint32_t vKeyCode = wParam;
	uint32_t wasDownBefore = ((lParam & (1 << 30)) != 0);
	uint32_t isDown = ((lParam & (1 << 31)) == 0);
	uint32_t isUp = ((lParam & (1 << 31)) == 1);
	if (vKeyCode == 'W')
	{
	}
	if (vKeyCode == 'A')
	{
	}
	if (vKeyCode == 'S')
	{
	}
	if (vKeyCode == 'D')
	{
	}
	if (vKeyCode == 'Q')
	{
	}
	if (vKeyCode == 'E')
	{
	}
	if (vKeyCode == VK_UP)
	{
	}
	if (vKeyCode == VK_DOWN)
	{
	}
	if (vKeyCode == VK_LEFT)
	{
	}
	if (vKeyCode == VK_RIGHT)
	{
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
			s_running = false;
			//PostQuitMessage(0);
			OutputDebugStringA("WM_CLOSE\n");
		}
		break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Win32HandleKeyBoardInput(wParam, lParam);
		}
		break;

		case WM_DESTROY:
		{
			s_running = false;
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
			s_running = true;


			while(s_running)
			{
				while(PeekMessage(&in_message,0,0,0,PM_REMOVE))
				{
					if(in_message.message == WM_QUIT)
						s_running = false;

					TranslateMessage(&in_message); 
					DispatchMessageA(&in_message);
				}

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
						bool up		= (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool down	= (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool left	= (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool right	= (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start	= (Gamepad->wButtons & XINPUT_GAMEPAD_START);
						bool back	= (Gamepad->wButtons & XINPUT_GAMEPAD_BACK);
						bool leftThumb	= (Gamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
						bool rightThumb	= (Gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
						bool leftShoulder	= (Gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool rightShoulder	= (Gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool aButton	= (Gamepad->wButtons & XINPUT_GAMEPAD_A);
						bool bButton	= (Gamepad->wButtons & XINPUT_GAMEPAD_B);
						bool xButton	= (Gamepad->wButtons & XINPUT_GAMEPAD_X);
						bool yButton	= (Gamepad->wButtons & XINPUT_GAMEPAD_Y);

						int16_t stickX = Gamepad->sThumbLX;
						int16_t stickY = Gamepad->sThumbLY;


						if (down)
						{
							yOffset++;
						}
						if (up)
						{
							yOffset--;
						}
						if (right)
						{
							xOffset++;
						}
						if (left)
						{
							xOffset--;
						}
						if (yOffset % 101 == 0)
						{
							XINPUT_VIBRATION vib;
							vib.wLeftMotorSpeed = 10;
							XInputSetState(controllerIdx, &vib);
						}
						else
						{
							XINPUT_VIBRATION vib;
							vib.wLeftMotorSpeed = 10;
							XInputSetState(controllerIdx, &vib);
						}
							 

					}
					else
					{
						// Controller is not connected
					}
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

