#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>

#define CIRCLE_RADIUS 20
#define CIRCLE_COUNT 5000
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SCROLL_STEP 20
#define NAME_CAPACITY 100
#define CONTENT_WIDTH 2200
#define CONTENT_HEIGHT 2000

// Идентификаторы меню
#define IDM_CIRCLE 1001
#define IDM_SQUARE 1002

struct Circle {
    int x, y;
    COLORREF color;
    RECT bounds;
};

std::vector<Circle> g_circles;
int g_scrollX = 0;
int g_scrollY = 0;
HMENU hMenu;

COLORREF GetRandomColor() {
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

void InitializeCircles(HWND hwnd) {
    //g_circles.clear();
    g_circles.reserve(CIRCLE_COUNT);

    for (int i = 0; i < CIRCLE_COUNT; i++) {
        Circle circle;
        circle.x = rand() % CONTENT_WIDTH;
        circle.y = rand() % CONTENT_HEIGHT;
        circle.color = GetRandomColor();

        circle.bounds.left = circle.x - CIRCLE_RADIUS;
        circle.bounds.top = circle.y - CIRCLE_RADIUS;
        circle.bounds.right = circle.x + CIRCLE_RADIUS;
        circle.bounds.bottom = circle.y + CIRCLE_RADIUS;

        g_circles.push_back(circle);
    }
}

void UpdateScrollBars(HWND hwnd) {
    SCROLLINFO si;

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = CONTENT_HEIGHT;
    si.nPage = WINDOW_HEIGHT;
    si.nPos = g_scrollY;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    si.nMin = 0;
    si.nMax = CONTENT_WIDTH;
    si.nPage = WINDOW_WIDTH;
    si.nPos = g_scrollX;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
}

void DrawVisibleFigures(HDC hdc, const RECT& updateRect) {
    // Вычисляем видимую область в координатах контента
    RECT visibleRect;
    visibleRect.left = updateRect.left + g_scrollX;
    visibleRect.top = updateRect.top + g_scrollY;
    visibleRect.right = updateRect.right + g_scrollX;
    visibleRect.bottom = updateRect.bottom + g_scrollY;

    for (const Circle& circle : g_circles) {
        RECT intersection;
        if (IntersectRect(&intersection, &circle.bounds, &visibleRect)) {
            // Вычисляем координаты фигуры с учетом прокрутки
            int screenX = circle.x - g_scrollX;
            int screenY = circle.y - g_scrollY;

            HBRUSH hBrush = CreateSolidBrush(circle.color);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

            if (GetMenuState(GetSubMenu(hMenu,0), IDM_CIRCLE, MF_BYCOMMAND) == MF_GRAYED) {
                // Рисуем круг
                Ellipse(hdc,
                    screenX - CIRCLE_RADIUS,
                    screenY - CIRCLE_RADIUS,
                    screenX + CIRCLE_RADIUS,
                    screenY + CIRCLE_RADIUS);
            }
            else {
                // Рисуем квадрат
                Rectangle(hdc,
                    screenX - CIRCLE_RADIUS,
                    screenY - CIRCLE_RADIUS,
                    screenX + CIRCLE_RADIUS,
                    screenY + CIRCLE_RADIUS);
            }

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBrush);
            DeleteObject(hPen);
        }
    }
}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        srand(static_cast<unsigned>(time(NULL)));
        InitializeCircles(hwnd);
        UpdateScrollBars(hwnd);

        // Создаем меню
        hMenu = CreateMenu();
        HMENU hShapeMenu = CreatePopupMenu();

        // Добавляем пункты в подменю "Фигуры"
        AppendMenu(hShapeMenu, MF_STRING | MF_GRAYED, IDM_CIRCLE, "Круги");
        AppendMenu(hShapeMenu, MF_STRING, IDM_SQUARE, "Квадраты");

        // Добавляем подменю в главное меню
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hShapeMenu, "Фигуры");

        // Устанавливаем меню для окна
        SetMenu(hwnd, hMenu);

        // Обновляем состояние меню
        //UpdateMenuState();
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Заливаем фон области перерисовки
        HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &ps.rcPaint, hBackgroundBrush);
        DeleteObject(hBackgroundBrush);

        // Рисуем только видимые фигуры
        DrawVisibleFigures(hdc, ps.rcPaint);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_COMMAND:
        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;

        switch (LOWORD(wParam)) {
        case IDM_CIRCLE:

            // Устанавливаем состояние для "Круги"
            mii.fState =  MF_GRAYED;
            SetMenuItemInfo(hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MF_ENABLED;
            SetMenuItemInfo(hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hwnd, NULL, TRUE); // Перерисовываем окно
            break;

        case IDM_SQUARE:

            // Устанавливаем состояние для "Квадраты"
            mii.fState = MF_ENABLED;
            SetMenuItemInfo(hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MF_GRAYED;
            SetMenuItemInfo(hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hwnd, NULL, TRUE); // Перерисовываем окно
            break;
        }
        break;

    case WM_VSCROLL: {
        int oldScrollY = g_scrollY;

        switch (LOWORD(wParam)) {
        case SB_LINEUP: g_scrollY -= SCROLL_STEP; break;
        case SB_LINEDOWN: g_scrollY += SCROLL_STEP; break;
        case SB_PAGEUP: g_scrollY -= WINDOW_HEIGHT; break;
        case SB_PAGEDOWN: g_scrollY += WINDOW_HEIGHT; break;
        case SB_THUMBTRACK: g_scrollY = HIWORD(wParam); break;
        case SB_TOP: g_scrollY = 0; break;
        case SB_BOTTOM: g_scrollY = CONTENT_WIDTH - WINDOW_WIDTH; break;
        }

        g_scrollY = max(0, min(g_scrollY, CONTENT_HEIGHT - WINDOW_HEIGHT));

        if (g_scrollY != oldScrollY) {
            int deltaY = oldScrollY - g_scrollY;
            ScrollWindowEx(hwnd, 0, deltaY, NULL, NULL, NULL, NULL, SW_INVALIDATE);
            UpdateScrollBars(hwnd);
        }
        break;
    }

    case WM_HSCROLL: {
        int oldScrollX = g_scrollX;

        switch (LOWORD(wParam)) {
        case SB_LINELEFT: g_scrollX -= SCROLL_STEP; break;
        case SB_LINERIGHT: g_scrollX += SCROLL_STEP; break;
        case SB_THUMBTRACK: g_scrollX = HIWORD(wParam); break;
        }

        g_scrollX = max(0, min(g_scrollX, CONTENT_WIDTH - WINDOW_WIDTH));

        if (g_scrollX != oldScrollX) {
            int deltaX = oldScrollX - g_scrollX;
            ScrollWindowEx(hwnd, deltaX, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
            UpdateScrollBars(hwnd);
        }
        break;
    }

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_UP:
            PostMessage(hwnd, WM_VSCROLL, MAKELONG(SB_LINEUP, 0), 0L);
            break;

        case VK_PRIOR:
            PostMessage(hwnd, WM_VSCROLL, MAKELONG(SB_PAGEUP, 0), 0L);
            break;

        case VK_NEXT:
            PostMessage(hwnd, WM_VSCROLL, MAKELONG(SB_PAGEDOWN, 0), 0L);
            break;

        case VK_DOWN:
            PostMessage(hwnd, WM_VSCROLL, MAKELONG(SB_LINEDOWN, 0), 0L);
            break;

        case VK_LEFT:
            PostMessage(hwnd, WM_HSCROLL, MAKELONG(SB_LINELEFT, 0), 0L);
            break;

        case VK_RIGHT:
            PostMessage(hwnd, WM_HSCROLL, MAKELONG(SB_LINERIGHT, 0), 0L);
            break;

        case VK_HOME:
            PostMessage(hwnd, WM_VSCROLL, MAKELONG(SB_TOP, 0), 0L);
            break;

        case VK_END:
            PostMessage(hwnd, WM_VSCROLL, MAKELONG(SB_BOTTOM, 0), 0L);
            break;
        }
        break;
    }

    case WM_SIZE:
        UpdateScrollBars(hwnd);
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    char CLASS_NAME[NAME_CAPACITY]{};

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    srand(static_cast<unsigned>(time(NULL)));

    int i = 0;
    do {
        CLASS_NAME[i] = 33 + rand() % 94;
        i++;
        wc.lpszClassName = CLASS_NAME;
    } while (!RegisterClass(&wc) && i < NAME_CAPACITY);

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab3-4 - Переключение круги/квадраты",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}