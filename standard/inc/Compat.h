#ifndef INC_Compat
#define INC_Compat

#ifndef _WIN32

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>

typedef uint32_t DWORD;
typedef DWORD* LPDWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef uint8_t* PBYTE;
typedef int32_t LONG;
typedef int32_t* PLONG;
typedef uint32_t UINT;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HGDIOBJ;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HPEN;
typedef void* HBRUSH;
typedef void* HPALETTE;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HMENU;
typedef uint32_t COLORREF;
typedef uint32_t* LPCOLORREF;
typedef LONG HRESULT;

typedef struct _SURFACE* SURFACE;
typedef struct _SURFACE* LPDIRECTDRAWSURFACE;
typedef struct _SURFACE* LPDIRECTDRAWSURFACE2;

#define TRUE 1
#define FALSE 0

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    uintptr_t wParam;
    uintptr_t lParam;
    DWORD  time;
    POINT  pt;
} MSG, *PMSG, *LPMSG;

typedef uintptr_t WPARAM;
typedef uintptr_t LPARAM;
typedef intptr_t LRESULT;

typedef int BOOL;

#define _MAX_PATH 260
#define _MAX_DIR 260
#define _MAX_FNAME 256
#define _MAX_EXT 256

#define WINAPI
#define CALLBACK
#define APIENTRY
#define __stdcall
#define __cdecl
#define __fastcall

#define MAX_PATH 260

// Some common Windows macros
#define LOWORD(l)           ((WORD)(((uintptr_t)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((uintptr_t)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((uintptr_t)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((uintptr_t)(w)) >> 8) & 0xff))

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

// Stubs for DirectX/Windows types that might be used
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef char* LPSTR;
typedef const char* LPCSTR;

// DirectDraw stubs
typedef void* LPDIRECTDRAW;
typedef void* LPDIRECTDRAWPALETTE;
typedef void* LPDIRECTDRAWCLIPPER;

typedef void* PAVIFILE;
typedef void* PAVISTREAM;
typedef void* PGETFRAME;

typedef uint32_t _fsize_t;

struct _finddata_t {
    char        name[260];
    uint32_t    attrib;
    uint32_t    size;
    time_t      time_create;
    time_t      time_access;
    time_t      time_write;
};

#define _A_SUBDIR 0x10

extern "C" {
intptr_t _findfirst(const char *a, struct _finddata_t *b);
int _findnext(intptr_t a, struct _finddata_t *b);
void _findclose(intptr_t a);
}

// Registry stubs
typedef void* HKEY;
#define HKEY_CLASSES_ROOT     ((HKEY)(intptr_t)0x80000000)
#define HKEY_CURRENT_USER      ((HKEY)(intptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE     ((HKEY)(intptr_t)0x80000002)
#define HKEY_USERS             ((HKEY)(intptr_t)0x80000003)

// Bitmap stubs
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER, *LPBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER {
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER, *LPBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, *PBITMAPINFO, *LPBITMAPINFO;

// Functions
inline BOOL PeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) { return FALSE; }
inline BOOL GetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) { return TRUE; }
inline BOOL TranslateMessage(const MSG *lpMsg) { return FALSE; }
inline LRESULT DispatchMessage(const MSG *lpMsg) { return 0; }
inline BOOL IsDialogMessage(HWND hWnd, LPMSG lpMsg) { return FALSE; }
inline void PostQuitMessage(int nExitCode) { exit(nExitCode); }

#include <unistd.h>
#include <libgen.h>

inline BOOL GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize) { 
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        char* dir = dirname(buf);
        strncpy(lpFilename, dir, nSize);
        lpFilename[nSize-1] = '\0';
        return TRUE;
    }
    return FALSE; 
}
inline BOOL SetCurrentDirectory(LPCSTR lpPathName) { return chdir(lpPathName) == 0; }
inline DWORD GetTempPath(DWORD nBufferLength, LPSTR lpBuffer) { 
    const char* tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
    strncpy(lpBuffer, tmp, nBufferLength);
    return strlen(lpBuffer);
}
#include <sys/stat.h>

inline BOOL CreateDirectory(LPCSTR lpPathName, void* lpSecurityAttributes) { 
    return mkdir(lpPathName, 0755) == 0; 
}

// UI Stubs
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_ESCAPE 0x1B
#define VK_SCROLL 0x91
#define VK_MULTIPLY 0x6A
#define MK_CONTROL 0x0008
#define MK_SHIFT 0x0004
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_SPACE 0x20

#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
#define VK_INSERT 0x2D
#define VK_HOME 0x24
#define VK_PRIOR 0x21
#define VK_DELETE 0x2E
#define VK_UP 0x26
#define VK_NEXT 0x22
#define VK_LEFT 0x25
#define VK_DOWN 0x28
#define VK_RIGHT 0x27
#define VK_END 0x23
#define VK_RETURN 0x0D
#define VK_BACK 0x08
#define VK_ADD 0x6B
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69

#define WM_ACTIVATEAPP 0x001C
#define WM_TIMER 0x0113
#define WM_DESTROY 0x0002
#define WM_CLOSE 0x0010
#define WM_KEYDOWN 0x0100
#define WM_SYSKEYDOWN 0x0104
#define WM_CHAR 0x0102
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MOUSEMOVE 0x0200
#define WM_USER 0x0400

