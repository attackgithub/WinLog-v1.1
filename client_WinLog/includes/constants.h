#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#define ID_B_OPENLOG    0
#define ID_B_IMPORTLOG  1
#define ID_B_QUIT       2
#define ID_B_SCREENSHOT 3
#define ID_B_INIT       4
#define ID_SERVER       5
#define ID_B_SETIP      6
#define ID_MINIMIZE     7
#define ID_CLOSE        8

BOOL isMouseDownOnCloseButton = FALSE;
BOOL isMouseDownOnMinimizeButton = FALSE;

LRESULT CALLBACK mainWindowProc(HWND main_window, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditFileProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

VOID FillWindow(HWND main_window);
void DrawCloseButton(HDC hdc);
void DrawMinimizeButton(HDC hdc);
void Draw_LeftRightBottom_Rectangles(RECT rect, HDC hdc, HBRUSH brush, int width, int height);

typedef int socklen_t;
#define PORT 4444
#define MAXDATASIZE 50000

#endif


