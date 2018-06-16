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

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>

#include <ddraw.h>
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

#include <string>


/* -------------------------------------------------------------------------- */

using namespace std;


/* -------------------------------------------------------------------------- */

LPDIRECTDRAW7        g_pDD = NULL;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_pDDSPrimary = NULL;// DirectDraw primary surface
LPDIRECTDRAWSURFACE7 g_pDDSBack = NULL;   // DirectDraw back surface
LPDIRECTDRAWCLIPPER  g_pClipper = NULL;   // Clipper for primary

/* -------------------------------------------------------------------------- */

static HRESULT DbgTrace(HWND hWnd, HRESULT hRet, LPCTSTR szError, ...);
static HRESULT InitInstanceDD7(HINSTANCE hInstance, int nCmdShow);
static void ReleaseAllObjects(void);

/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include "resource.h"
#include "RaycastEngine.h"

/* -------------------------------------------------------------------------- */

#define CELL_SIZE    256
#define VISUAL_DEGREE 60 

#define KEYBSTEP  10
#define KEYBALPHA  4

#define X_RES 900
#define Y_RES 900

#define PROJ_X_RES 512
#define PROJ_Y_RES 512

#define FIRE_EFFECT  1
#define WATER_EFFECT 2
#define LIGHT_EFFECT 3
#define FPS_INFO     4

#define SCALE 90000

#define MAX_LOADSTRING 100
#define FULL_SCREEN_MODE TRUE

#define CAMERA_CEL_COL_POS 5
#define CAMERA_CEL_ROW_POS 60

#define MAP_ROWS 64 
#define MAP_COLS 16


/* -------------------------------------------------------------------------- */

// Global Variables:
HINSTANCE g_hInstance;                // current instance
TCHAR g_szAppTitle[] = "WinRayCast";
TCHAR g_szAppWinClass[] = "WINRAYCAST";
HWND g_hWnd;


/* -------------------------------------------------------------------------- */

// Foward declarations of functions included in this code module:
static ATOM WRCstRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK  WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  About(HWND, UINT, WPARAM, LPARAM);

static void MovePlayer();
static void Render3DEnvironment();

static bool g_FullScreenModeActive = false;
static BOOL g_bActive = FALSE;   // Is application active?
static int  g_current_cell_of_player = 0;

WorldMap*      theWorldMap = 0;
RaycastEngine* the3DEngine = 0;


/* -------------------------------------------------------------------------- */

static
void ChangeToFullScreen()
{
    DEVMODE dmSettings;
    memset(&dmSettings, 0, sizeof(dmSettings));

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings)) {
        MessageBox(NULL, "Could Not Enum Display Settings", "Error", MB_OK);
        return;
    }

    dmSettings.dmPelsWidth = X_RES;
    dmSettings.dmPelsHeight = Y_RES;

    int result = ChangeDisplaySettings(&dmSettings, CDS_FULLSCREEN);

    if (result != DISP_CHANGE_SUCCESSFUL) {
        MessageBox(NULL, "Display Mode Not Compatible", "Error", MB_OK);
    }
}


/* -------------------------------------------------------------------------- */

