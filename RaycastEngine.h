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

#ifndef __RAYCASTENGINE_H__
#define __RAYCASTENGINE_H__

#include "BitmapBuffer.h"
#include "WorldMap.h"
#include "Player.h"

#include <windows.h>
#pragma warning (disable: 4786)
#include <math.h>
#include <map>
#include <vector>


/* -------------------------------------------------------------------------- */

using cell_t=uint64_t;

class WorldMap;


/* -------------------------------------------------------------------------- */

using fpoint_t = std::pair<double, double>;


/* -------------------------------------------------------------------------- */

class RaycastEngine
{
public:
    RaycastEngine(Player& player, double scale) :
        m_scale(scale),
        m_player(player)
    {
        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    ~RaycastEngine() {
        delete[] m_videoBuf;
    }

    void setShadingBrighter() noexcept {
        m_depthShadingPar /= 1.1;

        if (m_depthShadingPar < 1.0) {
            m_depthShadingPar = 1.0;
        }

        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    void setShadingDarker() noexcept {
        m_depthShadingPar *= 1.1;
        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    double getDepthShadingLevel() const noexcept {
        return m_depthShadingPar;
    }

    void setDepthShadingLevel(double level) noexcept {
        m_depthShadingPar = level;
        
        if (m_depthShadingPar < 1.0) {
            m_depthShadingPar = 1.0;
        }

        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    void renderScene(
        int videoPosX,
        int videoPosY,
        HDC videoHdc,
        WorldMap& aMap,
        const RECT& rt);

    Player& player() { 
        return m_player; 
    }

    const Player& player() const noexcept {
        return m_player;
    }

private:

     void DDrawPixel32(BYTE* surface, unsigned int x, unsigned int y, DWORD color_value) {
        if (y < m_renderAreaHeight && x < m_renderAreaWidth) {
            *((DWORD*)(surface + (x << 2) + (y*m_renderPitch))) = color_value;
        }
    }


    void renderTranspWall(int videoPosX,
        int videoPosY,
        HDC videoHdc,
        WorldMap& aMap,
        const RECT& rt,
        bool render_internal_wall);

    Player m_player;
    BYTE* m_videoBuf = nullptr;

    double getM(int ray) noexcept {
        return(m_player.tan(ray));
    }
    
    double getM1(int ray) noexcept {
        return (m_player.invtan(ray));
    }

    void vertint1st(
        WorldMap& map, 
        int ray, 
        const double& M, 
        fpoint_t& point) const noexcept;

    void horzint1st(
        WorldMap& map, 
        int ray, 
        const double& M1, 
        fpoint_t& point) const noexcept;

    void vertint(
        WorldMap& map, 
        const fpoint_t& firstInt, 
        int ray, 
        const double& M, 
        fpoint_t& point) const noexcept;

    void horzint(
        WorldMap& map, 
        const fpoint_t& firstInt, 
        int ray, 
        const double& M1, 
        fpoint_t& point) const noexcept;

    double horzDist(const fpoint_t& h_inter, int ray) noexcept {
        return (h_inter.second - m_player.getY()) *m_player.invsin(ray);
    }

    double vertDist(const fpoint_t& v_inter, int ray) noexcept {
        return (v_inter.first - m_player.getX()) *m_player.invcos(ray);
    }

    bool isInClientRect(const WorldMap& map, const fpoint_t& point) const noexcept {
        return point.first >= 0 && point.first <= map.getMaxX() &&
            point.second >= 0 && point.second <= map.getMaxY();
    }

    cell_t horzWall(WorldMap& map, const fpoint_t& point, int ray) const noexcept;
    cell_t vertWall(WorldMap& map, const fpoint_t& point, int ray) const noexcept;

    cell_t horzIntWall(WorldMap& map, const fpoint_t& point, int ray) const noexcept;
    cell_t vertIntWall(WorldMap& map, const fpoint_t& point, int ray) const noexcept;

    void shadingStretchBtl(
        HDC dest_hdc, int xDest, int yDest,
        int heightDest,
        int xSrc, int ySrc,
        int height_source, int widthSrc,
        int maxVisibleY, double depthPar, HBITMAP hBitmap
    );

    void transpShadingStretchBtl(
        HDC dest_hdc,
        int xDest, int yDest,
        int heightDest,
        int xSrc, int ySrc,
        int height_source, int widthSrc,
        int maxVisibleY, double depthPar, HBITMAP hBitmap,
        int transpC
    );

private:
    double m_scale = 0;
    double m_depthShadingPar = 100.0;
    double m_ceilFloorShadingPar = 0;

    DWORD m_renderAreaHeight = 0;
    DWORD m_renderAreaWidth = 0;
    DWORD m_renderPitch = 0;

    int m_fps = 0;
};

#endif