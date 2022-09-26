/*
 * This is the main program. The 2 primary requirements to run a windows program are:
 * 1. Message loop: feeds message into windows, and windows will process and send these messages to
 *    the application constantly to check for user input
 * 2. Window procedure (callback): when window receives a message, the message will be dispatched back
 *    into the window function, and this is where we will handle it with our own set of logic and
 *    instructions (aka coding).
 * For windows functions, there are 2 main types of input processing functions:
 *    - ASCII functions (SomeFunctionA)
 *    - Unicode functions (SomeFunction)
 *    The difference between these 2 are unicode is broader, though it will go with a different set of
 *    functions for message handling (whereas ASCII can be handled with strcpy and whatnot).
 *
 * Programming in windows: EVERYTHING is a window.
 * Each 'area' on the screen that may mean something is considered a window. The window itself is, of
 * course, a window. So one of the first steps that we must do is creating a window class.
 *
 * ----------------
 *
 * Coding tips:
 *
 * - struct bit initialization:
 *      We can initialize a struct in C like this: struct datatype t = {0};
 *      What this means is C will look into the size of t, and initialize 0s accordingly. Say, t has a
 *      size of 16 bytes, then it will initialize 16 zeroes into the memory heap.
 *
 * - label:
 *      We can label checkpoints anywhere in code, and simply type "goto" whenever we need the code to
 *      jump to that label.
 *
 * - Input/Output parameter specifier
 *      Remember declarative programming languages? It is quite good to have input and output specs as
 *      they give more insights to the functionality to the compiler and hence compilers can give more
 *      intelligent warnings. Here's how we do it in C:
 *      --> function(_In_ Image t, _Inout_ double x, _Out_ DWORD y);
 *      input: t, in-and-output: x, output: y
 *
 * - return void:
 *      There are more implications to returning void than just simply a function that isn't supposed
 *      to return anything. What this implies is that this function is NOT expected to fail, menaing
 *      there are no 'different states' to be returned, and that the function will not fail and simply
 *      carry out its job.
 *
 * - naming conventions:
 *      global variable often starts with a 'g', i.e. 'gVariableName'.
 */
/*
 * Documentation:
 * - Mutex:
 *      Ensures serialization. Meaning, say you don't want multiple threads to write onto the same
 *      file simultaneously because that may corrupt the file. What you want instead is for things to
 *      take turn - as long as the thread is holding the mutex, it's its turn until it gives up on it.
 * - OutputDebugString:
 *      Allows you to talk directly to the compiler, and is optimized out by compiler if run normally.
 *      It will however print in GDB console what you need to know in debug mode.
 * - Sleep(0):
 *      What Sleep(0) does is that it allows other threads to join it in the CPU instead of hogging
 *      the entire CPU to itself. However, the downside is that when there's no other thread wanting
 *      to take turn, the program (however small) will still take up the entire CPU anyway. Not to
 *      mention, this thread work exchange lacks predictability and control.
 * - Memory allocation:
 *      VirtualAlloc: window's function allocating large chunks of memory and is suitable only if.
 *      HeapAlloc: window's memory allocation for small or very small chunks of memory (like malloc).
 * - PostQuitMessage(0)
 *      Send the message as 0, hence terminating the get message (GetMessage or PeekMessage) while
 *      loop in the input processing while loop of WindMain function.
 * - Bits and bytes:
 *      uint8 means integer to 8 bits: 0000 0000, which is up to 255. Numbers are stored in computers
 *      this way:
 *                 31
 *                 2684 21
 *                 7310 0052 1
 *                 6899 4215 2631
 *                 8426 8426 8426 8421
 *                 -------------------
 *                 0000 0000 0000 0000
 *                 -------------------
 *      What this means is: if we flip any 0 to 1 corresponding to the position, it will reach to that
 *      integer. Say we flip:
 *                 -------------------
 *                 0000 1000 0000 0000  -->  2048
 *                 1000 0000 0000 0000  -->  32756
 *                 -------------------
 *      What this also means is: if we turn every value before flipping 0 to 1s, we get the largest
 *      possible integer within that bit range:
 *                 -------------------
 *                 0000 0111 1111 1111  -->  2047
 *                 1111 1111 1111 1111  -->  32756 * 2 - 1 = 131071
 *                 0000 0000 1111 1111  -->  255 (8-bit)
 *                 -------------------
 *      That's how we know RGBA (red-green-blue-alpha) is 8 bits * 4 = 32 bits (a 32-bit pixel).
 *      NOTE: so far we've only considered unsigned (positive only). If it's negative, then it must
 *      sacrifice 1 bit for sign, meaning it will take up more memory space to store a signed int.
 * - memset:
 *      This function essentially fills up the bits of memory with either 0 or 1. Can be used to draw
 *      pixels on the screen (memset would fill RGB values accordingly).
 */

#pragma once

#define GAME_NAME "RETRO-GAME"          // name of game
#define WINDOW_TITLE "Cool retro game"  // title displayed on window
#define WINDOW_WIDTH 640                // window's boot-up width
#define WINDOW_HEIGHT 480               // window's boot-up height
#define RES_WIDTH 384                   // NES width resolution (used for drawing surface)
#define RES_HEIGHT 240                  // NES height resolution
#define BPP 32                          // bits per pixel
#define DRAWING_AREA_MEMSIZE (RES_WIDTH*RES_HEIGHT*(BPP)/8)
                                        // memory required for an entire single frame

/* Game bit map structure */
typedef struct GAMEBITMAP {
    BITMAPINFO BitMapInfo;
    void* Memory;
} GAMEBITMAP;

/* 32-bit pixel */
typedef struct PIXEL32 {
    // each component is 8 bits, or 1 byte
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
    uint8_t Alpha;
} PIXEL32;

/* creating window, returns the double word correspond to message */
DWORD CreateMainWindowA(void);

/* callback function prototype */
LRESULT CALLBACK MainWndProc(
        _In_ HWND WindowHandle,      // handle to window
        _In_ UINT Message,           // message identifier
        _In_ WPARAM wParam,          // first message parameter
        _In_ LPARAM lParam);         // second message parameter

/* function restricting only 1 window of the game to be run at a time */
BOOL AlreadyRunning(void);

/* player's input processing */
void ProcessPlayerInput(void);

/* rendering frame graphics of window */
void RenderFrameGraphics(void);