static
void Setup3DEngine(RaycastEngine** the3DEngine, WorldMap** theWorldMap)
{
    const int map_rows = MAP_ROWS;
    const int map_cols = MAP_COLS;

    *theWorldMap = new WorldMap();

    Camera aCamera = Camera(0, 0, VISUAL_DEGREE, PROJ_X_RES, PROJ_Y_RES);
    aCamera.setPos(
        make_pair<int, int>(CELL_SIZE * CAMERA_CEL_COL_POS,
            CELL_SIZE * CAMERA_CEL_ROW_POS)
    );

    // Floor/Ceil/Wall MAP
    unsigned int array_map[map_rows][map_cols] = {
        0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x97000100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x00000002,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x00f1c100,0x0000c100,0x0000c100,0x00020200,0x00000000,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x00000002,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000004b,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000009,0x00000101,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000101,
        0x00000009,0x0000c100,0x00000002,0x0000002b,0x0000c100,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000009,0x00000101,0x96000100,0x00000100,0x96000100,0x00000100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x00000002,0x00000002,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000009,0x00000101,0x00000100,0x00000100,0x00000100,0x00000100,0x00000101,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000005,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00000005,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x00000009,0x0000c100,0x00000009,0x00000009,0x00000009,0x00fc0300,0x00000003,

        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000009,0x00000002,0x00000009,0x00000004,0x00000004,0x0000c100,0x00000004,0x00000004,0x00000003,0x0000003b,0x00000003,
        0x0000009c,0x00f1c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x00000004,0x0000c100,0x00000600,0x0000c100,0x0000c100,0x00000003,0x00fe0300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x00000004,0x00006b00,0x0000c100,0x00006b00,0x0000c100,0x00000300,0x00000300,0x00000003,
        0x00000009,0x00f1c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x0000004d,0x00f1c100,0x00000600,0x0000c100,0x00000004,0x00000003,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x0000004c,0x00006b00,0x0000c100,0x00006b00,0x00000004,0x00000003,0x00000300,0x00000003,
        0x0000009b,0x0000c100,0x0000c100,0x0000009b,0x00000009,0x00000002,0x00000002,0x00000009,0x0000004e,0x0000c100,0x00000600,0x0000c100,0x00000004,0x00000003,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x00000004,0x00006b00,0x00f1c100,0x00006b00,0x00000004,0x00000003,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x00000004,0x0000c100,0x00000600,0x00000004,0x0000003c,0x003d0300,0x00000300,0x00000003,
        0x0000009b,0x0000c100,0x0000c100,0x0000009b,0x00000009,0x00000002,0x00000002,0x00000009,0x00000004,0x00006b00,0x0000c100,0x00006b00,0x00000004,0x00000003,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000009,0x00000004,0x00000004,0x00000004,0x00000004,0x00000004,0x00000003,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x00000002,0x00000002,0x00000002,0x00000002,0x00000004,0x00000004,0x00000004,0x00000003,0x00000300,0x00000003,
        0x0000009b,0x0000c100,0x0000c100,0x0000009b,0x00000009,0x00000002,0x00000002,0x0000c100,0x0000c100,0x0000c100,0x00000003,0x00000003,0x00000003,0x00000003,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x0000c100,0x0000c100,0x00000002,0x0000c100,0x00000300,0x00000300,0x00000300,0x00000300,0x00000300,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x00000009,0x00000009,0x00000002,0x0000c100,0x00fcc100,0x0000002c,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,
        0x00000009,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x0000c100,0x00fdc100,0x0000002d,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,
        0x00000009,0x00000007,0x0000007b,0x00000009,0x00000009,0x00000002,0x0000c100,0x00000002,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,

        0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x0000c100,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000001,0x00000001,0x00000001,
        0x00000101,0x0000ff00,0x0000ff00,0x00000101,0x0000ff00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x0000ff00,0x0000ff00,0x0000ff00,0x0000ff00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x0000ff00,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000000,0x00000101,0x00000101,0x00000108,0x0000FF00,0x0010ff00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000000,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000110,0x00101000,0x00000101,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x0000FF00,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x000Aff00,0x00000001,
        0x00000101,0x00000101,0x00000101,0x97010100,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,0x00000101,

        0x00000090,0x00000090,0x00000090,0x00909000,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000101,0x00000101,0x00000101,0x00000101,
        0x00000090,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x00000000,0x00000000,0x00000000,0x00000101,
        0x00000088,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x00000101,0xea0Aff00,0x00000101,0x00000101,
        0x00000089,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000090,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x910000FF,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000090,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x910000ff,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000090,0x00909000,0x00000090,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000090,0x00909000,0x00000090,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00959000,0x97010100,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000090,0x00000090,0x00000090,0x00909000,0x00909000,0x00909000,0x00909000,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000087,0x00909000,0x00000090,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x910000ff,0x00909000,0x00000090,0x92909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000089,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000090,0x00909000,0x00909000,0x00909000,0x00909000,0x00909000,0x00000090,0x00959000,0x00959000,0x00959000,0x00000090,0x00000101,0x000Aff00,0x000Aff00,0x000Aff00,0x00000101,
        0x00000001,0x00000090,0x00000093,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000090,0x00000001,0x00000101,0x00000101,0x00000101,0x00000101,

    };

    (*theWorldMap)->loadMapInfo((const unsigned int*)array_map, map_rows, map_cols);
    (*theWorldMap)->resizeCell(CELL_SIZE, CELL_SIZE);

    *the3DEngine = new RaycastEngine(aCamera, SCALE);
}


