#define STRICT //строга€ проверка типов
#define WIN32_LEAN_AND_MEAN // исключает редко используемые заголовки Windows

#include <Windows.h>
#include <cmath>
#include <time.h>

#define NAME_CAPACITY 100
#define FIGURE_COUNT 50
#define FIGURE_SIZE 30
#define SCROLL_STEP 20
#define INTELLIGENT_SCROLL_FACTOR 2
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow //отвечает за показ свЄрнутого/развЄрнутого окна)
)
{

    char CLASS_NAME[NAME_CAPACITY]{};
    //char* name;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    srand(time(NULL));

    int i = 0;

    do {
        CLASS_NAME[i] += 33 + rand() % 94;
        i++;

        wc.lpszClassName = CLASS_NAME;

    } while (!RegisterClass(&wc) && i<NAME_CAPACITY);


    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab3-4",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL //&UserData
    );

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);
    //UpdateWindow(hWnd);  //можно вернуть чтобы при по€влении окна оно сразу отобразилось корректно

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {

    case WM_SIZE:

        // Retrieve the dimensions of the client area. 
        yClient = HIWORD(lParam);
        xClient = LOWORD(lParam);

        // Set the vertical scrolling range and page size
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = LINES - 1;
        si.nPage = yClient / yChar;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        // Set the horizontal scrolling range and page size. 
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = 2 + xClientMax / xChar;
        si.nPage = xClient / xChar;
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

        return 0;

    case WM_PAINT: {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        InvalidateRect(hWnd, NULL, TRUE);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_VSCROLL: {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        // ¬ертикальна€ прокрутка
        int dy = 0;
        switch (LOWORD(wParam)) {
        case SB_LINEUP:
            dy = -SCROLL_STEP;
            break;
        case SB_LINEDOWN: dy = SCROLL_STEP; break;
        case SB_PAGEUP: dy = -clientRect.bottom; break;
        case SB_PAGEDOWN: dy = clientRect.bottom; break;
        /*case SB_THUMBTRACK:
            dy = HIWORD(wParam) - data->scrollY;
            break;
        }
        if (dy != 0) {
            HandleScroll(hWnd, data, 0, dy);
        }*/
        return 0;
    }

    case WM_SIZE:
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;

    case WM_DESTROY: {

        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}