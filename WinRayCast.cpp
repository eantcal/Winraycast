// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include "DdxDevice.h"

#include <string>


/* -------------------------------------------------------------------------- */

using namespace std;


/* -------------------------------------------------------------------------- */

static void DbgTrace(HWND hWnd, LPCTSTR szError, ...);
static HRESULT InitInstance(HINSTANCE hInstance, int nCmdShow);


/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include "resource.h"
#include "RaycastEngine.h"

/* -------------------------------------------------------------------------- */

#define CELL_SIZE    512
#define VISUAL_DEGREE 60 

#define KEYBSTEP  10
#define KEYBALPHA  4

#define X_RES 1024
#define Y_RES 768

#define PROJ_X_RES 512
#define PROJ_Y_RES 512

#define FIRE_EFFECT  1
#define WATER_EFFECT 2
#define LIGHT_EFFECT 3
#define FPS_INFO     4

#define SCALE 250000

#define MAX_LOADSTRING 100
#define FULL_SCREEN_MODE TRUE

#define CAMERA_CEL_COL_POS 4
#define CAMERA_CEL_ROW_POS 4



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
static Cell g_current_cell_of_player = 0;

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
bool Setup3DEngine(RaycastEngine** the3DEngine, WorldMap** theWorldMap)
{
    *theWorldMap = new (std::nothrow) WorldMap;

    if (!(*theWorldMap)) {
        return false;
    }

    WorldMap & world = **theWorldMap;

    Player aCamera = Player(0, 0, VISUAL_DEGREE, PROJ_X_RES, PROJ_Y_RES);
    aCamera.setPos(
        make_pair<int, int>(CELL_SIZE * CAMERA_CEL_COL_POS,
            CELL_SIZE * CAMERA_CEL_ROW_POS)
    );

    world.load("res/world.ini");
    world.resizeCell(CELL_SIZE, CELL_SIZE);

    const auto & textureList = world.getTextureList();

    auto loadBMP = [](const std::string& image,
        const int dx = CELL_SIZE,
        const int dy = CELL_SIZE,
        const char* ext = ".bmp")
    {
        string image_name = "res/";
        image_name += image;
        image_name += ext;

        return (HBITMAP)
            LoadImage(g_hInstance,
                image_name.c_str(),
                IMAGE_BITMAP,
                dx, dy,
                LR_LOADFROMFILE);
    };

    for (const auto & item : textureList) {
        world.applyTextureToPanel(
            stoi(item.first, 0, 16),
            loadBMP(item.second)
        );
    }

#define SKY_BMP_RESOURCE "clouds"

    world.applyTextureToPanel(
        255,
        loadBMP(SKY_BMP_RESOURCE, PROJ_X_RES, PROJ_Y_RES)
    );

    *the3DEngine = new RaycastEngine(aCamera, SCALE);

    return true;
}


/* -------------------------------------------------------------------------- */

inline
HBITMAP loadBMP(const std::string& image,
    const int dx = CELL_SIZE,
    const int dy = CELL_SIZE,
    const char* ext = ".bmp")
{
    string image_name = "res/";
    image_name += image;
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

    if (InitInstance(hInstance, nCmdShow) != S_OK) {
        return FALSE;
    }

    g_hInstance = hInstance;

    Setup3DEngine(&the3DEngine, &theWorldMap);

    

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

    return int(msg.wParam);
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
                the3DEngine->player().moveToH(KEYBSTEP, *theWorldMap);
        }
        else if (GetAsyncKeyState(VK_RIGHT)) {
            g_current_cell_of_player =
                the3DEngine->player().moveToH(-KEYBSTEP, *theWorldMap);
        }
    }
    else {
        if (GetAsyncKeyState(VK_LEFT)) {
            the3DEngine->player().rotate(-KEYBALPHA);
        }
        else if (GetAsyncKeyState(VK_RIGHT)) {
            the3DEngine->player().rotate(KEYBALPHA);
        }
    }

    bool move_up_down = false;

    if (GetAsyncKeyState(VK_UP)) {
        g_current_cell_of_player =
            the3DEngine->player().moveTo(KEYBSTEP*speedFact, *theWorldMap);

        move_up_down = true;
    }
    else if (GetAsyncKeyState(VK_DOWN)) {
        g_current_cell_of_player =
            the3DEngine->player().moveTo(-KEYBSTEP * speedFact, *theWorldMap);

        move_up_down = true;
    }

    if (GetAsyncKeyState(VK_PRIOR)) {
        the3DEngine->player().setSlope(
            the3DEngine->player().getSlope() + KEYBSTEP);
    }
    else if (GetAsyncKeyState(VK_NEXT)) {
        the3DEngine->player().setSlope(
            the3DEngine->player().getSlope() - KEYBSTEP);
    }

    if (GetAsyncKeyState(VK_END)) {
        the3DEngine->player().setCenterProj(double(0.90));
    }
    else if (GetAsyncKeyState(VK_HOME)) {
        the3DEngine->player().setCenterProj(double(0.10));
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

    case WM_PAINT:
        videoHdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
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
        DdxDevice::getInstance().releaseObjects();
        //ReleaseAllObjects();
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

static void DbgTrace(HWND hWnd, LPCTSTR szError, ...) 
{
    char szBuff[256];
    va_list vl;

    va_start(vl, szError);
    vsprintf(szBuff, szError, vl);
    //ReleaseAllObjects();

    DdxDevice::getInstance().releaseObjects();
    MessageBox(hWnd, szBuff, g_szAppTitle, MB_OK);
    DestroyWindow(hWnd);
    va_end(vl);
}


/* -------------------------------------------------------------------------- */

static HRESULT InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    WRCstRegisterClass(hInstance);

    // Create a window
    HWND hWnd = CreateWindowEx(
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

    auto err = DdxDevice::getInstance().init(hWnd, g_FullScreenModeActive, X_RES, Y_RES );

    if (err != DdxDevice::error_t::Success) {
        DbgTrace(hWnd, "DdxDevice initialization failed");
        DdxDevice::getInstance().releaseObjects();
        DestroyWindow(hWnd);
    }

    g_hWnd = hWnd;
    return S_OK;
}

