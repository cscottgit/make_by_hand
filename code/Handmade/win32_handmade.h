#ifndef _WIN32_HANDMADE_
#define _WIN32_HANDMADE_
#include "build_opts.h"



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


#endif /* _WIN32_HANDMADE_ */