#include <windows.h>
#include "resourceLab9.h"

#define CLASS_NAME "F(DSHNLK#NROUFYE(*nlk234nouo3yf98h23n"
#define MAX_EXTANTIONS 256


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void GetFileExtension(const char* filename, char* extBuffer, int bufferSize);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow)
{

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) return -1;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int indentX = (int)(screenWidth * 0.07);  // 7% от ширины экрана
    int indentY = (int)(screenHeight * 0.07); // 7% от высоты экрана

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab9",
        WS_OVERLAPPEDWINDOW,
        indentX,
        indentY,
        screenWidth * 0.35,
        screenHeight * 0.35,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, iCmdShow);

    char currentPath[MAX_PATH];

    // путь к исполняемому файлу
    GetModuleFileNameA(NULL, currentPath, MAX_PATH);

    // удаление имени файла из пути
    char* lastBackslash = strrchr(currentPath, '\\');
    if (lastBackslash)
    {
        *lastBackslash = '\0';
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int width = clientRect.right - clientRect.left;  // Ширина
    int height = clientRect.bottom - clientRect.top; // Высота

    // путь
    //окна стоило создать через ресурсы
    HWND hStaticPath = CreateWindow("STATIC",
        currentPath,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        (int)width * 0.025, (int)height * 0.03, (int)width * 0.96, (int)height * 0.07, //стоило минимизировать рассчёты и сделать аккуратнее
        hWnd,
        (HMENU)IDC_STATIC_PATH,
        hInstance,
        NULL);

    // COMBOBOX для расширений
    HWND hComboExt = CreateWindow("COMBOBOX",
        NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_TABSTOP | WS_VSCROLL,
        (int)width * 0.025, (int)height * 0.15, (int)width * 0.96, (int)height * 0.5,
        hWnd,
        (HMENU)IDC_COMBO_EXT,
        hInstance,
        NULL);

    //  LISTBOX для файлов
    HWND hListFiles = CreateWindow("LISTBOX",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
        LBS_STANDARD | LBS_SORT | LBS_MULTIPLESEL,
        (int)width * 0.025, (int)height * 0.25, (int)width * 0.96, (int)height * 0.75,
        hWnd,
        (HMENU)IDC_LIST_FILES,
        hInstance,
        NULL);

    // заполнение LISTBOX 
    SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_DIR, DDL_READWRITE, (LPARAM)"*.*");

    // массив для заполнения COMBOBOX
    int fileCount = (int)SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_GETCOUNT, 0, 0);
    char extensions[MAX_EXTANTIONS][20] = { 0 };
    int extCount = 0;

    for (int i = 0; i < fileCount; i++)
    {
        char filename[MAX_PATH];
        SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_GETTEXT, i, (LPARAM)filename);


        char ext[20];
        GetFileExtension(filename, ext, 20);

        BOOL isUnique = TRUE;
        for (int j = 0; j < extCount; j++)
        {
            if (strcmp(extensions[j], ext) == 0)
            {
                isUnique = FALSE;
                break;
            }
        }

        // Добавление уникального расширения
        if (isUnique && extCount < MAX_EXTANTIONS)
        {
            strcpy_s(extensions[extCount], 20, ext);
            extCount++;
        }
    }

    // Добавление расширений в COMBOBOX
    for (int i = 0; i < extCount; i++)
    {
        SendDlgItemMessageA(hWnd, IDC_COMBO_EXT, CB_ADDSTRING, 0, (LPARAM)extensions[i]);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_COMMAND:
    {

        switch (LOWORD(wParam))
        {
        case ID_EXIT:
            DestroyWindow(hWnd);
            return 0;

        case IDC_COMBO_EXT:
            int selIndex = (int)SendDlgItemMessageA(hWnd, IDC_COMBO_EXT, CB_GETCURSEL, 0, 0);

            char selectedExt[20];
            SendDlgItemMessageA(hWnd, IDC_COMBO_EXT, CB_GETLBTEXT, selIndex, (LPARAM)selectedExt);

            // снятие выделения со всех файлов
            SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_SETSEL, FALSE, -1);

            int fileCount = (int)SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_GETCOUNT, 0, 0);

            // выделение файлов с выбранным расширением
            for (int i = 0; i < fileCount; i++)
            {
                char filename[MAX_PATH];
                SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_GETTEXT, i, (LPARAM)filename);


                char fileExt[20];
                GetFileExtension(filename, fileExt, 20);

                // Сравнение расширений
                if (strcmp(selectedExt, "<без расширения>") == 0)
                {
                    // Проверка на файл без расширения
                    const char* dot = strrchr(filename, '.');
                    if (dot == NULL || dot[1] == '\0')
                    {
                        SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_SETSEL, TRUE, i);
                    }
                }
                else if (strcmp(fileExt, selectedExt) == 0)
                {
                    SendDlgItemMessageA(hWnd, IDC_LIST_FILES, LB_SETSEL, TRUE, i);
                }
            }
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

// получение расширения файла
void GetFileExtension(const char* filename, char* extBuffer, int bufferSize)
{
    const char* dot = strrchr(filename, '.'); //проверка на последнее вхождение символа

    if (dot && dot[1] != '\0')
    {
        // копирование расширения
        strncpy_s(extBuffer, bufferSize, dot + 1, _TRUNCATE);
    }
    else
    {
        // для файлов без расширения
        strcpy_s(extBuffer, bufferSize, "<без расширения>");
    }
}