// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#include "Player.h"
#include "WorldMap.h"


/* -------------------------------------------------------------------------- */

Player::Cell Player::moveTo(int offset, WorldMap& wMap, int deg)
{
    Cell retVal = 0;

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

static const double POSITIVE_INFINITY = 1000000.0;
static const double SMALLEST_EPSILON = double(1.0) / POSITIVE_INFINITY;


/* -------------------------------------------------------------------------- */

Player::Player(
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
    auto sign = [](double x) {
        return x == 0.0 ? 0.0 : (x>.0 ? 1. : -1.);
    };

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

