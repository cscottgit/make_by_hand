#include <windows.h>
#include <stdint.h>


//#include <winuser.h>
//#include <iostream>

#define internal static
#define global   static

global bool s_running = true;

typedef struct _WIN32_offscreen_buffer
{
    BITMAPINFO info;
    void    *memory;
    int     width;
    int     height;
    int     bytesPerPixel;
    int     pitch;
}WIN32_offscreen_buffer;

typedef struct _WIN32_dims
{
    int width;
    int height;
}WIN32_dims;

global WIN32_offscreen_buffer global_offscreen_buffer;

internal WIN32_dims Win32GetWindowDims(HWND hwnd)
{
    WIN32_dims dims;
    
    RECT rect;
    GetClientRect(window, &rect);
    dims.height = rect.bottom - rect.top;
    dims.width = rect.right - rect.left;
    
    return dims;
}


internal void Render(WIN32_offscreen_buffer buffer, int xOffset, int yOffset)
{
    //static_assert(buffer);
    
	uint8_t *row = (uint8_t *)buffer.memory;

	for (int y = 0; y < buffer.height; y++)
	{
		uint32_t *pixel = (uint32_t *)row;
		for (int x = 0; x < buffer.width; x++)
		{
			uint8_t red = (x + xOffset);
			uint8_t green = (y + yOffset);
			uint8_t blue = 0;
			uint8_t blank = 0;
			// windows is glue green red
			*pixel = blue | (green << 8) | (red  << 16) | (blank << 24);
			pixel++;
		}
		row += buffer.pitch;
	}
}

internal void ResizeDIBSection(WIN32_offscreen_buffer* buffer, int width, int height)
{
	if (bitMapMemory)
	{
		VirtualFree(bitMapMemory, NULL, MEM_RELEASE);
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

    buffer->pitch = buffer.bytesPerPixel * (buffer.width);


}

internal void Win32UpdateWindow(HDC hdc,
                            WIN32_offscreen_buffer buffer,
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

LRESULT MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
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

            WIN32_dims dims = Win32GetWindowDimension(window);
			Win32UpdateWindow(hdc,
                              global_offscreen_buffer,
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

    ResizeDIBSection(&global_offscreen_buffer, 1280, 720);

    
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
				Render(global_offscreen_buffer, xOffset, yOffset);
				HDC hdc = GetDC(hwnd);
                
                WIN32_dims dims = Win32GetWindowDimension(window);
				Win32UpdateWindow(hdc,
                                  global_offscreen_buffer,
                                  dims.width, dims.height);

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