/* -------------------------------------------------------------------------- */

inline
HBITMAP loadBMP(const char* image,
    const int dx = CELL_SIZE,
    const int dy = CELL_SIZE,
    const char* ext = ".WRC")
{
    string image_name = image;
    image_name += ext;

    return (HBITMAP)
        LoadImage(g_hInstance,
            image_name.c_str(),
            IMAGE_BITMAP,
            dx, dy,
            LR_LOADFROMFILE);
}


/* -------------------------------------------------------------------------- */

int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;

#if 0
    g_FullScreenModeActive =
        IDYES == MessageBox(0,
            "Go to in full screen mode ?",
            "Video Mode",
            MB_ICONQUESTION | MB_YESNO);
#endif
    if (InitInstanceDD7(hInstance, nCmdShow) != DD_OK) {
        return FALSE;
    }

    SetTimer(g_hWnd, WATER_EFFECT, 220, NULL);
    SetTimer(g_hWnd, LIGHT_EFFECT, 120, NULL);
    SetTimer(g_hWnd, FIRE_EFFECT, 100, NULL);
    SetTimer(g_hWnd, FPS_INFO, 4000, NULL);


    g_hInstance = hInstance;

    Setup3DEngine(&the3DEngine, &theWorldMap);

    struct cellTextureId_bmpName_t {
        unsigned int txtId;
        const char * txtFileName;
    };

    cellTextureId_bmpName_t cellTxtTable[] = {
    { 0xA1, "eff00027" },
    { 0xA2, "eff00028" },
    { 0xA3, "eff00029" },
    { 0xA4, "eff00030" },
    { 0xE0, "eff00031" },
    { 0xE1, "eff00032" },
    { 0xE2, "eff00033" },
    { 0xE3, "eff00034" },
    { 0xb1, "water1" },
    { 0xb2, "water2" },
    { 0xb3, "water3" },
    { 0xb4, "water4" },
    { 0xb5, "water5" },
    { 0xb6, "water6" },
    { 0x1b, "w1b" },
    { 0xf1, "f1b" },
    { 0xfc, "fc" },
    { 0xfd, "fd" },
    { 0xfe, "fe" },
    { 0x02, "w2" },
    { 0x2b, "w2b" },
    { 0x2c, "w2c" },
    { 0x2d, "w2d" },
    { 0x03, "w3" },
    { 0x3b, "w3b" },
    { 0x3c, "w3c" },
    { 0x3d, "w3d" },
    { 0x04, "w4" },
    { 0x4b, "w4b" },
    { 0x4c, "w4c" },
    { 0x4d, "w4d" },
    { 0x4e, "w4e" },
    { 0x05, "o2w1" },
    { 0x06, "w6" },
    { 0x6b, "w6b" },
    { 0x07, "w7" },
    { 0x7B, "w7b" },
    { 0x08, "w8" },
    { 0x09, "w9" },
    { 0x9b, "w9b" },
    { 0x9c, "w9c" },
    { 0xEE, "tr1" },
    { 0xEA, "tr2" },
    { 0x0A, "block1" },
    { 0xc1, "c1" },
    { 0xc2, "c1b" },
    { 0x01,  "w1" },
    { 0x00,  "f1" },
    { 0xf0, "oc1" },
    { 0x10, "f0" },
    { 0x91, "wtr1" },
    { 0x90, "wtr2" },
    { 0x92, "tr3" },
    { 0x93, "wtr3" },
    { 0x94, "wtr4" },
    { 0x95, "wtr5" },
    { 0x96, "wtr6" },
    { 0x87, "wtr7" },
    { 0x88, "wtr8" },
    { 0x89, "wtr9" },
    { 0x97, "w1p" },
    };

    const int cellTxtTable_item_count = sizeof(cellTxtTable) / sizeof(cellTextureId_bmpName_t);
    for (int i = 0; i < cellTxtTable_item_count; ++i) {
        theWorldMap->applyTextureToPanel(
            cellTxtTable[i].txtId,
            loadBMP(cellTxtTable[i].txtFileName)
        );
    }

