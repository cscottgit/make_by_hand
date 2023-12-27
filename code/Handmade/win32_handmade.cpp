#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <xinput.h>
#include <DSound.h>
#include <math.h>

#include "win32_handmade.h"
#include "game.h"

//#include <winuser.h>
//#include <iostream>


#define Pi32 3.14159265359f


global bool sRunning = true;

global WIN32OffscreenBuffer globalOffscreenBuffer;
global LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

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

internal void Win32InitDSound(HWND hwnd, uint32_t samplesPerSecond, uint32_t bufferSize)
{
    // load the library
 
    // get directsound object
    LPDIRECTSOUND sound;
    HRESULT result = DirectSoundCreate(0, 
                                    &sound,
                                    0);

    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.cbSize = 0;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;


    result = sound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
    {
        DSBUFFERDESC DSBufferDesc;
        ZeroMemory(&DSBufferDesc, sizeof(DSBUFFERDESC));
        DSBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        DSBufferDesc.dwBufferBytes = 0;
        DSBufferDesc.dwSize = sizeof(DSBufferDesc);

        LPDIRECTSOUNDBUFFER primaryBuffer;

        result = sound->CreateSoundBuffer(&DSBufferDesc, &primaryBuffer, 0);

        result = primaryBuffer->SetFormat(&waveFormat);
    }


    {

        DSBUFFERDESC DSBufferDesc = {};
        ZeroMemory(&DSBufferDesc, sizeof(DSBUFFERDESC));
        DSBufferDesc.dwFlags = 0;//DSBCAPS_GETCURRENTPOSITION2
        DSBufferDesc.dwBufferBytes = bufferSize;
        DSBufferDesc.dwSize = sizeof(DSBufferDesc);
        DSBufferDesc.lpwfxFormat = &waveFormat;
        //DSBufferDesc.guid3DAlgorithm = ;

        //LPDIRECTSOUNDBUFFER secondaryBuffer;
        result = sound->CreateSoundBuffer(&DSBufferDesc, &GlobalSecondaryBuffer, 0);
    }



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
    uint32_t vKeyCode = (uint32_t)wParam;
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

internal void Win32ResizeDIBSection(WIN32OffscreenBuffer *buffer, int width, int height)
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

    buffer->pitch = Align16(buffer->bytesPerPixel * (buffer->width));
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
internal LRESULT Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
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

struct win32_sound_output
{
    int SamplesPerSecond;
    int ToneHz;
    int16_t ToneVolume;
    uint32_t RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    float tSine;
    int LatencySampleCount;
};

internal void
Win32FillSoundBuffer(win32_sound_output* SoundOutput, DWORD ByteToLock, DWORD BytesToWrite)
{
    // TODO(casey): More strenuous test!
    // TODO(casey): Switch to a sine wave
    VOID* Region1;
    DWORD Region1Size;
    VOID* Region2;
    DWORD Region2Size;
    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
        &Region1, &Region1Size,
        &Region2, &Region2Size,
        0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid

        // TODO(casey): Collapse these two loops
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16_t* SampleOut = (int16_t*)Region1;
        for (DWORD SampleIndex = 0;
            SampleIndex < Region1SampleCount;
            ++SampleIndex)
        {
            // TODO(casey): Draw this out for people
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (float)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        SampleOut = (int16_t*)Region2;
        for (DWORD SampleIndex = 0;
            SampleIndex < Region2SampleCount;
            ++SampleIndex)
        {
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (float)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

/*
* Main Entry point for Windown Subsystem
*/
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    Win32LoadXInput();
    WNDCLASSA windowClass = { 0 };
    
    Win32ResizeDIBSection(&globalOffscreenBuffer, 1280, 720);
    
    windowClass.style = CS_VREDRAW | CS_HREDRAW; // | CS_OWNDC;
    windowClass.hInstance = hInst;
    windowClass.lpfnWndProc = (WNDPROC)Win32MainWindowCallback;
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
            win32_sound_output SoundOutput = {};


            // NOTE(casey): Sound test
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 256;
            SoundOutput.ToneVolume = 1200;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            Win32InitDSound(hwnd, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);
            bool SoundIsPlaying = false;
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            ShowWindow(hwnd, cmdshow);
            UpdateWindow(hwnd);
            sRunning = true;
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            uint64_t LastCycleCount = __rdtsc();

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


                // NOTE(casey): DirectSound output test
                DWORD PlayCursor;
                DWORD WriteCursor;
                if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    DWORD ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) %
                        SoundOutput.SecondaryBufferSize);

                    DWORD TargetCursor =
                        ((PlayCursor +
                            (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) %
                            SoundOutput.SecondaryBufferSize);
                    DWORD BytesToWrite;
                    // TODO(casey): Change this to using a lower latency offset from the playcursor
                    // when we actually start having sound effects.
                    if (ByteToLock > TargetCursor)
                    {
                        BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                        BytesToWrite += TargetCursor;
                    }
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }

                    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
                }



                
                WIN32Dims dims = Win32GetWindowDims(hwnd);
                Win32UpdateWindow(hdc,
                                  &globalOffscreenBuffer,
                                  dims.width, dims.height);
                
                //ReleaseDC(hwnd, hdc);



                uint64_t EndCycleCount = __rdtsc();

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                // TODO(casey): Display the value here
                uint64_t CyclesElapsed = EndCycleCount - LastCycleCount;
                int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                double MSPerFrame = (((1000.0f * (double)CounterElapsed) / (double)PerfCountFrequency));
                double FPS = (double)PerfCountFrequency / (double)CounterElapsed;
                double MCPF = ((double)CyclesElapsed / (1000.0f * 1000.0f));

                char Buffer[256];
                sprintf(Buffer, "%.02fms/f,  %.02ff/s,  %.02fMc/f\n", MSPerFrame, FPS, MCPF);
                OutputDebugStringA(Buffer);

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;


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

