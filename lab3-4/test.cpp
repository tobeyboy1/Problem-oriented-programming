#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>

// Константы через #define
#define FIGURE_COUNT 50
#define FIGURE_SIZE 30
#define SCROLL_STEP 20
#define INTELLIGENT_SCROLL_FACTOR 2
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Структура для хранения данных окна
struct WindowData {
    int scrollX;
    int scrollY;
    int contentWidth;
    int contentHeight;
    bool showCircles;
    std::vector<POINT> figurePositions;
    
    WindowData() : scrollX(0), scrollY(0), contentWidth(2000), contentHeight(1500), showCircles(true) {}
};

// Прототипы функций
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitializeFigures(WindowData* data);
void DrawContent(HDC hdc, WindowData* data, const RECT& clientRect);
void HandleScroll(HWND hWnd, WindowData* data, int dx, int dy, bool intelligent = false);
void UpdateScrollInfo(HWND hWnd, WindowData* data);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = "SmartScrollApp";
    
    RegisterClassEx(&wcex);
    
    // Создание окна
    HWND hWnd = CreateWindow(
        "SmartScrollApp", "Интеллектуальная прокрутка с фигурами", 
        WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hWnd) return FALSE;
    
    // Инициализация генератора случайных чисел
    srand(static_cast<unsigned>(time(nullptr)));
    
    // Создание и инициализация данных окна
    WindowData* data = new WindowData();
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    InitializeFigures(data);
    UpdateScrollInfo(hWnd, data);
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    delete data;
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    WindowData* data = reinterpret_cast<WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    
    switch (message) {
        case WM_CREATE: {
            // Создание меню
            HMENU hMenu = CreateMenu();
            HMENU hSubMenu = CreatePopupMenu();
            
            AppendMenu(hSubMenu, MF_STRING, 1, "Показать круги");
            AppendMenu(hSubMenu, MF_STRING, 2, "Показать квадраты");
            AppendMenu(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), "Фигуры");
            
            SetMenu(hWnd, hMenu);
            break;
        }
        
        case WM_COMMAND: {
            // Обработка команд меню
            switch (LOWORD(wParam)) {
                case 1: // Круги
                    data->showCircles = true;
                    InvalidateRect(hWnd, nullptr, TRUE);
                    break;
                case 2: // Квадраты
                    data->showCircles = false;
                    InvalidateRect(hWnd, nullptr, TRUE);
                    break;
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            
            // Сохраняем оригинальные координаты
            SetViewportOrgEx(hdc, -data->scrollX, -data->scrollY, nullptr);
            
            DrawContent(hdc, data, clientRect);
            
            EndPaint(hWnd, &ps);
            break;
        }
        
        case WM_VSCROLL: {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            // Вертикальная прокрутка
            int dy = 0;
            switch (LOWORD(wParam)) {
                case SB_LINEUP: dy = -SCROLL_STEP; break;
                case SB_LINEDOWN: dy = SCROLL_STEP; break;
                case SB_PAGEUP: dy = -clientRect.bottom; break;
                case SB_PAGEDOWN: dy = clientRect.bottom; break;
                case SB_THUMBTRACK: 
                    dy = HIWORD(wParam) - data->scrollY; 
                    break;
            }
            if (dy != 0) {
                HandleScroll(hWnd, data, 0, dy);
            }
            break;
        }
        
        case WM_HSCROLL: {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            // Горизонтальная прокрутка
            int dx = 0;
            switch (LOWORD(wParam)) {
                case SB_LINELEFT: dx = -SCROLL_STEP; break;
                case SB_LINERIGHT: dx = SCROLL_STEP; break;
                case SB_PAGELEFT: dx = -clientRect.right; break;
                case SB_PAGERIGHT: dx = clientRect.right; break;
                case SB_THUMBTRACK: 
                    dx = HIWORD(wParam) - data->scrollX; 
                    break;
            }
            if (dx != 0) {
                HandleScroll(hWnd, data, dx, 0);
            }
            break;
        }
        
        case WM_MOUSEWHEEL: {
            // Интеллектуальная прокрутка колесом мыши
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int scrollDelta = delta / WHEEL_DELTA * SCROLL_STEP * INTELLIGENT_SCROLL_FACTOR;
            
            if (GetKeyState(VK_SHIFT) & 0x8000) {
                // Горизонтальная прокрутка при зажатом Shift
                HandleScroll(hWnd, data, scrollDelta, 0, true);
            } else {
                // Вертикальная прокрутка
                HandleScroll(hWnd, data, 0, scrollDelta, true);
            }
            break;
        }
        
        case WM_KEYDOWN: {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            // Клавиатурный интерфейс для скроллинга
            switch (wParam) {
                case VK_UP: 
                    HandleScroll(hWnd, data, 0, -SCROLL_STEP); 
                    break;
                case VK_DOWN: 
                    HandleScroll(hWnd, data, 0, SCROLL_STEP); 
                    break;
                case VK_LEFT: 
                    HandleScroll(hWnd, data, -SCROLL_STEP, 0); 
                    break;
                case VK_RIGHT: 
                    HandleScroll(hWnd, data, SCROLL_STEP, 0); 
                    break;
                case VK_PRIOR: // Page Up
                    HandleScroll(hWnd, data, 0, -clientRect.bottom); 
                    break;
                case VK_NEXT: // Page Down
                    HandleScroll(hWnd, data, 0, clientRect.bottom); 
                    break;
                case VK_HOME: 
                    HandleScroll(hWnd, data, -data->scrollX, -data->scrollY); 
                    break;
                case VK_END: 
                    HandleScroll(hWnd, data, 
                        data->contentWidth - clientRect.right - data->scrollX, 
                        data->contentHeight - clientRect.bottom - data->scrollY); 
                    break;
            }
            break;
        }
        
        case WM_SIZE: {
            if (data) {
                UpdateScrollInfo(hWnd, data);
            }
            break;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void InitializeFigures(WindowData* data) {
    data->figurePositions.clear();
    for (int i = 0; i < FIGURE_COUNT; i++) {
        POINT pos;
        pos.x = rand() % (data->contentWidth - FIGURE_SIZE);
        pos.y = rand() % (data->contentHeight - FIGURE_SIZE);
        data->figurePositions.push_back(pos);
    }
}

void DrawContent(HDC hdc, WindowData* data, const RECT& clientRect) {
    // Создаем кисти для разных фигур
    HBRUSH hCircleBrush = CreateSolidBrush(RGB(255, 0, 0));    // Красный для кругов
    HBRUSH hSquareBrush = CreateSolidBrush(RGB(0, 0, 255));    // Синий для квадратов
    HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(240, 240, 240)); // Светло-серый фон
    
    // Заливаем фон
    RECT backgroundRect = {0, 0, data->contentWidth, data->contentHeight};
    FillRect(hdc, &backgroundRect, hBackgroundBrush);
    
    HBRUSH hCurrentBrush = data->showCircles ? hCircleBrush : hSquareBrush;
    SelectObject(hdc, hCurrentBrush);
    
    // Рисуем фигуры
    for (const auto& pos : data->figurePositions) {
        if (data->showCircles) {
            // Рисуем круг
            Ellipse(hdc, pos.x, pos.y, pos.x + FIGURE_SIZE, pos.y + FIGURE_SIZE);
        } else {
            // Рисуем квадрат
            Rectangle(hdc, pos.x, pos.y, pos.x + FIGURE_SIZE, pos.y + FIGURE_SIZE);
        }
    }
    
    // Освобождаем ресурсы
    DeleteObject(hCircleBrush);
    DeleteObject(hSquareBrush);
    DeleteObject(hBackgroundBrush);
}

void HandleScroll(HWND hWnd, WindowData* data, int dx, int dy, bool intelligent) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    
    int oldScrollX = data->scrollX;
    int oldScrollY = data->scrollY;
    
    // Применяем интеллектуальный множитель если нужно
    if (intelligent) {
        dx *= INTELLIGENT_SCROLL_FACTOR;
        dy *= INTELLIGENT_SCROLL_FACTOR;
    }
    
    // Обновляем позицию прокрутки с проверкой границ
    data->scrollX = max(0, min(data->contentWidth - clientRect.right, data->scrollX + dx));
    data->scrollY = max(0, min(data->contentHeight - clientRect.bottom, data->scrollY + dy));
    
    // Если позиция изменилась, обновляем отображение
    if (data->scrollX != oldScrollX || data->scrollY != oldScrollY) {
        UpdateScrollInfo(hWnd, data);
        
        // Прокручиваем содержимое
        ScrollWindowEx(hWnd, oldScrollX - data->scrollX, oldScrollY - data->scrollY, 
                      nullptr, nullptr, nullptr, nullptr, SW_INVALIDATE);
        UpdateWindow(hWnd);
    }
}

void UpdateScrollInfo(HWND hWnd, WindowData* data) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    
    // Обновляем информацию о вертикальной прокрутке
    SCROLLINFO siVert = {};
    siVert.cbSize = sizeof(SCROLLINFO);
    siVert.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    siVert.nMin = 0;
    siVert.nMax = data->contentHeight;
    siVert.nPage = clientRect.bottom;
    siVert.nPos = data->scrollY;
    SetScrollInfo(hWnd, SB_VERT, &siVert, TRUE);
    
    // Обновляем информацию о горизонтальной прокрутке
    SCROLLINFO siHorz = {};
    siHorz.cbSize = sizeof(SCROLLINFO);
    siHorz.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    siHorz.nMin = 0;
    siHorz.nMax = data->contentWidth;
    siHorz.nPage = clientRect.right;
    siHorz.nPos = data->scrollX;
    SetScrollInfo(hWnd, SB_HORZ, &siHorz, TRUE);
}