#define SKY_BMP_RESOURCE "clouds2"

    theWorldMap->applyTextureToPanel(
        SKY_PANEL_MASK,
        loadBMP(SKY_BMP_RESOURCE, PROJ_X_RES, PROJ_Y_RES)
    );

    g_bActive = TRUE;

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WINRAYCAST);

    BOOL bGotMsg;

    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

    while (WM_QUIT != msg.message) {
        // Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        MovePlayer();

        bGotMsg = g_bActive ?
            PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) :
            GetMessage(&msg, NULL, 0U, 0U);

        if (bGotMsg) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // Render a frame during idle time (no messages are waiting)
            if (g_bActive) {
                Render3DEnvironment();
            }
        }
    }

    return msg.wParam;
}


/* -------------------------------------------------------------------------- */

static
ATOM WRCstRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_WINRAYCAST);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = 0; //(HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = g_FullScreenModeActive ? 0 : (LPCSTR)IDC_WINRAYCAST;
    wcex.lpszClassName = g_szAppWinClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
}


/* -------------------------------------------------------------------------- */

static
void MovePlayer()
{
    BOOL shift_pressed = GetAsyncKeyState(VK_LSHIFT);
    int speedFact = 1;

    if (shift_pressed) {
        speedFact = 2;
        if (GetAsyncKeyState(VK_LEFT)) {
            g_current_cell_of_player =
                the3DEngine->camera().hmove(KEYBSTEP, *theWorldMap);
        }
        else if (GetAsyncKeyState(VK_RIGHT)) {
            g_current_cell_of_player =
                the3DEngine->camera().hmove(-KEYBSTEP, *theWorldMap);
        }
    }
    else {
        if (GetAsyncKeyState(VK_LEFT)) {
            the3DEngine->camera().rotate(-KEYBALPHA);
        }
        else if (GetAsyncKeyState(VK_RIGHT)) {
            the3DEngine->camera().rotate(KEYBALPHA);
        }
    }

    bool move_up_down = false;

    if (GetAsyncKeyState(VK_UP)) {
        g_current_cell_of_player =
            the3DEngine->camera().move(KEYBSTEP*speedFact, *theWorldMap);

        move_up_down = true;
    }
    else if (GetAsyncKeyState(VK_DOWN)) {
        g_current_cell_of_player =
            the3DEngine->camera().move(-KEYBSTEP * speedFact, *theWorldMap);

        move_up_down = true;
    }

    if (GetAsyncKeyState(VK_PRIOR)) {
        the3DEngine->camera().setSlope(
            the3DEngine->camera().getSlope() + KEYBSTEP);
    }
    else if (GetAsyncKeyState(VK_NEXT)) {
        the3DEngine->camera().setSlope(
            the3DEngine->camera().getSlope() - KEYBSTEP);
    }

    if (GetAsyncKeyState(VK_END)) {
        the3DEngine->camera().setCenterProj(double(0.90));
    }
    else if (GetAsyncKeyState(VK_HOME)) {
        the3DEngine->camera().setCenterProj(double(0.10));
    }
}


/* -------------------------------------------------------------------------- */