#define MM_MCINOTIFY 0x3B9
#define MCI_NOTIFY_SUCCESSFUL 1

inline LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return 0; }
inline BOOL DestroyWindow(HWND hWnd) { return FALSE; }
inline HWND CreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) { return (HWND)1; }
inline BOOL ShowWindow(HWND hWnd, int nCmdShow) { return TRUE; }
inline HWND SetFocus(HWND hWnd) { return hWnd; }
inline int ShowCursor(BOOL bShow) { return 0; }
inline WORD RegisterClassEx(const void* lpwcx) { return 1; }
inline HICON LoadIcon(HINSTANCE hInstance, LPCSTR lpIconName) { return 0; }
inline HCURSOR LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName) { return 0; }

typedef struct tagWNDCLASSEX {
    UINT        cbSize;
    UINT        style;
    LRESULT (CALLBACK *lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
    HICON       hIconSm;
} WNDCLASSEX, *PWNDCLASSEX;

#define CS_DBLCLKS 0x0008
#define CS_BYTEALIGNCLIENT 0x1000
#define COLOR_BACKGROUND 1
#define IDC_ARROW ((LPCSTR)32512)
#define IDC_WAIT  ((LPCSTR)32514)
#define SW_SHOWNORMAL 1
#define WS_POPUP 0x80000000L
#define CW_USEDEFAULT ((int)0x80000000)
#define MAKEINTRESOURCE(i) ((LPCSTR)((DWORD)((WORD)(i))))

#define STILL_ACTIVE 259
inline BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) { return FALSE; }
inline void Sleep(DWORD dwMilliseconds) { usleep(dwMilliseconds * 1000); }
inline BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode) { return FALSE; }
inline HANDLE CreateThread(void* lpThreadAttributes, DWORD dwStackSize, DWORD (WINAPI *lpStartAddress)(void*), void* lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) { return 0; }

inline BOOL PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return FALSE; }
inline LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return 0; }

inline BOOL DeleteObject(HGDIOBJ ho) { return FALSE; }
inline HDC GetDC(HWND hWnd) { return 0; }
inline int ReleaseDC(HWND hWnd, HDC hDC) { return 0; }
inline HBITMAP CreateDIBitmap(HDC hdc, const void* lpbmih, DWORD fdwInit, const void* lpbInit, const void* lpbmi, UINT fuUsage) { return 0; }
inline HICON CreateIconFromResource(PBYTE presbits, DWORD dwResSize, BOOL fIcon, DWORD dwVer) { return 0; }
inline DWORD GetLastError() { return 0; }

typedef struct utimbuf _utimbuf;
#define _utime utime

#define CBM_INIT 0
#define DIB_RGB_COLORS 0
#define BI_RGB 0

typedef struct hostent HOSTENT;
inline short GetKeyState(int nVirtKey) { return 0; }

inline HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) { return 0; }
inline BOOL GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize) { return FALSE; }
#define TA_LEFT 0
inline UINT SetTextAlign(HDC hdc, UINT align) { return 0; }
#define TRANSPARENT 1
inline int SetBkMode(HDC hdc, int mode) { return 0; }
#define RGB(r,g,b) ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
inline COLORREF SetTextColor(HDC hdc, COLORREF color) { return 0; }
#define ETO_CLIPPED 0x0004
inline BOOL ExtTextOut(HDC hdc, int x, int y, UINT options, const RECT *lprc, LPCSTR lpString, UINT c, const int *lpDx) { return FALSE; }
inline int SaveDC(HDC hdc) { return 0; }
inline BOOL RestoreDC(HDC hdc, int nSavedDC) { return 0; }
#define GM_ADVANCED 2
inline int SetGraphicsMode(HDC hdc, int iMode) { return 0; }
#define MWT_IDENTITY 1
inline BOOL ModifyWorldTransform(HDC hdc, const void* lpxf, DWORD mode) { return FALSE; }
inline BOOL SetViewportOrgEx(HDC hdc, int x, int y, LPPOINT lppt) { return FALSE; }
inline BOOL SetWindowOrgEx(HDC hdc, int x, int y, LPPOINT lppt) { return FALSE; }
typedef struct tagLOGFONT { LONG lfHeight; LONG lfWidth; LONG lfEscapement; LONG lfOrientation; LONG lfWeight; BYTE lfItalic; BYTE lfUnderline; BYTE lfStrikeOut; BYTE lfCharSet; BYTE lfOutPrecision; BYTE lfClipPrecision; BYTE lfQuality; BYTE lfPitchAndFamily; char lfFaceName[32]; } LOGFONT;
inline HFONT CreateFontIndirect(const LOGFONT *lplf) { return 0; }

inline BOOL CopyFile(const char* lpExistingFileName, const char* lpNewFileName, BOOL bFailIfExists) { return FALSE; }
inline int LoadString(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int nBufferMax) { return 0; }

// Dummy function to match some windows calls
#define GetUserDefaultLangID() 0x0409
#define PRIMARYLANGID(l) (l & 0x3ff)
#define LANG_GERMAN 0x07

#endif // _WIN32

#endif // INC_Compat
