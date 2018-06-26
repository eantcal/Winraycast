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

#include "WorldMap.h"


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

