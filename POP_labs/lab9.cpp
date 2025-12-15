#include <windows.h>

// Единственная разрешенная глобальная переменная
HINSTANCE hInst;

// ID элементов управления
#define IDC_STATIC_PATH   1001
#define IDC_COMBO_EXT     1002
#define IDC_LIST_FILES    1003
#define IDM_EXIT          1004

// Прототип функции
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Получение расширения файла (без точки)
void GetFileExtension(const char* filename, char* extBuffer, int bufferSize)
{
    const char* dot = strrchr(filename, '.');

    if (dot && dot[1] != '\0')
    {
        // Безопасное копирование расширения
        strncpy_s(extBuffer, bufferSize, dot + 1, _TRUNCATE);
    }
    else
    {
        // Безопасное копирование строки для файлов без расширения
        strcpy_s(extBuffer, bufferSize, "<без расширения>");
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow)
{
    hInst = hInstance;

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "FileManagerClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
        return 0;

    // Создание меню
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, "Выход");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Файл");

    // Создание главного окна
    HWND hWnd = CreateWindowEx(0,
        "FileManagerClass",
        "Lab9 - Менеджер файлов",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600, 400,
        NULL,
        hMenu,
        hInstance,
        NULL);

    if (!hWnd)
        return 0;

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// Основная оконная процедура
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_CREATE:
    {
        char currentPath[MAX_PATH];

        // Получаем путь к исполняемому файлу
        GetModuleFileNameA(NULL, currentPath, MAX_PATH);

        // Удаляем имя файла из пути
        char* lastBackslash = strrchr(currentPath, '\\');
        if (lastBackslash)
        {
            *lastBackslash = '\0';
        }

        // Создание статического текста с путем
        HWND hStaticPath = CreateWindowA("STATIC",
            currentPath,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 10, 560, 20,
            hWnd,
            (HMENU)IDC_STATIC_PATH,
            hInst,
            NULL);

        // Создание COMBOBOX для расширений
        HWND hComboExt = CreateWindowA("COMBOBOX",
            NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_TABSTOP | WS_VSCROLL,
            10, 40, 200, 200,
            hWnd,
            (HMENU)IDC_COMBO_EXT,
            hInst,
            NULL);

        // Создание LISTBOX для файлов
        HWND hListFiles = CreateWindowA("LISTBOX",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
            LBS_STANDARD | LBS_SORT | LBS_MULTIPLESEL | WS_TABSTOP,
            10, 70, 560, 280,
            hWnd,
            (HMENU)IDC_LIST_FILES,
            hInst,
            NULL);

        // Заполнение LISTBOX файлами из текущего каталога
        SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_DIR, DDL_READWRITE, (LPARAM)"*.*");

        // Сбор уникальных расширений
        int fileCount = (int)SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_GETCOUNT, 0, 0);
        char extensions[1024][20] = { 0 };  // Максимум 1024 уникальных расширений по 20 символов
        int extCount = 0;

        // Добавляем специальный элемент "все файлы"
        strcpy_s(extensions[extCount], 20, "<все файлы>");
        extCount++;

        // Добавляем специальный элемент для файлов без расширения
        strcpy_s(extensions[extCount], 20, "<без расширения>");
        extCount++;

        for (int i = 0; i < fileCount; i++)
        {
            char filename[MAX_PATH];
            SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_GETTEXT, i, (LPARAM)filename);

            // Пропускаем каталоги (они имеют префикс [ и суффикс ])
            if (filename[0] == '[' && strstr(filename, "]"))
                continue;

            char ext[20];
            GetFileExtension(filename, ext, 20);

            // Пропускаем уже добавленные расширения и специальные элементы
            if (strcmp(ext, "<без расширения>") == 0)
                continue;

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
            if (isUnique && extCount < 1024)
            {
                strcpy_s(extensions[extCount], 20, ext);
                extCount++;
            }
        }

        // Добавление расширений в COMBOBOX
        for (int i = 0; i < extCount; i++)
        {
            SendMessageA(GetDlgItem(hWnd, IDC_COMBO_EXT), CB_ADDSTRING, 0, (LPARAM)extensions[i]);
        }

        // Установка первого элемента как выбранного
        SendMessageA(GetDlgItem(hWnd, IDC_COMBO_EXT), CB_SETCURSEL, 0, 0);

        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case IDC_COMBO_EXT:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int selIndex = (int)SendMessageA(GetDlgItem(hWnd, IDC_COMBO_EXT), CB_GETCURSEL, 0, 0);

                if (selIndex != CB_ERR)
                {
                    char selectedExt[20];
                    SendMessageA(GetDlgItem(hWnd, IDC_COMBO_EXT), CB_GETLBTEXT, selIndex, (LPARAM)selectedExt);

                    // Снятие выделения со всех файлов
                    SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_SETSEL, FALSE, -1);

                    // Если выбран "<все файлы>" - не выделяем ничего
                    if (strcmp(selectedExt, "<все файлы>") != 0)
                    {
                        int fileCount = (int)SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_GETCOUNT, 0, 0);

                        // Выделение файлов с выбранным расширением
                        for (int i = 0; i < fileCount; i++)
                        {
                            char filename[MAX_PATH];
                            SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_GETTEXT, i, (LPARAM)filename);

                            // Пропускаем каталоги
                            if (filename[0] == '[' && strstr(filename, "]"))
                                continue;

                            char fileExt[20];
                            GetFileExtension(filename, fileExt, 20);

                            // Сравнение расширений
                            if (strcmp(selectedExt, "<без расширения>") == 0)
                            {
                                // Проверка на файл без расширения
                                const char* dot = strrchr(filename, '.');
                                if (dot == NULL || dot[1] == '\0')
                                {
                                    SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_SETSEL, TRUE, i);
                                }
                            }
                            else if (strcmp(fileExt, selectedExt) == 0)
                            {
                                SendMessageA(GetDlgItem(hWnd, IDC_LIST_FILES), LB_SETSEL, TRUE, i);
                            }
                        }
                    }
                }
            }
            break;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}