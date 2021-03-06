// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.


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

using Cell=uint64_t;

class WorldMap;


/* -------------------------------------------------------------------------- */

using Point2d = std::pair<double, double>;


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
        Point2d& point) const noexcept;

    void horzint1st(
        WorldMap& map, 
        int ray, 
        const double& M1, 
        Point2d& point) const noexcept;

    void vertint(
        WorldMap& map, 
        const Point2d& firstInt, 
        int ray, 
        const double& M, 
        Point2d& point) const noexcept;

    void horzint(
        WorldMap& map, 
        const Point2d& firstInt, 
        int ray, 
        const double& M1, 
        Point2d& point) const noexcept;

    double horzDist(const Point2d& h_inter, int ray) noexcept {
        return (h_inter.second - m_player.getY()) *m_player.invsin(ray);
    }

    double vertDist(const Point2d& v_inter, int ray) noexcept {
        return (v_inter.first - m_player.getX()) *m_player.invcos(ray);
    }

    bool isInClientRect(const WorldMap& map, const Point2d& point) const noexcept {
        return point.first >= 0 && point.first <= map.getMaxX() &&
            point.second >= 0 && point.second <= map.getMaxY();
    }

    Cell horzWall(WorldMap& map, const Point2d& point, int ray) const noexcept;
    Cell vertWall(WorldMap& map, const Point2d& point, int ray) const noexcept;

    Cell horzIntWall(WorldMap& map, const Point2d& point, int ray) const noexcept;
    Cell vertIntWall(WorldMap& map, const Point2d& point, int ray) const noexcept;

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
