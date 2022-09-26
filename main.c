/*
 * main program creating a window and booting up the game
 */

#include <stdio.h>
//#pragma push_macro("")
#include <windows.h>
//#pragma pop_macro("")
#include <stdint.h>
#include "main.h"

// variable ensuring game is constantly running even without a message sent
BOOL gGameIsRunning;
// window handle
HWND gWindowHandle;
// drawing surface
GAMEBITMAP gBackBuffer;
// monitor info particularly important for full-screen mode
MONITORINFO gMonitorInfo = {sizeof(MONITORINFO)};
// monitor information
int32_t gMonitorWidth;
int32_t gMonitorHeight;

/* window main function */
INT WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, INT CmdShow) {
    // specifying intentionally unused variables
    UNREFERENCED_PARAMETER(Instance)
    UNREFERENCED_PARAMETER(PrevInstance)
    UNREFERENCED_PARAMETER(CommandLine)
    UNREFERENCED_PARAMETER(CmdShow)

    // if game is already running
    if (AlreadyRunning() == TRUE) {
        MessageBoxA(NULL, "Program is already opened!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    // remember ERROR_SUCCESS means everything's fine
    if (CreateMainWindowA() != ERROR_SUCCESS) {
        goto Exit;
    }

    /* Drawing surface specifications */
    gBackBuffer.BitMapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitMapInfo.bmiHeader);
    gBackBuffer.BitMapInfo.bmiHeader.biWidth = RES_WIDTH;
    gBackBuffer.BitMapInfo.bmiHeader.biHeight = RES_HEIGHT;
    gBackBuffer.BitMapInfo.bmiHeader.biBitCount = BPP;
    gBackBuffer.BitMapInfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.BitMapInfo.bmiHeader.biPlanes = 1;
    gBackBuffer.Memory = VirtualAlloc(NULL, DRAWING_AREA_MEMSIZE,
                                      MEM_RESERVE | MEM_COMMIT,
                                      PAGE_READWRITE);
    // assertion that memory allocation doesn't fail
    if (gBackBuffer.Memory == NULL) {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!",
                    "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    /* Processing message and dispatching to callback function */
    MSG Message = {0};
    gGameIsRunning = TRUE;
    while (gGameIsRunning) {
        // GetMessageA(&Message, NULL, 0, 0) - while loop freezes until a message is passed (>0);
        // by changing to PeekMessage + a while loop outside, game will continue updating frames.
        while (PeekMessageA(&Message, gWindowHandle, 0, 0,
                            PM_REMOVE) > 0) {
            DispatchMessageA(&Message);
        }

        // process player's input
        ProcessPlayerInput();

        // render frame graphics
        RenderFrameGraphics();

        // sleep - letting the while loop rest (reducing CPU intake) - read more in documentation
        Sleep(1);
    }

Exit:
    return 0;
}

/* callback function (window processing) */
LRESULT CALLBACK MainWndProc(_In_ HWND WindowHandle, _In_ UINT Message,
                             _In_ WPARAM wParam, _In_ LPARAM lParam) {
    LRESULT Result;

    /// DEBUG - output string whenever message is sent
    char buf[12] = {0};
    _itoa_s((int) Message, buf, _countof(buf), 10);
    OutputDebugStringA(buf);
    ///

    /* switch-case for different user messages */
    switch (Message) {

        // window closed with [x]
        case WM_CLOSE:
            gGameIsRunning = FALSE;
            // setting GetMessageA to 0
            PostQuitMessage(0);
            Result = 0;
            break;

        case WM_CREATE:
            // Initialize the window.

        case WM_PAINT:
            // Paint the window's client area.

        case WM_SIZE:
            // Set the size and position of the window.

        case WM_DESTROY:
            // Clean up window-specific data objects.

        default:
            // DefWindowProc(A) returns the result of message processing
            Result = DefWindowProcA(WindowHandle, Message, wParam, lParam);
    }
    return Result;
}

/* creating window, returns the double word correspond to message */
DWORD CreateMainWindowA() {

    // nothing went wrong (ERROR_SUCCESS)
    DWORD Result = ERROR_SUCCESS;
    // GetModuleHandle is the equivalent to getInstance of Java
    HINSTANCE Instance = GetModuleHandleA(NULL);

    /* Creating window class */
    WNDCLASSEXA WindowClass = {0}; // WNDCLASSEXA is ASCII's version, WNDCLASSEX is unicode.

    WindowClass.cbSize        = sizeof(WNDCLASSEXA);
    WindowClass.style         = 0;
    WindowClass.lpfnWndProc   = MainWndProc;
    WindowClass.cbClsExtra    = 0;
    WindowClass.cbWndExtra    = 0;
    WindowClass.hInstance     = Instance;
    WindowClass.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hIconSm       = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255,0,255));
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

    // DPI awareness (regardless of zoom level)
    SetProcessDPIAware();

    if (!RegisterClassExA(&WindowClass)) {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Registration Failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    /* create the window */
    gWindowHandle = CreateWindowExA(
            0,
            WindowClass.lpszClassName,      // window class name
            WINDOW_TITLE,                  // window title
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // window style
            CW_USEDEFAULT, CW_USEDEFAULT,          // window positions
            WINDOW_WIDTH, WINDOW_HEIGHT, // window width and height
            NULL, NULL,
            Instance, NULL);

    // if window creation fails
    if (gWindowHandle == NULL) {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Creation Failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    // after creating window, we may later resize that window, and store into MONITORINFO struct
    if (GetMonitorInfoA(MonitorFromWindow(gWindowHandle,
                        MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0) {
        Result = ERROR_MCA_MONITOR_VIOLATES_MCCS_SPECIFICATION;
        goto Exit;
    }

    // full-screen styling
    if (SetWindowLongPtrA(gWindowHandle, GWL_STYLE,
                          // window style: keeping VISIBLE but remove OVERLAPPEDWINDOW attributes
                          (WS_OVERLAPPEDWINDOW|WS_VISIBLE)&~WS_OVERLAPPEDWINDOW)==0) {
        Result = GetLastError();
        goto Exit;
    }

    // monitor information
    gMonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;
    gMonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

    // full-screen position
    if (SetWindowPos(gWindowHandle, HWND_TOP,
                     gMonitorInfo.rcMonitor.left, gMonitorInfo.rcMonitor.top,
                     gMonitorWidth, gMonitorHeight, SWP_FRAMECHANGED) == 0) {
        Result = GetLastError();
        goto Exit;
    }

Exit:
    return Result;
}

/* function restricting only 1 window of the game to be run at a time */
BOOL AlreadyRunning(void) {
    HANDLE Mutex = NULL;
    // creating a mutex (serialization allowing 1 thing at a time)
    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_Mutex");
    // now we check if the mutex is already held, then there must be another window opened
    if (GetLastError() == ERROR_ALREADY_EXISTS) return TRUE;
    return FALSE;
}

/* player's input processing */
void ProcessPlayerInput(void) {
    int16_t EscIsDown = GetAsyncKeyState(VK_ESCAPE);
    // send the window a message
    if (EscIsDown) {
        SendMessageW(gWindowHandle, WM_CLOSE, 0, 0);
    }
}

/* rendering frame graphics of window */
void RenderFrameGraphics(void) {
    // drawing on back buffer
    PIXEL32 Pixel = {0};
    Pixel.Blue = 0x7f;
    Pixel.Green = 0;
    Pixel.Red = 0;
    Pixel.Alpha = 0xff;
    for (uint32_t i=0; i < RES_WIDTH*RES_HEIGHT; i++) {
        memcpy_s((PIXEL32*) gBackBuffer.Memory + i, sizeof(PIXEL32),
                 &Pixel, sizeof(PIXEL32));
    }

    HDC DeviceContext = GetDC(gWindowHandle);
    // drawing on window Device-Independently (for stretching)
    StretchDIBits(DeviceContext, 0, 0,
                  gMonitorWidth, gMonitorHeight,
                  0, 0, RES_WIDTH, RES_HEIGHT,
                  gBackBuffer.Memory, &gBackBuffer.BitMapInfo,
                  DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(gWindowHandle, DeviceContext);
}