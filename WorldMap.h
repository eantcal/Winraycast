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

#ifndef __WORLDMAP_H__
#define __WORLDMAP_H__

#include "BitmapBuffer.h"
#include "Player.h"

#include <windows.h>
#pragma warning (disable: 4786)
#include <math.h>
#include <map>
#include <vector>


/* -------------------------------------------------------------------------- */

class WorldMap
{
public:
    using Cell = uint64_t;
    using Point2d = std::pair<double, double>;
    using TextureList = std::map<std::string, std::string>;

    WorldMap() = default;

    HBITMAP getBmp(int key) const noexcept { 
        return m_bmp[key]; 
    }

    const Point2d& getPlayerCellPos() const noexcept { 
        return m_playerCellPos; 
    }

    int getRowCount() const noexcept { 
        return int(m_map.size()); 
    }

    int getColCount() const noexcept { 
        return int(m_map.empty() ? 0 : m_map[0].size()); 
    }

    uint32_t getCellDx() const noexcept { 
        return m_cellDx; 
    }

    uint32_t getCellDy() const noexcept { 
        return m_cellDy; 
    }

    std::vector<Cell> & operator[](uint32_t index) throw () { 
        return m_map[index]; 
    }

    const std::vector<Cell> & operator[](uint32_t index) const throw () {
        return m_map[index];
    }


    void resizeCell(uint32_t cellDx, uint32_t cellDy) noexcept {
        m_cellDx = cellDx;
        m_cellDy = cellDy;
        m_maxX = getCellDx() * getColCount();
        m_maxY = getCellDy() * getRowCount();
    }

    void setPlayerPos(int x, int y) noexcept {
        m_playerCellPos.first = /*player.getX()*/ x / getCellDx();
        m_playerCellPos.second = /*player.getY()*/ y / getCellDy();
    }

    void applyTextureToPanel(int panelKey, HBITMAP hBitmap) noexcept {
        m_bmp[panelKey & 0xff] = hBitmap;
    }

    int getMaxX() const noexcept { 
        return m_maxX; 
    }

    int getMaxY() const noexcept { 
        return m_maxY; 
    }

    void set(int row, int col, Cell cellVal) {
        if (col < getColCount() && row < getRowCount())
            m_map[row][col] = cellVal;
    }

    bool load(const std::string& fileName);

    const TextureList& getTextureList() const noexcept {
        return m_textureList;
    }

    TextureList& getTextureList() noexcept {
        return m_textureList;
    }

private:
    bool setMapInfo(const Cell* array, uint32_t rows, uint32_t cols);

    using Row = std::vector<Cell>;
    using Matrix = std::vector<Row>;

    Matrix m_map;

    int m_cellDx = 256;
    int m_cellDy = 256;

    int m_maxX = 0;
    int m_maxY = 0;

    Point2d m_playerCellPos{ 0,0 };

    HBITMAP m_bmp[256] = { 0 };

    TextureList m_textureList;
};


/* -------------------------------------------------------------------------- */

#endif