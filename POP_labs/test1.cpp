
#include <windows.h>
#include "resourceLab8.h"

#define CLASS_NAME "F(DSHNLK#NROUFYE(*nlk234nouo3yf98h23n"
#define N 400
#define M 300
#define MIN_WIDTH (int)(1.2 * N)
#define MIN_HEIGHT (int)(1.2 * M)


// Минимальные данные: только буфер пикселей и предыдущая точка
struct UserData {
    BYTE pixelBuffer[N][M];
    POINT prevPoint;
    int drawMethod;
};

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
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);

    if (!RegisterClass(&wc)) return -1;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int indentX = (int)(screenWidth * 0.07);  // 7% от ширины экрана
    int indentY = (int)(screenHeight * 0.07); // 7% от высоты экрана

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab8",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        MIN_WIDTH + GetSystemMetrics(SM_CXSIZEFRAME) * 2,
        MIN_HEIGHT + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) * 2,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);

    UserData userData = { 0 };
    userData.prevPoint.x = -1;
    userData.prevPoint.y = -1;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&userData);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {

    case WM_COMMAND: {

        switch (LOWORD(wParam)) {
        case ID_EXIT: DestroyWindow(hWnd); break;
        }
        return 0;
    }

    case WM_LBUTTONDOWN: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        // Центрируем прямоугольник рисования
        int rectLeft = (clientRect.right - N) / 2;
        int rectTop = (clientRect.bottom - M) / 2;

        // Создаём RECT для области рисования
        RECT drawRect = { rectLeft, rectTop, rectLeft + N, rectTop + M };

        // Проверяем попадание точки в прямоугольник через PtInRect
        POINT pt = { x, y };
        if (PtInRect(&drawRect, pt)) {

            int bufX = x - rectLeft;
            int bufY = y - rectTop;

            if (bufX >= 0 && bufX < N && bufY >= 0 && bufY < M) {
                if (pUserData->drawMethod == 0) {
                    pUserData->pixelBuffer[bufX][bufY] = 1;
                    HDC hdc = GetDC(hWnd);
                    SetPixel(hdc, x, y, RGB(0, 0, 0));
                    ReleaseDC(hWnd, hdc);
                }

                pUserData->prevPoint.x = bufX;
                pUserData->prevPoint.y = bufY;
            }

            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_LBUTTONUP: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        pUserData->prevPoint.x = -1;
        pUserData->prevPoint.y = -1;
        break;
    }

    case WM_MOUSEMOVE: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (wParam & MK_LBUTTON) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            // Центрируем прямоугольник рисования
            int rectLeft = (clientRect.right - N) / 2;
            int rectTop = (clientRect.bottom - M) / 2;

            // Создаём RECT для области рисования
            RECT drawRect = { rectLeft, rectTop, rectLeft + N, rectTop + M };

            // Проверяем попадание точки в прямоугольник через PtInRect
            POINT pt = { x, y };
            if (PtInRect(&drawRect, pt)) {
                int bufX = x - rectLeft;
                int bufY = y - rectTop;

                if (bufX >= 0 && bufX < N && bufY >= 0 && bufY < M) {
                    if (pUserData->drawMethod == 0) {
                        pUserData->pixelBuffer[bufX][bufY] = 1;
                        HDC hdc = GetDC(hWnd);
                        SetPixel(hdc, x, y, RGB(0, 0, 0));
                        ReleaseDC(hWnd, hdc);
                    }
                    else if (pUserData->drawMethod == 1 && pUserData->prevPoint.x != -1) {
                        int dx = abs(bufX - pUserData->prevPoint.x);
                        int dy = abs(bufY - pUserData->prevPoint.y);
                        int sx = (pUserData->prevPoint.x < bufX) ? 1 : -1;
                        int sy = (pUserData->prevPoint.y < bufY) ? 1 : -1;
                        int err = dx - dy; //вычисление ошибки для выбора направления движения

                        while (1) {
                            if (pUserData->prevPoint.x >= 0 && pUserData->prevPoint.x < N && pUserData->prevPoint.y >= 0 && pUserData->prevPoint.y < M) {
                                pUserData->pixelBuffer[pUserData->prevPoint.x][pUserData->prevPoint.y] = 1;
                            }

                            if (pUserData->prevPoint.x == bufX && pUserData->prevPoint.y == bufY) break;

                            int e2 = 2 * err; //для проверки допустимости размера ошибки
                            if (e2 > -dy) {
                                err -= dy;
                                pUserData->prevPoint.x += sx;
                            }
                            if (e2 < dx) {
                                err += dx;
                                pUserData->prevPoint.y += sy;
                            }
                        }

                        HDC hdc = GetDC(hWnd);
                        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

                        MoveToEx(hdc,
                            pUserData->prevPoint.x + rectLeft,
                            pUserData->prevPoint.y + rectTop,
                            NULL);
                        LineTo(hdc, x, y);

                        SelectObject(hdc, hOldPen);
                        DeleteObject(hPen);
                        ReleaseDC(hWnd, hdc); //узнать что делает
                    }

                    pUserData->prevPoint.x = bufX;
                    pUserData->prevPoint.y = bufY;

                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
        }
        break;
    }

    case WM_PAINT: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        // Центрируем прямоугольник рисования
        int rectLeft = (clientRect.right - N) / 2;
        int rectTop = (clientRect.bottom - M) / 2;

        // Рамка прямоугольника
        HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, rectLeft, rectTop, rectLeft + N, rectTop + M);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBorderPen);

        // Пиксели из буфера
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (pUserData->pixelBuffer[i][j]) {
                    SetPixel(hdc, rectLeft + i, rectTop + j, RGB(0, 0, 0));
                }
            }
        }

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SIZE: {
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }

    case WM_SIZING: {
        RECT* pRect = (RECT*)lParam;

        if (pRect->right - pRect->left < MIN_WIDTH) {
            switch (wParam) {
            case WMSZ_LEFT:
            case WMSZ_TOPLEFT:
            case WMSZ_BOTTOMLEFT:
                pRect->left = pRect->right - MIN_WIDTH;
                break;
            default:
                pRect->right = pRect->left + MIN_WIDTH;
                break;
            }
        }

        if (pRect->bottom - pRect->top < MIN_HEIGHT) {
            switch (wParam) {
            case WMSZ_TOP:
            case WMSZ_TOPLEFT:
            case WMSZ_TOPRIGHT:
                pRect->top = pRect->bottom - MIN_HEIGHT;
                break;
            default:
                pRect->bottom = pRect->top + MIN_HEIGHT;
                break;
            }
        }
        break;
    }
    case WM_KEYDOWN: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (wParam == VK_SPACE) {
            pUserData->drawMethod = 1 - pUserData->drawMethod;

            if (pUserData->drawMethod == 0) {
                SetWindowText(hWnd, "Рисование - Метод: Точка (Пробел-смена, Esc-очистка)");
            }
            else {
                SetWindowText(hWnd, "Рисование - Метод: Линия (Пробел-смена, Esc-очистка)");
            }
        }
        else if (wParam == VK_ESCAPE) {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    pUserData->pixelBuffer[i][j] = 0;
                }
            }
            pUserData->prevPoint.x = -1;
            pUserData->prevPoint.y = -1;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
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
