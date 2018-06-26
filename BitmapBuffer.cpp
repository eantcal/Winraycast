// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
//
// This file is part of the WinRayCast Application (a 3D Engine Demo).
// This program is free software; you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this program; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.


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

