
#include <windows.h>
#include "resourceLab3-6.h"

#define CLASS_NAME "F(DSHNLK#NROUFYE(*nlk234nouo3yf98h23n"
#define MAX_IDS IDS_STRING107+16

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
)
{

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) return -1;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int indentX = (int)(screenWidth * 0.07);  // 7% от ширины экрана
    int indentY = (int)(screenHeight * 0.07); // 7% от высоты экрана

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab6",
        WS_OVERLAPPEDWINDOW,
        indentX,
        indentY,
        screenWidth - 2 * indentX,
        screenHeight - 2 * indentY,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);


    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {

    case WM_CREATE: {
        
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

        HBITMAP hBMP1 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));

        HBRUSH hBr1 = CreatePatternBrush(hBMP1);
        DeleteObject(hBMP1); //load объекты можно не удалять

        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBr1);
        return 0;
    }
    case WM_COMMAND: {

        switch (LOWORD(wParam)) {

        case ID_BITMAP1: {
            HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE); //можно было обернуть в ф-ю 

            HBITMAP hBMP1 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));

            HBRUSH hBr1 = CreatePatternBrush(hBMP1);
            DeleteObject(hBMP1);

            HBRUSH hOldBrush = (HBRUSH)GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);

            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBr1); //возвращает старую кисть
            DeleteObject(hOldBrush); //можно передать сразу SetClassLongPtr

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        case ID_BITMAP2: {
            HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

            HBITMAP hBMP2 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP2));

            HBRUSH hBr2 = CreatePatternBrush(hBMP2);
            DeleteObject(hBMP2);

            HBRUSH hOldBrush = (HBRUSH)GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);

            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBr2);
            DeleteObject(hOldBrush);

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        default: {
            if (LOWORD(wParam) >= IDS_STRING107 && LOWORD(wParam) < MAX_IDS) {
                HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
                TCHAR sz[160];
                LoadString(hInstance, LOWORD(wParam), sz, sizeof(sz) / sizeof(TCHAR));
                SetWindowText(hWnd, sz);
            }
        }
        }
        return 0;
    }

    case WM_PAINT: {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_RBUTTONUP: {

        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

        HMENU hContextMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_CONTEXT_MENU));

        HMENU hSubMenu = GetSubMenu(hContextMenu, 0); //получение подменю из ресурсов


        for (int i = MAX_IDS; i >= IDS_STRING107; i--)
        {
            TCHAR sz[160];
            if (LoadString(hInstance, i, sz, sizeof(sz) / sizeof(TCHAR)) > 0) {
                InsertMenu(hSubMenu, 0, MF_BYPOSITION | MF_STRING, i, sz);
            }
        }

        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ClientToScreen(hWnd, &pt);

        TrackPopupMenu(hSubMenu, TPM_CENTERALIGN, pt.x, pt.y, 0, hWnd, NULL);

        DestroyMenu(hContextMenu);
        return 0;
    }

    case WM_DESTROY: {

        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}