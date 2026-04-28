#include <windows.h>
#include <stdio.h>

void DumpWindow(HWND hwnd, int indent) {
    char className[256];
    char windowText[256];
    RECT rect, crect;
    if (!GetClassName(hwnd, className, sizeof(className))) className[0] = 0;
    if (!GetWindowText(hwnd, windowText, sizeof(windowText))) windowText[0] = 0;
    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &crect);
    
    POINT pt = {0, 0};
    ClientToScreen(hwnd, &pt);
    
    int wx = rect.left, wy = rect.top;
    int ww = rect.right - rect.left, wh = rect.bottom - rect.top;
    
    int cx = pt.x, cy = pt.y;
    int cw = crect.right, ch = crect.bottom;

    for(int i=0; i<indent; i++) printf("  ");
    printf("[%s] Text:'%s' Window:%d,%d,%dx%d Client:%d,%d,%dx%d\n", 
           className, windowText, wx, wy, ww, wh, cx, cy, cw, ch);

    HWND child = GetWindow(hwnd, GW_CHILD);
    while(child) {
        DumpWindow(child, indent + 1);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    char text[256];
    if (!GetWindowText(hwnd, text, sizeof(text))) return TRUE;
    if (strstr(text, "Clonk") || strstr(text, "Planet")) {
        DumpWindow(hwnd, 0);
    }
    return TRUE;
}

int main() {
    EnumWindows(EnumProc, 0);
    return 0;
}
