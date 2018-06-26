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

#ifndef __BITMAPBUFFER_H__
#define __BITMAPBUFFER_H__

/* -------------------------------------------------------------------------- */

#include <windows.h>


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
        for (int y = 0; y < m_dy; ++y) {
            for (int x = 0; x < m_dx; ++x) {
                (*((DWORD*)destBuf + x + y * m_dx)) = 
                    getPixel((x + offset) % org_dx, y);
            }
        }
    }

private:
    DWORD * _bitmap;
    int m_dx, m_dy;
};


/* -------------------------------------------------------------------------- */

#endif // __BITMAPBUFFER_H__