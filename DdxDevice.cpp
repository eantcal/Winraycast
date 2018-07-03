// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.

#include "DdxDevice.h"


/* ------------------------------------------------------------------------- */

DdxDevice& DdxDevice::getInstance() noexcept 
{
    static DdxDevice _instance;
    return _instance;
}


/* ------------------------------------------------------------------------- */

void DdxDevice::releaseObjects() noexcept
{
    if (m_pDD) {
        if (m_pDDSPrimary) {
            m_pDDSPrimary->Release();
            m_pDDSPrimary = nullptr;
        }
        m_pDD->Release();
        m_pDD = nullptr;
    }
}


/* ------------------------------------------------------------------------- */

DdxDevice::error_t DdxDevice::init(HWND hWnd, bool fullScreen , int xres, int yres)
{
    if (m_pDD) {
        return error_t::AlreadyInitialized;
    }

    // Create the main DirectDraw object
    auto hRet = DirectDrawCreateEx(
        nullptr, (VOID**)&m_pDD, IID_IDirectDraw7, nullptr);

    if (hRet != DD_OK || m_pDD == nullptr) {
        return error_t::DirectDrawCreateExFailed;
    }

    // Get normal mode
    hRet = m_pDD->SetCooperativeLevel(hWnd,
        fullScreen ? DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN : DDSCL_NORMAL);

    if (hRet != DD_OK) {
        return error_t::SetCooperativeLevelFailed;
    }

    DDSURFACEDESC2 ddsd = { 0 };
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;


    if (fullScreen) {
        hRet = m_pDD->SetDisplayMode(xres, yres, 32 /* bits per color */, 0, 0);
        if (hRet != DD_OK) {
            return error_t::SetDisplayModeFailed;
        }

        // Create the primary surface with 1 back buffer
        ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;
    }

    hRet = m_pDD->CreateSurface(&ddsd, &m_pDDSPrimary, nullptr);

    if (hRet != DD_OK) {
        return error_t::CreateSurfaceFailed;
    }

    return error_t::Success;
}

