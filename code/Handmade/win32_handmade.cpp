#include <windows.h>
#include <stdint.h>


//#include <winuser.h>
//#include <iostream>

#define internal static
#define global   static

global bool s_running = true;
global BITMAPINFO bitMapInfo;
global void	*bitMapMemory;
global int bitmapWidth;
global int bitmapHeight;
global int bytesPerPixel = 4;
global int windowHeight = 0;
global int windowWidth = 0;

internal void Render(int xOffset, int yOffset)
{
	int pitch = bytesPerPixel * (bitmapWidth);

	uint8_t *row = (uint8_t *)bitMapMemory;

	for (int y = 0; y < bitmapHeight; y++) 
	{
		uint32_t *pixel = (uint32_t *)row;
		for (int x = 0; x < bitmapWidth; x++) 
		{
			uint8_t red = (x + xOffset);
			uint8_t green = (y + yOffset);
			uint8_t blue = 0;
			uint8_t blank = 0;
			// windows is glue green red
			*pixel = blue | (green << 8) | (red  << 16) | (blank << 24);
			pixel++;
		}
		row += pitch;
	}
}

internal void ResizeDIBSection(int width, int height)
{
	if (bitMapMemory)
	{
		VirtualFree(bitMapMemory, NULL, MEM_RELEASE);
	}
	
	bitmapWidth = width;
	bitmapHeight = height;
	
	bitMapInfo.bmiHeader.biSize = sizeof(bitMapInfo.bmiHeader);
	bitMapInfo.bmiHeader.biWidth = bitmapWidth;
	bitMapInfo.bmiHeader.biHeight = -bitmapHeight; // negative so it is bottom down...
	bitMapInfo.bmiHeader.biPlanes = 1;
	bitMapInfo.bmiHeader.biBitCount = 32; //rgb but 8 bits of padding
	bitMapInfo.bmiHeader.biCompression = BI_RGB;

	bitMapInfo.bmiHeader.biSizeImage = 0;
	bitMapInfo.bmiHeader.biXPelsPerMeter = 0;
	bitMapInfo.bmiHeader.biYPelsPerMeter = 0;
	bitMapInfo.bmiHeader.biClrUsed = 0;
	bitMapInfo.bmiHeader.biClrImportant = 0;
	
	int bitMapMemorySize = bytesPerPixel * (bitmapWidth * bitmapHeight);
	bitMapMemory = VirtualAlloc(NULL, bitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void win32UpdateWindow(HDC hdc, RECT windowRect, int x, int y, int width, int height)
{
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;
	
	StretchDIBits(hdc,
		          //x, y, width, height,
		          //x, y, width, height,
				  0,0, bitmapWidth, bitmapHeight, 
				  0,0, windowWidth, windowHeight, 
		          bitMapMemory,
		          &bitMapInfo,
		          DIB_RGB_COLORS,
				  SRCCOPY);
}

LRESULT MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	switch (message)
	{
		case WM_SIZE:
		{
			RECT rect;
			GetClientRect(window, &rect);
			int height = rect.bottom - rect.top;
			int width = rect.right - rect.left;
			//int x = rect.left;
			//int y = rect.top;
			
			ResizeDIBSection(width, height);
			OutputDebugStringA("WM_SIZE\n");
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
			windowHeight = ps.rcPaint.bottom - ps.rcPaint.top;
			windowWidth = ps.rcPaint.right - ps.rcPaint.left;
			int x = ps.rcPaint.left;
			int y = ps.rcPaint.top;

			RECT rect;
			GetClientRect(window, &rect);

			win32UpdateWindow(hdc, rect, x, y, windowWidth, windowHeight);

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



int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	WNDCLASSA windowClass = { 0 };
	windowClass.style = CS_VREDRAW | CS_HREDRAW; // | CS_OWNDC;
	windowClass.hInstance = hInst;
	windowClass.lpfnWndProc = (WNDPROC)MainWindowCallback;
	windowClass.lpszClassName = "HandMadeHeroWindowClass";
	
	if (RegisterClass(&windowClass))
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
			int xOffset = 0, yOffset = 0;
			ShowWindow(hwnd, cmdshow);
			UpdateWindow(hwnd);
			MSG in_message;
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
				Render(xOffset,yOffset);
				HDC hdc = GetDC(hwnd);
				RECT rect;
				GetClientRect(hwnd, &rect);
				win32UpdateWindow(hdc, &rect, 0, 0, windowWidth, windowHeight);
				ReleaseDC(hwnd, hdc);
				//yOffset++;
				xOffset++;
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

