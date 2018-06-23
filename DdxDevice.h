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

#ifndef __DDXDEVICE_H__
#define __DDXDEVICE_H__

#include <ddraw.h>
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")


/* -------------------------------------------------------------------------- */

class DdxDevice
{
public:
    friend class Ctx;

    class Ctx {
    public:
        Ctx(DdxDevice& renderer) :
            m_renderer(renderer)
        {
            if ((renderer.m_pDDSPrimary)->GetDC(&m_hdc) != DD_OK) {
                m_hdc = nullptr;
            }
        }

        HDC getDc() const noexcept {
            return m_hdc;
        }

        ~Ctx() {
            if (m_hdc) {
                m_renderer.m_pDDSPrimary->ReleaseDC(m_hdc);
            }
        }
    private:
        DdxDevice & m_renderer;
        HDC m_hdc = nullptr;
    };

    enum class error_t {
        Success,
        AlreadyInitialized,
        DirectDrawCreateExFailed,
        SetCooperativeLevelFailed,
        SetDisplayModeFailed,
        CreateSurfaceFailed,
    };

    static DdxDevice& getInstance() noexcept;

    error_t init(HWND hWnd, bool fullScreen, int xres, int yres);

    void releaseObjects() noexcept;

    bool ready() const noexcept {
        return m_pDD && m_pDDSPrimary;
    }

private:
    DdxDevice() {}

    // DirectDraw object
    LPDIRECTDRAW7 m_pDD = nullptr;

    // DirectDraw primary surface
    LPDIRECTDRAWSURFACE7 m_pDDSPrimary = nullptr;
};


/* -------------------------------------------------------------------------- */

#endif // __DDXDEVICE_H__