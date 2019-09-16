#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>

#define HOOKDLL __declspec(dllexport)

int HOOKDLL set_hook(HWND hWnd, UINT UpdateMsg);
int HOOKDLL unset_hook();


#endif