static
void Render3DEnvironment() {
    static HDC hdc = GetDC(g_hWnd);

    static RECT rt, wrt;

    int cxBorder = 0;
    int cyBorder = 0;
    int cCaption = 0;

    if (g_FullScreenModeActive) {
        GetClientRect(g_hWnd, &rt);
    }
    else {
        GetWindowRect(g_hWnd, &wrt);

        cxBorder = GetSystemMetrics(SM_CXBORDER);
        cyBorder = GetSystemMetrics(SM_CXBORDER);
        cCaption = GetSystemMetrics(SM_CYSIZE);

        rt.right = wrt.right - wrt.left - cxBorder;
        rt.bottom = wrt.bottom - wrt.top - cyBorder - cCaption;
    }

    if (the3DEngine) {
        the3DEngine->renderScene(wrt.left + cxBorder,
            wrt.top + cyBorder + cCaption,
            hdc,
            *theWorldMap,
            rt);
    }
}


/* -------------------------------------------------------------------------- */

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC videoHdc, hdc = 0;

#ifdef SHOW_FPS
    static int FPS = 0;
#endif

    switch (message) {
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case ID_FILE_INFO: {
            char info[256] = { 0 };
            sprintf(
                info,
                "X_RES = %i\r\n"
                "Y_RES = %i\r\n"
                "PROJ_X_RES = %i\r\n"
                "PROJ_Y_RES = %i\r\n"
                "VISUAL_DEGREE = %i\r\n"
                "Direct Draw 7 MODE\r\n"
                , X_RES, Y_RES, PROJ_X_RES, PROJ_Y_RES, VISUAL_DEGREE
            );
            MessageBox(hWnd, info, g_szAppTitle, 0);
        }
        break;

        case IDM_ABOUT:
        case ID_FILE_ABOUT:
            DialogBox(g_hInstance, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_TIMER: {
        if (!the3DEngine)
            break;

        switch (wParam) {
        case FIRE_EFFECT: {
            static int i = 0;
            //fireplace
            i = (rand() & 0x3) ^ (i & 1);
            (*theWorldMap)[3][6] = i + 0xA1;
            (*theWorldMap)[32][6] = ((i + 0xE0) << 24) | ((i + 0xE0) << 16) | ((0x1) << 8);
        }
        break;

        case WATER_EFFECT: {
            //water
            static int j = 0;
            j++;

            int water_effect = ((j % 6 + 0xB1) << 16) & 0x00FF0000;

            (*theWorldMap)[8][5] = water_effect | 0xee00FF00;
            for (int x = 0; x < 4; ++x) {
                for (int y = 0; y < 10; ++y) {
                    (*theWorldMap)[1 + y][11 + x] = (x & y & 1) ? 0x00000101 : 0x0000FF00 | water_effect;
                    (*theWorldMap)[33 + y][11 + x] = (x & y & 1) ? 0x00000505 : 0x0000FF00 | water_effect;
                }
            }

            int floor_type = g_current_cell_of_player >> 16;
            if (floor_type <= 0xB6 && floor_type >= 0xB1) {
                the3DEngine->camera().setCenterProj((double)0.90);
            }
            else {
                the3DEngine->camera().setCenterProj((double)0.50);
            }

            if (floor_type == 0x10) {
                the3DEngine->camera().setPos(make_pair<int, int>(CELL_SIZE * 2, CELL_SIZE * 2));
            }
        }
        break;

        case LIGHT_EFFECT: {
            int y = the3DEngine->camera().getRow(CELL_SIZE);
            if (y > 31) {
                the3DEngine->setDepthShadingLevel(50.0);
            }
            else {
                the3DEngine->setDepthShadingLevel(150.0);
            }

            int toggle = rand() & 1;
            if (toggle) {
                (*theWorldMap)[17][1] = 0x00f1c200;
                (*theWorldMap)[17][0] = 0x0000009c;
                (*theWorldMap)[2][3] = 0x00f1c200;
            }
            else {
                (*theWorldMap)[17][1] = 0x0000c100;
                (*theWorldMap)[17][0] = 0x00000009;
                (*theWorldMap)[2][3] = 0x0000c100;
            }
        }
        break;

        }
    }
    break;

    case WM_PAINT:
    {
        videoHdc = BeginPaint(hWnd, &ps);
#ifdef SHOW_FPS
        char s_fps[32] = { 0 };
        sprintf(s_fps, "FPS = %g", double(FPS) / 4.0);
        TextOut(videoHdc, 0, 0, s_fps, strlen(s_fps));
#endif
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_ACTIVATE:
        // Pause if minimized
        g_bActive = !((BOOL)HIWORD(wParam));
        return 0L;

    case WM_KEYDOWN:
        // Handle any non-accelerated key commands
        switch (wParam) {
        case VK_ESCAPE:
        case VK_F12:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0L;
        default:
            break;
        }
        break;

    case WM_DESTROY:
        //delete theJoystick;
        delete theWorldMap;
        delete the3DEngine;
        ReleaseAllObjects();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


/* -------------------------------------------------------------------------- */

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}


/* -------------------------------------------------------------------------- */

static void ReleaseAllObjects(void) 
{
    if (!g_pDD)
        return;

    if (g_pDDSPrimary) {
        g_pDDSPrimary->Release();
        g_pDDSPrimary = nullptr;
    }

    g_pDD->Release();
    g_pDD = nullptr;
}


/* -------------------------------------------------------------------------- */

static HRESULT DbgTrace(HWND hWnd, HRESULT hRet, LPCTSTR szError, ...) 
{
    char szBuff[256];
    va_list vl;

    va_start(vl, szError);
    vsprintf(szBuff, szError, vl);
    ReleaseAllObjects();
    MessageBox(hWnd, szBuff, g_szAppTitle, MB_OK);
    DestroyWindow(hWnd);
    va_end(vl);
    return hRet;
}


/* -------------------------------------------------------------------------- */

static HRESULT InitInstanceDD7(HINSTANCE hInstance, int nCmdShow)
{
    HWND           hWnd;
    DDSURFACEDESC2 ddsd;
    //DDSCAPS2     ddscaps;
    HRESULT        hRet;

    WRCstRegisterClass(hInstance);

    // Create a window
    hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        g_szAppWinClass,
        g_szAppTitle,
        g_FullScreenModeActive ?
        WS_POPUP :
        WS_POPUPWINDOW | WS_CAPTION | WS_BORDER,
        0,
        0,
        X_RES,
        Y_RES,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    SetFocus(hWnd);
    if (g_FullScreenModeActive) ShowCursor(FALSE);

    // Create the main DirectDraw object
    hRet = DirectDrawCreateEx(NULL, (VOID**)&g_pDD, IID_IDirectDraw7, NULL);
    if (hRet != DD_OK)
        return DbgTrace(hWnd, hRet, "DirectDrawCreateEx FAILED");

    // Get normal mode
    hRet = g_pDD->SetCooperativeLevel(hWnd,
        g_FullScreenModeActive ? DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN : DDSCL_NORMAL);

    if (hRet != DD_OK)
        return DbgTrace(hWnd, hRet, "SetCooperativeLevel FAILED");

    if (g_FullScreenModeActive) {
        hRet = g_pDD->SetDisplayMode(X_RES, Y_RES, 32 /* bits per color */, 0, 0);
        if (hRet != DD_OK)
            return DbgTrace(hWnd, hRet, "SetDisplayMode FAILED");

        // Create the primary surface with 1 back buffer
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;
        hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL);
        
        if (hRet != DD_OK) {
            return DbgTrace(hWnd, hRet, "CreateSurface FAILED");
        }
    }
    else {
        // Create the primary surface
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL);
        if (hRet != DD_OK)
            return DbgTrace(hWnd, hRet, "CreateSurface FAILED");

    }
    g_hWnd = hWnd;

    return DD_OK;
}

