// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#ifndef __BITMAPBUFFER_H__
#define __BITMAPBUFFER_H__

/* -------------------------------------------------------------------------- */

#include <windows.h>
#include <stdint.h>


/* -------------------------------------------------------------------------- */

class BitmapBuffer {
public:
    BitmapBuffer(HDC hdc, HBITMAP hBitmap, int dx, int dy);

    virtual ~BitmapBuffer() { 
        delete[] _bitmap; 
    }

    DWORD getPixel(unsigned int x, unsigned int y) const noexcept {
        if ((x < (unsigned int)m_dx) && (y < (unsigned int)m_dy)) {
            return _bitmap[(x + (y * m_dx))];
        }

        return 0;
    }

    void fillBuffer(void* destBuf, int offset, int org_dx) {
        for (long y = 0; y < m_dy; ++y) {
            for (int x = 0; x < m_dx; ++x) {
                (*((DWORD*)destBuf + x + int64_t(y) * int64_t(m_dx))) = 
                    getPixel((x + offset) % org_dx, y);
            }
        }
    }

private:
    DWORD * _bitmap = nullptr;
    int m_dx = 0, m_dy = 0;
};


/* -------------------------------------------------------------------------- */

#endif // __BITMAPBUFFER_H__
