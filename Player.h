// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#ifndef __PLAYER_H__
#define __PLAYER_H__


/* -------------------------------------------------------------------------- */

#include "BitmapBuffer.h"

#include <math.h>
#include <map>
#include <vector>

class WorldMap;


/* -------------------------------------------------------------------------- */

class Player
{
private:
    int m_visualDeg;
    int m_xProjRes;
    int m_yProjRes;
    int m_slope;
    int m_floorShadingPar;
    int m_deg90;
    int m_deg180;
    int m_deg270;
    int m_deg360;
    int m_degVisual;
    int m_degVisual2;
    double m_projCenter;

    std::vector<double> m_cosTbl;
    std::vector<double> m_sinTbl;
    std::vector<double> m_tanTbl;
    std::vector<double> m_invSinTbl;
    std::vector<double> m_invCosTbl;
    std::vector<double> m_invTanTbl;

    double m_x, m_y;
    int m_alpha;

public:
    using Point2d = std::pair<double, double>;

    using Cell = uint64_t;

    Player(int x = 0,
        int y = 0,
        int visualDeg = 60,
        int xProjRes = 320,
        int yProjRes = 200,
        int slope = 0,
        double m_projCenter = double(0.5)) noexcept;

    int deg90() const noexcept { 
        return m_deg90; 
    }

    int deg180() const noexcept { 
        return m_deg180; 
    }

    int deg270() const noexcept { 
        return m_deg270; 
    }

    int deg360() const noexcept {
        return m_deg360;
    }

    int degHalfVisual() const noexcept { 
        return m_degVisual2; 
    }

    const double& tan(int ray) const noexcept { 
        return m_tanTbl[ray]; 
    }

    const double& cos(int ray) const noexcept { 
        return m_cosTbl[ray]; 
    }

    const double& sin(int ray) const noexcept { 
        return m_sinTbl[ray]; 
    }

    const double& invsin(int ray) const noexcept { 
        return m_invSinTbl[ray]; 
    }

    const double& invcos(int ray) const noexcept { 
        return m_invCosTbl[ray]; 
    }

    const double& invtan(int ray) const noexcept { 
        return m_invTanTbl[ray]; 
    }

    int getX() const noexcept { 
        return int(m_x); 
    }

    int getY() const noexcept { 
        return int(m_y); 
    }

    int getCol(int cellDx) const noexcept { 
        return int(m_x / cellDx); 
    }

    int getRow(int cellDy) const noexcept { 
        return int(m_y / cellDy); 
    }

    void setPos(const Point2d& position) noexcept {
        m_x = position.first; 
        m_y = position.second;
    }

    Cell Player::moveToH(int offset, WorldMap& wMap) {
        return moveTo(offset, wMap, -m_deg90);
    }

    Cell moveTo(int offset, WorldMap& wMap, int deg = 0);

    int getAlpha() const noexcept { 
        return m_alpha; 
    }

    void setAlpha(int alpha) noexcept {
        m_alpha = alpha;
        
        if (m_alpha <= 0) {
            m_alpha += m_deg360;
        }
        
        if (m_alpha >= m_deg360) {
            m_alpha -= m_deg360;
        }
    }

    void rotate(double rad) noexcept { 
        setAlpha(int(m_alpha + rad)); 
    }

    int getSlope() const noexcept { 
        return m_slope; 
    }

    void setSlope(int slope) noexcept { 
        m_slope = slope; 
    }

    int getXProjRes() const noexcept { 
        return m_xProjRes; 
    }

    int getYProjRes() const noexcept { 
        return m_yProjRes; 
    }

    double getCenterProj() const noexcept { 
        return m_projCenter; 
    }

    void setCenterProj(double projCenter) noexcept {
        m_projCenter = projCenter;
    }
};


/* -------------------------------------------------------------------------- */

#endif
