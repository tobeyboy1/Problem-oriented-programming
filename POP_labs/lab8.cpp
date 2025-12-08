#include <windows.h>

// Константы
#define N 400
#define M 300
#define CLASS_NAME "DrawingWindowClass"
#define MIN_WIDTH (int)(1.2 * N)
#define MIN_HEIGHT (int)(1.2 * M)

// Структура для хранения данных (без динамических массивов)
struct DrawingData {
    BYTE pixelBuffer[N][M];  // Двумерный массив как в подсказке
    POINT prevPoint;
    int drawMethod;
    int rectLeft;
    int rectTop;
};

// Функция для проверки попадания точки в прямоугольник
BOOL IsPointInRect(int x, int y, int rectLeft, int rectTop) {
    return (x >= rectLeft && x < rectLeft + N &&
        y >= rectTop && y < rectTop + M);
}

// Функция для обновления позиции прямоугольника рисования
void UpdateRectPosition(DrawingData* data, int clientWidth, int clientHeight) {
    data->rectLeft = (clientWidth - N) / 2;
    data->rectTop = (clientHeight - M) / 2;
}

// Функция рисования линии (алгоритм Брезенхема)
void DrawLineToBuffer(DrawingData* data, int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x1 >= 0 && x1 < N && y1 >= 0 && y1 < M) {
            data->pixelBuffer[x1][y1] = 1;
        }

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Основная процедура окна
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    DrawingData* data = (DrawingData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message) {
    case WM_CREATE: {
        // Выделяем память для данных
        data = (DrawingData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DrawingData));
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);

        // Инициализация данных
        data->prevPoint.x = -1;
        data->prevPoint.y = -1;
        data->drawMethod = 0;  // 0 - точка, 1 - линия

        // Создаем меню
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        AppendMenu(hFileMenu, MF_STRING, 1, "Выход");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "Файл");
        SetMenu(hWnd, hMenu);

        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        if (data) {
            // Получаем координаты мыши напрямую из lParam
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (IsPointInRect(x, y, data->rectLeft, data->rectTop)) {

                // Конвертируем координаты в координаты буфера
                int bufX = x - data->rectLeft;
                int bufY = y - data->rectTop;

                if (bufX >= 0 && bufX < N && bufY >= 0 && bufY < M) {
                    if (data->drawMethod == 0) {
                        // Метод 1: Точка
                        data->pixelBuffer[bufX][bufY] = 1;

                        HDC hdc = GetDC(hWnd);
                        SetPixel(hdc, x, y, RGB(0, 0, 0));
                        ReleaseDC(hWnd, hdc);
                    }

                    // Сохраняем точку для метода с линией
                    data->prevPoint.x = bufX;
                    data->prevPoint.y = bufY;
                }

                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (data) {
            data->prevPoint.x = -1;
            data->prevPoint.y = -1;
        }
        break;
    }

    case WM_MOUSEMOVE: {
        if (data && (wParam & MK_LBUTTON)) {
            // Получаем координаты мыши напрямую из lParam
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (IsPointInRect(x, y, data->rectLeft, data->rectTop)) {
                // Конвертируем координаты в координаты буфера
                int bufX = x - data->rectLeft;
                int bufY = y - data->rectTop;

                if (bufX >= 0 && bufX < N && bufY >= 0 && bufY < M) {
                    if (data->drawMethod == 0) {
                        // Метод 1: Точка
                        data->pixelBuffer[bufX][bufY] = 1;

                        HDC hdc = GetDC(hWnd);
                        SetPixel(hdc, x, y, RGB(0, 0, 0));
                        ReleaseDC(hWnd, hdc);
                    }
                    else if (data->drawMethod == 1 && data->prevPoint.x != -1) {
                        // Метод 2: Линия (линейная интерполяция)
                        DrawLineToBuffer(data, data->prevPoint.x, data->prevPoint.y, bufX, bufY);

                        // Рисуем линию на экране
                        HDC hdc = GetDC(hWnd);
                        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

                        MoveToEx(hdc,
                            data->prevPoint.x + data->rectLeft,
                            data->prevPoint.y + data->rectTop,
                            NULL);
                        LineTo(hdc, x, y);

                        SelectObject(hdc, hOldPen);
                        DeleteObject(hPen);
                        ReleaseDC(hWnd, hdc);
                    }

                    // Сохраняем текущую точку как предыдущую
                    data->prevPoint.x = bufX;
                    data->prevPoint.y = bufY;

                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (data) {
            // Получаем размеры клиентской области
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            UpdateRectPosition(data, clientRect.right, clientRect.bottom);

            // Рисуем рамку прямоугольника
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));

            Rectangle(hdc,
                data->rectLeft,
                data->rectTop,
                data->rectLeft + N,
                data->rectTop + M);

            // Восстанавливаем перо
            SelectObject(hdc, hOldPen);
            DeleteObject(hBorderPen);

            // Рисуем пиксели из буфера
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    if (data->pixelBuffer[i][j]) {
                        SetPixel(hdc,
                            data->rectLeft + i,
                            data->rectTop + j,
                            RGB(0, 0, 0));
                    }
                }
            }
        }

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SIZE: {
        if (data) {
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    }

    case WM_SIZING: {
        RECT* pRect = (RECT*)lParam;

        // Ограничиваем минимальный размер окна
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
        if (data && wParam == VK_SPACE) {
            // Переключение метода рисования по пробелу
            data->drawMethod = 1 - data->drawMethod;

            // Очистка предыдущей точки при смене метода
            data->prevPoint.x = -1;
            data->prevPoint.y = -1;

            // Обновление заголовка окна
            if (data->drawMethod == 0) {
                SetWindowText(hWnd, "Рисование в ограниченной области - Метод: Точка");
            }
            else {
                SetWindowText(hWnd, "Рисование в ограниченной области - Метод: Линия");
            }
        }
        else if (wParam == VK_ESCAPE) {
            // Очистка буфера по Escape
            if (data) {
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        data->pixelBuffer[i][j] = 0;
                    }
                }
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        break;
    }

    case WM_DESTROY: {
        if (data) {
            HeapFree(GetProcessHeap(), 0, data);
        }
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Точка входа
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Создание окна с начальным размером больше минимального
    HWND hWnd = CreateWindow(
        CLASS_NAME,
        "Рисование в ограниченной области - Метод: Точка (Пробел - сменить метод, Esc - очистить)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        MIN_WIDTH + GetSystemMetrics(SM_CXSIZEFRAME) * 2,
        MIN_HEIGHT + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) * 2,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}