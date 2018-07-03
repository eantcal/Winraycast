// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#include <windows.h>
#pragma warning (disable: 4786)

#include "BitmapBuffer.h"

/* -------------------------------------------------------------------------- */

BitmapBuffer::BitmapBuffer(HDC hdc, HBITMAP hBitmap, int dx, int dy) :
    m_dx(dx), m_dy(dy)
{
    HDC texture_hdc = CreateCompatibleDC(hdc);

    SelectObject(texture_hdc, hBitmap);

    BITMAPINFO BmpInfo;

    memset((void*)&BmpInfo, 0, sizeof(BITMAPINFOHEADER));
    BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    BmpInfo.bmiHeader.biWidth = dx;
    BmpInfo.bmiHeader.biHeight = -dy;

    BmpInfo.bmiHeader.biPlanes = 1;
    BmpInfo.bmiHeader.biBitCount = 32;
    BmpInfo.bmiHeader.biCompression = BI_RGB;
    BmpInfo.bmiHeader.biClrUsed = 0;
    BmpInfo.bmiHeader.biClrImportant = 0;

    _bitmap = new DWORD[dx*dy];

    GetDIBits(
        texture_hdc, 
        hBitmap, 
        0, dy, 
        (LPVOID)_bitmap, 
        &BmpInfo, 
        DIB_RGB_COLORS);

    DeleteDC(texture_hdc);
}

