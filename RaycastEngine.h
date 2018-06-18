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

#include <windows.h>
#pragma warning (disable: 4786)
#include <math.h>
#include <map>
#include <vector>


/* -------------------------------------------------------------------------- */

#include <ddraw.h>  
extern LPDIRECTDRAWSURFACE7 g_pDDSBack;


using cell_t=uint64_t;

class WorldMap;


/* -------------------------------------------------------------------------- */

using fpoint_t = std::pair<double, double>;


/* -------------------------------------------------------------------------- */

class Camera
{
private:
    int  m_visualDeg,
        m_xProjRes,
        m_yProjRes,
        m_slope,
        m_floorShadingPar;

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
    Camera(int x = 0,
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

    void setPos(const fpoint_t& position) noexcept {
        m_x = position.first; 
        m_y = position.second;
    }

    cell_t Camera::moveToH(int offset, WorldMap& wMap) {
        return moveTo(offset, wMap, -m_deg90);
    }

    cell_t moveTo(int offset, WorldMap& wMap, int deg = 0);

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

private:
    
};


/* -------------------------------------------------------------------------- */

class TextureBuffer {
public:
    virtual ~TextureBuffer() { delete[] _bitmap; }

    TextureBuffer(HDC hdc, HBITMAP hBitmap, int dx, int dy) : 
        m_dx(dx), m_dy(dy) 
    {
        HDC texture_hdc = CreateCompatibleDC(hdc);

        SelectObject(texture_hdc, hBitmap);

        BITMAPINFO BmpInfo;

        memset((void*)&BmpInfo, 0, sizeof(BITMAPINFOHEADER));
        BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        BmpInfo.bmiHeader.biWidth = dx;
        BmpInfo.bmiHeader.biHeight = -dy;

        BmpInfo.bmiHeader.biPlanes = 1;
        BmpInfo.bmiHeader.biBitCount = 32;
        BmpInfo.bmiHeader.biCompression = BI_RGB;
        BmpInfo.bmiHeader.biClrUsed = 0;
        BmpInfo.bmiHeader.biClrImportant = 0;

        _bitmap = new DWORD[dx*dy];

        GetDIBits(texture_hdc, hBitmap, 0, dy, (LPVOID)_bitmap, &BmpInfo, DIB_RGB_COLORS);

        DeleteDC(texture_hdc);
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
                (*((DWORD*)destBuf + x + y * m_dx)) = getPixel((x + offset) % org_dx, y);
            }
        }
    }

private:
    DWORD * _bitmap;
    int m_dx, m_dy;
};


/* -------------------------------------------------------------------------- */

class WorldMap
{
private:
    using row_t = std::vector<cell_t>;
    using matrix_t = std::vector<row_t>;

    matrix_t m_map;
    Camera m_camera;

    int m_cellDx = 256;
    int m_cellDy = 256;

    int m_maxX = 0;
    int m_maxY = 0;

    fpoint_t m_cameraCellPos{0,0};

    HBITMAP m_bmp[256] = { 0 };

public:
    WorldMap() = default;

    HBITMAP getBmp(int key) const noexcept { 
        return m_bmp[key]; 
    }

    const fpoint_t& get_camera_cell_pos() const noexcept { 
        return m_cameraCellPos; 
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

    std::vector<cell_t> & operator[](uint32_t index) throw () { 
        return m_map[index]; 
    }

    const std::vector<cell_t> & operator[](uint32_t index) const throw () {
        return m_map[index];
    }

    bool loadMapInfo(const cell_t* array, uint32_t rows, uint32_t cols);

    void resizeCell(uint32_t cellDx, uint32_t cellDy) noexcept {
        m_cellDx = cellDx;
        m_cellDy = cellDy;
        m_maxX = getCellDx() * getColCount();
        m_maxY = getCellDy() * getRowCount();
    }

    void setCamera(const Camera& camera) noexcept {
        m_camera = camera;
        m_cameraCellPos.first = camera.getX() / getCellDx();
        m_cameraCellPos.second = camera.getY() / getCellDy();
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

    void set(int row, int col, cell_t cellVal) {
        if (col < getColCount() && row < getRowCount())
            m_map[row][col] = cellVal;
    }

};


/* -------------------------------------------------------------------------- */

class RaycastEngine
{
public:
    RaycastEngine(Camera& camera, double scale) :
        m_scale(scale),
        m_camera(camera)
    {
        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    ~RaycastEngine() {
        if (!g_pDDSBack) {
            delete[] m_videoBuf;
        }
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

    Camera& camera() { 
        return m_camera; 
    }

    const Camera& camera() const noexcept {
        return m_camera;
    }

private:
    void renderTranspWall(int videoPosX,
        int videoPosY,
        HDC videoHdc,
        WorldMap& aMap,
        const RECT& rt,
        bool render_internal_wall);

    Camera m_camera;
    BYTE* m_videoBuf = nullptr;

    double getM(int ray) noexcept {
        return(m_camera.tan(ray));
    }
    
    double getM1(int ray) noexcept {
        return (m_camera.invtan(ray));
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
        return (h_inter.second - m_camera.getY()) *m_camera.invsin(ray);
    }

    double vertDist(const fpoint_t& v_inter, int ray) noexcept {
        return (v_inter.first - m_camera.getX()) *m_camera.invcos(ray);
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

    int m_fps = 0;
};

#endif