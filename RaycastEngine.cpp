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

#include "RaycastEngine.h"

/* -------------------------------------------------------------------------- */
// RAYCAST ENGINE
/* -------------------------------------------------------------------------- */

#define TRANSP_COLOR RGB(0,0,0)

#include <ddraw.h>
extern LPDIRECTDRAW7          g_pDD;        // DirectDraw object
extern LPDIRECTDRAWSURFACE7   g_pDDSPrimary;// DirectDraw primary surface
extern LPDIRECTDRAWSURFACE7   g_pDDSBack;   // DirectDraw back surface
extern BOOL                   g_bActive;    // Is application active?


/* -------------------------------------------------------------------------- */

static
DDSURFACEDESC2  DDSurfaceDesc2;


/* -------------------------------------------------------------------------- */

BYTE* LockSurfaceAndGetAddr(IDirectDrawSurface7* surface)
{
    memset(&DDSurfaceDesc2, 0, sizeof(DDSURFACEDESC2));
    DDSurfaceDesc2.dwSize = sizeof(DDSURFACEDESC2);

    if (SUCCEEDED(
        surface->Lock(NULL, &DDSurfaceDesc2, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL))
        )
    {
        return (BYTE*)DDSurfaceDesc2.lpSurface;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

inline
void UnlockSurface(IDirectDrawSurface7* surface) {
    surface->Unlock(NULL);
}


/* -------------------------------------------------------------------------- */

inline
void DDrawPixel32(BYTE* surface, unsigned int x, unsigned int y, DWORD color_value) {
    if (y < DDSurfaceDesc2.dwHeight && x < DDSurfaceDesc2.dwWidth) {
        *((DWORD*)(surface + (x << 2) + (y*DDSurfaceDesc2.lPitch))) = color_value;
    }
}


/* -------------------------------------------------------------------------- */

TextureBuffer* getTextureMap(HDC hdc, HBITMAP hBitmap, int width, int height) {
    static std::map<HBITMAP, TextureBuffer*> TextureImageCollection;

    TextureBuffer* textureBuf;
     
    auto i = TextureImageCollection.find(hBitmap);

    if (i == TextureImageCollection.end()) {
        textureBuf = new TextureBuffer(hdc, hBitmap, width, height);
        TextureImageCollection.insert({ hBitmap, textureBuf });
    }
    else {
        textureBuf = i->second;
    }

    return textureBuf;
}


/* -------------------------------------------------------------------------- */

static const double POSITIVE_INFINITY = 1000000.0;
static const double SMALLEST_EPSILON = double(1.0) / POSITIVE_INFINITY;


/* -------------------------------------------------------------------------- */

void RaycastEngine:: horzint1st(
    WorldMap& wMap,
    int ray,
    const double& M1,
    fpoint_t& point) const noexcept
{
    const double xp = m_camera.getX();
    const double yp = m_camera.getY();

    double yi = ray >= m_camera.deg180() && ray < m_camera.deg360() ?
        ((double)wMap.get_camera_cell_pos().second) * wMap.getCellDy() :
        ((double)wMap.get_camera_cell_pos().second + 1) * wMap.getCellDy();

    double xi = M1 * (yi - yp) + xp;

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::vertint1st(
    WorldMap& wMap,
    int ray,
    const double& M,
    fpoint_t& point) const noexcept
{
    const double xp = m_camera.getX();
    const double yp = m_camera.getY();

    const double xi = ray >= m_camera.deg90() && ray < m_camera.deg270() ?
        ((double)wMap.get_camera_cell_pos().first) * wMap.getCellDx() :
        ((double)wMap.get_camera_cell_pos().first + 1) * wMap.getCellDx();

    const double yi = M * (xi - xp) + yp;

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

void
RaycastEngine::
vertint(WorldMap& wMap,
    const fpoint_t& firstInt,
    int ray,
    const double& M,
    fpoint_t& point) const noexcept
{
    double xi, yi;

    if (ray >= m_camera.deg90() && ray < m_camera.deg270()) {
        yi = firstInt.second - M * wMap.getCellDx();
        xi = firstInt.first - wMap.getCellDx();
    }
    else {
        yi = firstInt.second + M * wMap.getCellDx();
        xi = firstInt.first + wMap.getCellDx();
    }

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

void
RaycastEngine::
horzint(WorldMap& wMap,
    const fpoint_t& firstInt,
    int ray,
    const double& M1,
    fpoint_t& point) const noexcept
{
    double xi, yi;

    if (ray >= m_camera.deg180() && ray < m_camera.deg360()) {
        xi = firstInt.first - M1 * wMap.getCellDy();
        yi = firstInt.second - wMap.getCellDy();
    }
    else {
        xi = firstInt.first + M1 * wMap.getCellDy();
        yi = firstInt.second + wMap.getCellDy();
    }

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

cell_t
RaycastEngine::
horzWall(WorldMap& wMap,
    const fpoint_t& point,
    int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (ray >= m_camera.deg180() && ray < m_camera.deg360()) {
        --r;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    }
    else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    }
    else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

cell_t
RaycastEngine::
vertWall(WorldMap& wMap,
    const fpoint_t& point,
    int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (ray >= m_camera.deg90() && ray < m_camera.deg270()) {
        --c;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    }
    else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    }
    else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

cell_t
RaycastEngine::
horzIntWall(WorldMap& wMap,
    const fpoint_t& point,
    int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (!(ray >= m_camera.deg180() && ray < m_camera.deg360())) {
        --r;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    }
    else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    }
    else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

cell_t
RaycastEngine::
vertIntWall(WorldMap& wMap,
    const fpoint_t& point,
    int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (!(ray >= m_camera.deg90() && ray < m_camera.deg270())) {
        --c;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    }
    else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    }
    else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

void
RaycastEngine::
shadingStretchBtl(HDC dest_hdc,
    int xDest,
    int yDest,
    int heightDest,
    int xSrc,
    int ySrc,
    int height_source,
    int widthSrc,
    int maxVisibleY,
    double depthPar,
    HBITMAP hBitmap)
{
    heightDest += 2;

    double step = double(height_source) / double(heightDest);

    double ys = double(ySrc);
    int yd = yDest - 1;
    int max_yd = min(maxVisibleY, heightDest + yDest);

    double Rcomp, Gcomp, Bcomp;

    if (yd < 0) {
        ys += (-yd)*step;
        yd = 0;
    }

    TextureBuffer* textureBuf =
        getTextureMap(dest_hdc, hBitmap, widthSrc, height_source);

    while (yd < max_yd && ys < height_source) {
        COLORREF c = textureBuf->getPixel(xSrc % widthSrc, int(ys) % height_source);

        if (depthPar<double(1.0)) {
            Rcomp = depthPar * (GetRValue(c));
            Gcomp = depthPar * (GetGValue(c));
            Bcomp = depthPar * (GetBValue(c));

            DDrawPixel32(m_videoBuf, xDest, yd, RGB(Rcomp, Gcomp, Bcomp));
        } // if
        else {
            DDrawPixel32(m_videoBuf, xDest, yd, c);
        }

        ++yd;
        ys += step;
    }
}


/* -------------------------------------------------------------------------- */

void
RaycastEngine::
transpShadingStretchBtl(HDC dest_hdc,
    int xDest,
    int yDest,
    int heightDest,
    int xSrc,
    int ySrc,
    int height_source,
    int widthSrc,
    int maxVisibleY,
    double depthPar,
    HBITMAP hBitmap,
    int transpC)
{
    heightDest += 2;

    double step = double(height_source) / double(heightDest);

    double ys = double(ySrc);

    int yd = yDest - 1;
    int max_yd = min(maxVisibleY, heightDest + yDest);

    double Rcomp, Gcomp, Bcomp;

    if (yd < 0) {
        ys += (-yd)*step;
        yd = 0;
    }

    TextureBuffer* textureBuf =
        getTextureMap(dest_hdc, hBitmap, widthSrc, height_source);

    while (yd < max_yd && ys < height_source) {
        COLORREF c = textureBuf->getPixel(xSrc % widthSrc, int(ys) /*% height_source*/);

        if (transpC != c) {
            if (depthPar<double(1.0)) {
                Rcomp = depthPar * (GetRValue(c));
                Gcomp = depthPar * (GetGValue(c));
                Bcomp = depthPar * (GetBValue(c));

                DDrawPixel32(m_videoBuf, xDest, yd, RGB(Rcomp, Gcomp, Bcomp));
            } // if
            else {
                DDrawPixel32(m_videoBuf, xDest, yd, c);
            }
        }

        ++yd;
        ys += step;
    }
}


/* -------------------------------------------------------------------------- */

void
RaycastEngine::
renderTranspWall(int videoPosX,
    int videoPosY,
    HDC videoHdc,
    WorldMap& wMap,
    const RECT& rt,
    bool render_internal_wall)
{
    if (!g_pDDSPrimary) return;

    double d = -1.0; // distance from intersection

    int cameraRayOffset = m_camera.getAlpha();
    int cameraXPos = m_camera.getX();
    int cameraYPos = m_camera.getY();

    // main casting loop (for each pixel of projection x coord...)
    for (int ray = 0; ray < m_camera.getXProjRes(); ++ray) {
        int relRay = ray + cameraRayOffset;

        if (relRay < 0) relRay += m_camera.deg360();
        else if (relRay >= m_camera.deg360()) relRay -= m_camera.deg360();

        double M = getM(relRay);
        double M1 = getM1(relRay);

        //Search first intersection with the grid (WorldMap)
        fpoint_t pv;
        fpoint_t ph;

        horzint1st(wMap, relRay, M1, ph);
        vertint1st(wMap, relRay, M, pv);

        cell_t hCellVal = 0;
        cell_t vCellVal = 0;

        cell_t mapKey = 0;

        bool v_not_found = false;
        bool h_not_found = false;

        if (render_internal_wall) {
            while (vCellVal = vertIntWall(wMap, pv, relRay), (vCellVal & 0xff0000ff) == 0) {
                vertint(wMap, pv, relRay, M, pv);
                if (!isInClientRect(wMap, pv)) {
                    v_not_found = true;
                    break;
                }
            }

            while (hCellVal = horzIntWall(wMap, ph, relRay), (hCellVal & 0xff0000ff) == 0) {
                horzint(wMap, ph, relRay, M1, ph);
                if (!isInClientRect(wMap, ph)) {
                    h_not_found = true;
                    break;
                }
            }
        }
        else {
            while (vCellVal = vertWall(wMap, pv, relRay), (vCellVal & 0xff0000ff) == 0) {
                vertint(wMap, pv, relRay, M, pv);
                if (!isInClientRect(wMap, pv)) {
                    v_not_found = true;
                    break;
                }
            }

            while (hCellVal = horzWall(wMap, ph, relRay), (hCellVal & 0xff0000ff) == 0) {
                horzint(wMap, ph, relRay, M1, ph);
                if (!isInClientRect(wMap, ph)) {
                    h_not_found = true;
                    break;
                }
            }
        }

        if (v_not_found && h_not_found) {
            continue;
        }

        //Compute the distance with intersections
        double dh = horzDist(ph, relRay);
        double dv = vertDist(pv, relRay);

        bool vert = true;

        //What's the nearest to the camera ?
        if (dh < dv) {
            mapKey = hCellVal;
            d = dh;
            vert = false;
        }
        else {
            mapKey = vCellVal;
            d = dv;
        }

        if (render_internal_wall && (mapKey & 0xFF)) {
            continue;
        }

        const cell_t wallKey = (mapKey & 0xFF000000) >> 24;
        const cell_t wallHeight = (mapKey & 0xff00000000UL) >> 32;

        if (!wallKey) {
            continue;
        }
        //Compute the view distort LTU
        int distortDeg = ray - m_camera.degHalfVisual();

        if (distortDeg >= m_camera.deg360()) {
            distortDeg -= m_camera.deg360();
        }
        else if (distortDeg < 0) {
            distortDeg += m_camera.deg360();
        }

        double viewDistortLut = m_camera.cos(distortDeg);
        double scaledDistortLut = m_scale / viewDistortLut;

        int k = 0;
        int centerProj = 0;

        //Prevent division by zero
        if (d > double(0.0)) {
            k = int(scaledDistortLut / d);
            centerProj = int(k*m_camera.getCenterProj());
        }

        if (unsigned(k) < POSITIVE_INFINITY) {
            ////////////////////
            // Walls rendering 

            int cellBound = 0;
            int currentCellRay = 0;

            if (vert) {
                cellBound = wMap.getCellDy();
                currentCellRay = int(pv.second) % cellBound;
            }
            else {
                cellBound = wMap.getCellDx();
                currentCellRay = int(ph.first) % cellBound;
            }

            if (currentCellRay >= 0 && currentCellRay < cellBound) {
                int x_coord_source = currentCellRay;

                double shadingAttr = double(k) / double(m_depthShadingPar);

                if (wallHeight &&
                    (wMap[cameraYPos / wMap.getCellDy()]
                        [cameraXPos / wMap.getCellDx()] & 0xff00) == 0xff00)
                {
                    transpShadingStretchBtl(
                        videoHdc,
                        ray,
                        ((m_camera.getSlope() + m_camera.getYProjRes()) >> 1) - centerProj - k,
                        k,
                        x_coord_source,
                        0,
                        wMap.getCellDy(), //height
                        wMap.getCellDx(), //width (do not invert it)
                        m_camera.getYProjRes(),
                        shadingAttr,
                        wMap.getBmp(wallHeight & 0xff),
                        TRANSP_COLOR
                    );
                }

                transpShadingStretchBtl(
                    videoHdc,
                    ray,
                    ((m_camera.getSlope() + m_camera.getYProjRes()) >> 1) - centerProj,
                    k,
                    x_coord_source,
                    0,
                    wMap.getCellDy(), //height
                    wMap.getCellDx(), //width (do not invert it)
                    m_camera.getYProjRes(),
                    shadingAttr,
                    wMap.getBmp(wallKey & 0xff),
                    TRANSP_COLOR
                );
            } // if current_cell...
        } // if k...

    } // for ray

}


/* -------------------------------------------------------------------------- */

void
RaycastEngine::
renderScene(int videoPosX, int videoPosY,
    HDC videoHdc,
    WorldMap& wMap,
    const RECT& rt)
{
    if (!g_pDDSPrimary) {
        return;
    }

    int videoBufSize = rt.right * rt.bottom * 4;

    if (!m_videoBuf) {
        LockSurfaceAndGetAddr(g_pDDSPrimary);
        UnlockSurface(g_pDDSPrimary);

        DDSurfaceDesc2.lPitch = rt.right * 4;
        DDSurfaceDesc2.dwWidth = rt.right;
        DDSurfaceDesc2.dwHeight = rt.bottom;

        m_videoBuf = new BYTE[videoBufSize];
    }

    TextureBuffer* textureBuf = getTextureMap(videoHdc,
        wMap.getBmp(0xff),
        rt.right,
        rt.bottom);

    wMap.setCamera(m_camera);

    double d = -1.0; // distance from intersection

    const int cameraRayOffset = m_camera.getAlpha();
    const int cameraXPos = m_camera.getX();
    const int cameraYPos = m_camera.getY();

    const int org_x_res = m_camera.getXProjRes();

    textureBuf->fillBuffer(m_videoBuf, cameraRayOffset + m_fps, org_x_res);

    // main casting loop (for each pixel of projection x coord...)
    for (int ray = 0; ray < org_x_res; ++ray) {
        int relRay = ray + cameraRayOffset;

        if (relRay < 0) relRay += m_camera.deg360();
        else if (relRay >= m_camera.deg360()) relRay -= m_camera.deg360();

        const double M = getM(relRay);
        const double M1 = getM1(relRay);

        //Search first intersection with the grid (WorldMap)
        fpoint_t pv;
        fpoint_t ph;

        horzint1st(wMap, relRay, M1, ph);
        vertint1st(wMap, relRay, M, pv);

        cell_t hCellVal = 0;
        cell_t vCellVal = 0;

        cell_t mapKey = 0;

        //while you don't cross a wall limit, search for next intersection
        while (vCellVal = vertWall(wMap, pv, relRay), (vCellVal & 0xff) == 0) {
            vertint(wMap, pv, relRay, M, pv);
            if (!isInClientRect(wMap, pv))
                break;
        }

        while (hCellVal = horzWall(wMap, ph, relRay), (hCellVal & 0xff) == 0) {
            horzint(wMap, ph, relRay, M1, ph);
            if (!isInClientRect(wMap, ph))
                break;
        }

        //Compute the distance with intersections
        const double dh = horzDist(ph, relRay);
        const double dv = vertDist(pv, relRay);

        bool vert = true;

        //What's the nearest to the camera ?
        if (dh < dv) {
            mapKey = hCellVal;
            d = dh;
            vert = false;
        }
        else {
            mapKey = vCellVal;
            d = dv;
        }

        const int wallKey = mapKey & 0xff;
        const int wallHeight = (mapKey & 0xff00000000UL) >> 32;

        //Compute the view distort LTU
        int distortDeg = ray - m_camera.degHalfVisual();

        if (distortDeg >= m_camera.deg360()) {
            distortDeg -= m_camera.deg360();
        }
        else if (distortDeg < 0) {
            distortDeg += m_camera.deg360();
        }

        const double viewDistortLut = m_camera.cos(distortDeg);
        const double scaledDistortLut = m_scale / viewDistortLut;
        const double ceilScaledDistortLut = scaledDistortLut * (double)m_camera.getCenterProj();
        const double floorScaledDistortLut = scaledDistortLut - ceilScaledDistortLut;

        int k = 0;
        int centerProj = 0;

        //Prevent division by zero
        if (d > 0.0) {
            k = int(scaledDistortLut / d);
            centerProj = int(k*m_camera.getCenterProj());
        }

        if (unsigned(k) < POSITIVE_INFINITY) {
            // Ceil rendering 
            int ceilBottom = ((m_camera.getYProjRes() + m_camera.getSlope()) >> 1);

            //For each visible y-coord of screen
            for (int ceilRay = 0; ceilRay < (ceilBottom - centerProj); ++ceilRay) {
                const double deltaC = ceilBottom - ceilRay;

                if (deltaC <= 0.0) continue;

                const double distToPtOnCeiling = ceilScaledDistortLut / deltaC;

                const int xPicture = int(m_camera.cos(relRay)*distToPtOnCeiling) + cameraXPos;
                const int yPicture = int(m_camera.sin(relRay)*distToPtOnCeiling) + cameraYPos;

                const int cellDx = wMap.getCellDx();
                const int cellDy = wMap.getCellDy();

                cell_t ceilKey = 0;

                const int row = yPicture / cellDy;
                const int col = xPicture / cellDx;

                if (row<int(wMap.getRowCount()) && col<int(wMap.getColCount()) && col >= 0 && row >= 0) {
                    const cell_t mapKey = wMap[row][col];
                    
                    if (mapKey & 0xff) 
                        continue;
                    
                    ceilKey = (mapKey >> 8) & 0xff;
                }
                else {
                    continue;
                }

                if (ceilKey == 0xff) {
                    continue;
                }

                const auto textureBuf = 
                    getTextureMap(videoHdc, wMap.getBmp(ceilKey & 0xff), cellDx, cellDy);

                const double shadingAttr = m_ceilFloorShadingPar / double(distToPtOnCeiling);

                const COLORREF c = textureBuf->getPixel(xPicture % cellDx, yPicture % cellDy);

                if (shadingAttr >= 1.0) {
                    DDrawPixel32(m_videoBuf, ray, ceilRay, c);
                }
                else {
                    const double Rcomp = shadingAttr * (GetRValue(c));
                    const double Gcomp = shadingAttr * (GetGValue(c));
                    const double Bcomp = shadingAttr * (GetBValue(c));

                    DDrawPixel32(m_videoBuf, ray, ceilRay, RGB(Rcomp, Gcomp, Bcomp));
                } //else
            }

            // Floor rendering
            for (int floorRay = m_camera.getSlope();
                floorRay < (ceilBottom + centerProj);
                ++floorRay)
            {
                TextureBuffer* textureBuf;

                const double deltaC = ceilBottom - floorRay;
                if (deltaC <= 0.0) continue;

                const double distToPtOnCeiling = floorScaledDistortLut / deltaC;

                const int xPicture = int(m_camera.cos(relRay)*distToPtOnCeiling) + cameraXPos;
                const int yPicture = int(m_camera.sin(relRay)*distToPtOnCeiling) + cameraYPos;

                const int cellDx = wMap.getCellDx();
                const int cellDy = wMap.getCellDy();

                int floorKey = 0;

                const int row = yPicture / cellDy;
                const int col = xPicture / cellDx;

                if ((row<int(wMap.getRowCount())) && col<int(wMap.getColCount()) && row >= 0 && col >= 0) {
                    const cell_t mapKey = wMap[row][col];
                    if (mapKey & 0xff) continue;
                    floorKey = (mapKey & 0x00ff0000) >> 16;
                }
                else {
                    continue;
                }

                if (floorKey == 0xFF) {
                    continue;
                }

                textureBuf = getTextureMap(videoHdc, wMap.getBmp(floorKey), cellDx, cellDy);

                const double shadingAttr = m_ceilFloorShadingPar / double(distToPtOnCeiling);
                const COLORREF c = textureBuf->getPixel(xPicture % cellDx, yPicture % cellDy);

                const int y = m_camera.getSlope() + m_camera.getYProjRes() - floorRay;

                if (shadingAttr >= 1.0) {
                    DDrawPixel32(m_videoBuf, ray, y, c);
                }
                else {
                    const double Rcomp = shadingAttr * (GetRValue(c));
                    const double Gcomp = shadingAttr * (GetGValue(c));
                    const double Bcomp = shadingAttr * (GetBValue(c));

                    DDrawPixel32(m_videoBuf, ray, y, RGB(Rcomp, Gcomp, Bcomp));
                } //else
            } // for


            ////////////////////
            // Walls rendering 
            if (wallKey && wallKey != 0xff) {
                int cellBound = 0;
                int currentCellRay = 0;

                // Determine the cell bound
                if (vert) {
                    cellBound = wMap.getCellDy();
                    currentCellRay = int(pv.second) % cellBound;
                }
                else {
                    cellBound = wMap.getCellDx();
                    currentCellRay = int(ph.first) % cellBound;
                }

                const HBITMAP current_bmp = wMap.getBmp(wallKey);

                const double shadingAttr = double(k) / double(m_depthShadingPar);

                if (wallHeight) {
                    shadingStretchBtl(
                        videoHdc,
                        ray,
                        ((m_camera.getSlope() + m_camera.getYProjRes()) >> 1) - centerProj - k,
                        k,
                        currentCellRay, //x_coord_source,
                        0,
                        wMap.getCellDy(), //height
                        wMap.getCellDx(), //width (do not invert it)
                        m_camera.getYProjRes(),
                        shadingAttr,
                        wMap.getBmp(wallHeight & 0xff));

                    // Ceil rendering 
                    const int ceilBottom = ((m_camera.getYProjRes() + m_camera.getSlope()) >> 1);

                    //For each visible y-coord of screen
                    for (int ceilRay = 0;
                        ceilRay < (ceilBottom - centerProj);
                        ++ceilRay)
                    {

                        const double deltaC = ceilBottom - ceilRay;

                        if (deltaC <= 0.0) continue;
                        const double distToPtOnCeiling = ceilScaledDistortLut / deltaC;

                        const int xPicture = int(m_camera.cos(relRay)*distToPtOnCeiling) + cameraXPos;
                        const int yPicture = int(m_camera.sin(relRay)*distToPtOnCeiling) + cameraYPos;

                        const int cellDx = wMap.getCellDx();
                        const int cellDy = wMap.getCellDy();

                        int ceilKey = 0;

                        const int row = yPicture / cellDy;
                        const int col = xPicture / cellDx;

                        if (row<int(wMap.getRowCount()) && col<int(wMap.getColCount()) && col >= 0 && row >= 0) {
                            cell_t mapKey = wMap[row][col];
                            if (mapKey & 0xff) continue;
                            ceilKey = (mapKey >> 8) & 0xff;
                        }
                        else {
                            continue;
                        }

                        if (ceilKey == 0xff) {
                            continue;
                        }

                        const auto textureBuf = 
                            getTextureMap(videoHdc, wMap.getBmp(ceilKey), cellDx, cellDy);

                        const double shadingAttr = m_ceilFloorShadingPar / double(distToPtOnCeiling);

                        const COLORREF c = textureBuf->getPixel(xPicture % cellDx, yPicture % cellDy);

                        if (shadingAttr >= 1.0) {
                            DDrawPixel32(m_videoBuf, ray, ceilRay, c);
                        }
                        else {
                            const double Rcomp = shadingAttr * (GetRValue(c));
                            const double Gcomp = shadingAttr * (GetGValue(c));
                            const double Bcomp = shadingAttr * (GetBValue(c));

                            DDrawPixel32(m_videoBuf, ray, ceilRay, RGB(Rcomp, Gcomp, Bcomp));
                        } //else
                    }
                }

                shadingStretchBtl(
                    videoHdc,
                    ray,
                    ((m_camera.getSlope() + m_camera.getYProjRes()) >> 1) - centerProj,
                    k,
                    currentCellRay, //x_coord_source,
                    0,
                    wMap.getCellDy(), //height
                    wMap.getCellDx(), //width (do not invert it)
                    m_camera.getYProjRes(),
                    shadingAttr,
                    current_bmp
                );
            }
        } // if k...
    } // for

    renderTranspWall(videoPosX, videoPosY, videoHdc, wMap, rt, true);  //internal
    renderTranspWall(videoPosX, videoPosY, videoHdc, wMap, rt, false); //external

    HDC dxHdc;

    if ((g_pDDSPrimary)->GetDC(&dxHdc) == DD_OK) {

        BITMAPINFO BmpInfo;

        memset((void*)&BmpInfo, 0, sizeof(BITMAPINFOHEADER));

        BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        BmpInfo.bmiHeader.biWidth = rt.right;
        BmpInfo.bmiHeader.biHeight = rt.bottom;
        BmpInfo.bmiHeader.biPlanes = 1;
        BmpInfo.bmiHeader.biBitCount = 32;
        BmpInfo.bmiHeader.biCompression = BI_RGB;
        BmpInfo.bmiHeader.biClrUsed = 0;
        BmpInfo.bmiHeader.biClrImportant = 0;

        StretchDIBits(
            dxHdc,                         // handle to DC
            videoPosX,                      // x-coord of destination upper-left corner
            rt.bottom + videoPosY,          // y-coord of destination upper-left corner
            rt.right,                       // width of destination rectangle
            -rt.bottom,                     // height of destination rectangle
            0,                              // x-coord of source upper-left corner
            0,                              // y-coord of source upper-left corner
            m_camera.getXProjRes(),         // width of source rectangle
            m_camera.getYProjRes(),         // height of source rectangle
            (CONST VOID *)m_videoBuf,       // bitmap bits
            (CONST BITMAPINFO *)&BmpInfo,   // bitmap data
            DIB_RGB_COLORS,                 // usage options
            SRCCOPY                         // raster operation code
        );
        g_pDDSPrimary->ReleaseDC(dxHdc);
    }

    ++m_fps;
}


/* -------------------------------------------------------------------------- */
// CAMERA
/* -------------------------------------------------------------------------- */

cell_t Camera::moveTo(int offset, WorldMap& wMap, int deg)
{
    cell_t retVal = 0;

    try {
        int alpha = m_alpha + degHalfVisual() + deg;
        
        if (alpha >= m_deg360) {
            alpha -= m_deg360;
        }
        else if (alpha < 0) {
            alpha += m_deg360;
        }

        double x = double(offset)*m_cosTbl[alpha];
        double y = double(offset)*m_sinTbl[alpha];

        const int c = (int)(m_x + x) / wMap.getCellDx();
        const int r = (int)(m_y + y) / wMap.getCellDy();

        if (r >= wMap.getRowCount() || c >= wMap.getColCount()) {
            return retVal;
        }

        retVal = wMap[r][c];

        if ((retVal & 0x000000ff) == 0) {
            m_x += x;
            m_y += y;
        }
    }
    catch (...) {
    }

    return retVal;
}


/* -------------------------------------------------------------------------- */

inline double sign(double x) {
    return x == 0.0 ? 0.0 : (x>.0 ? 1. : -1.);
}


/* -------------------------------------------------------------------------- */

Camera::Camera(
    int x, int y,
    int visualDeg,
    int xProjRes, int yProjRes,
    int slope,
    double projCenter) noexcept :
    m_x(x),
    m_y(y),
    m_alpha(0),
    m_visualDeg(visualDeg),
    m_xProjRes(xProjRes),
    m_yProjRes(yProjRes),
    m_slope(slope),
    m_projCenter(projCenter)
{
    m_floorShadingPar = yProjRes / 16;

    const int vecSize = m_xProjRes * (360 / m_visualDeg);

    m_degVisual = (vecSize * m_visualDeg) / 360;
    m_degVisual2 = m_degVisual / 2;

    m_deg90 = vecSize / 4;
    m_deg180 = vecSize / 2;
    m_deg270 = (vecSize / 4) * 3;
    m_deg360 = vecSize - 1;

    m_cosTbl.resize(vecSize);
    m_sinTbl.resize(vecSize);
    m_tanTbl.resize(vecSize);
    m_invSinTbl.resize(vecSize);
    m_invCosTbl.resize(vecSize);
    m_invTanTbl.resize(vecSize);

    for (int ray = 0; ray < vecSize; ++ray) {
        const double alpha = (double(ray*360.0) / double(m_deg360))*(3.14159265359 / 180.0);

        m_cosTbl[ray] = ::cos(alpha);
        m_sinTbl[ray] = ::sin(alpha);
        m_tanTbl[ray] = ::tan(alpha);

        m_invCosTbl[ray] = fabs(m_cosTbl[ray]) <= SMALLEST_EPSILON ?
            sign(m_cosTbl[ray]) * POSITIVE_INFINITY :
            double(1.0) / m_cosTbl[ray];

        m_invSinTbl[ray] = fabs(m_sinTbl[ray]) <= SMALLEST_EPSILON ?
            sign(m_cosTbl[ray]) * POSITIVE_INFINITY :
            double(1.0) / m_sinTbl[ray];

        m_invTanTbl[ray] = fabs(m_tanTbl[ray]) <= SMALLEST_EPSILON ?
            sign(m_cosTbl[ray]) * POSITIVE_INFINITY :
            double(1.0) / m_tanTbl[ray];
    }

}


/* -------------------------------------------------------------------------- */
// WORLDMAP
/* -------------------------------------------------------------------------- */

bool WorldMap::loadMapInfo(const cell_t* array, uint32_t rows, uint32_t cols)
{
    if (rows <= 0 || cols <= 0) {
        return false;
    }

    m_map.resize(rows);

    int i = 0;

    for (uint32_t r = 0; r < rows; ++r) {
        m_map[r].resize(cols);

        for (uint32_t c = 0; c < cols; ++c) {
            m_map[r][c] = array[i++];
        }
    }

    m_maxX = getCellDx() * getColCount();
    m_maxY = getCellDy() * getRowCount();

    return true; // success
}


/* -------------------------------------------------------------------------- */

