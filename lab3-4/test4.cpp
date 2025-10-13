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

struct UserData {
    std::vector<Circle> g_circles;
    int g_scrollX;
    int g_scrollY;
    int oldClientWidth;    // Добавить это
    int oldClientHeight;   // И это
    HMENU hMenu;
};

COLORREF GetRandomColor() {
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

void UpdateScrollBars(HWND hWnd, UserData* pUserData) {

    //UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;

    // Вертикальная прокрутка
    si.nMin = 0;
    si.nMax = CONTENT_HEIGHT;
    si.nPage = clientHeight;
    si.nPos = pUserData->g_scrollY;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

    // Горизонтальная прокрутка
    si.nMin = 0;
    si.nMax = CONTENT_WIDTH;
    si.nPage = clientWidth;
    si.nPos = pUserData->g_scrollX;
    SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {

        srand(time(NULL));

        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        UserData* pUserData = (UserData*)(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUserData);

        pUserData->g_scrollX = 0;
        pUserData->g_scrollY = 0;
        pUserData->oldClientWidth = WINDOW_WIDTH;     // Инициализация
        pUserData->oldClientHeight = WINDOW_HEIGHT;

        pUserData->g_circles.reserve(CIRCLE_COUNT);

        for (int i = 0; i < CIRCLE_COUNT; i++) {
            Circle circle;
            circle.x = rand() % CONTENT_WIDTH;
            circle.y = rand() % CONTENT_HEIGHT;
            circle.color = GetRandomColor();

            circle.bounds.left = circle.x - CIRCLE_RADIUS;
            circle.bounds.top = circle.y - CIRCLE_RADIUS;
            circle.bounds.right = circle.x + CIRCLE_RADIUS;
            circle.bounds.bottom = circle.y + CIRCLE_RADIUS;

            pUserData->g_circles.push_back(circle);
        }

        //UpdateScrollBars(hWnd);

        // Создаем меню
        pUserData->hMenu = CreateMenu();
        HMENU hShapeMenu = CreatePopupMenu();

        // Добавляем пункты в подменю "Фигуры"
        AppendMenu(hShapeMenu, MF_STRING | MF_GRAYED, IDM_CIRCLE, "Круги");
        AppendMenu(hShapeMenu, MF_STRING, IDM_SQUARE, "Квадраты");

        // Добавляем подменю в главное меню
        AppendMenu(pUserData->hMenu, MF_POPUP, (UINT_PTR)hShapeMenu, "Фигуры");

        // Устанавливаем меню для окна
        SetMenu(hWnd, pUserData->hMenu);

        InvalidateRect(hWnd, NULL, TRUE);
        // Обновляем состояние меню
        //UpdateMenuState();
        break;
    }
    case WM_PAINT: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Заливаем фон области перерисовки
        HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &ps.rcPaint, hBackgroundBrush);
        DeleteObject(hBackgroundBrush);

        // Рисуем только видимые фигуры
        // Вычисляем видимую область в координатах контента
        RECT visibleRect;
        visibleRect.left = ps.rcPaint.left + pUserData->g_scrollX;
        visibleRect.top = ps.rcPaint.top + pUserData->g_scrollY;
        visibleRect.right = ps.rcPaint.right + pUserData->g_scrollX;
        visibleRect.bottom = ps.rcPaint.bottom + pUserData->g_scrollY;

        for (const Circle& circle : pUserData->g_circles) {
            RECT intersection;
            if (IntersectRect(&intersection, &circle.bounds, &visibleRect)) {
                // Вычисляем координаты фигуры с учетом прокрутки
                int screenX = circle.x - pUserData->g_scrollX;
                int screenY = circle.y - pUserData->g_scrollY;

                HBRUSH hBrush = CreateSolidBrush(circle.color);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

                if (GetMenuState(GetSubMenu(pUserData->hMenu, 0), IDM_CIRCLE, MF_BYCOMMAND) == MF_GRAYED) {
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

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_COMMAND: {

        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;

        switch (LOWORD(wParam)) {
        case IDM_CIRCLE:

            // Устанавливаем состояние для "Круги"
            mii.fState = MF_GRAYED;
            SetMenuItemInfo(pUserData->hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MF_ENABLED;
            SetMenuItemInfo(pUserData->hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hWnd, NULL, TRUE); // Перерисовываем окно
            break;

        case IDM_SQUARE:

            // Устанавливаем состояние для "Квадраты"
            mii.fState = MF_ENABLED;
            SetMenuItemInfo(pUserData->hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MF_GRAYED;
            SetMenuItemInfo(pUserData->hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hWnd, NULL, TRUE); // Перерисовываем окно
            break;
        }
        break;
    }
    case WM_VSCROLL: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int clientHeight = clientRect.bottom - clientRect.top;

        int oldScrollY = pUserData->g_scrollY;

        switch (LOWORD(wParam)) {
        case SB_LINEUP: pUserData->g_scrollY -= SCROLL_STEP; break;
        case SB_LINEDOWN: pUserData->g_scrollY += SCROLL_STEP; break;
        case SB_PAGEUP: pUserData->g_scrollY -= clientHeight; break;
        case SB_PAGEDOWN: pUserData->g_scrollY += clientHeight; break;
        case SB_THUMBTRACK: pUserData->g_scrollY = HIWORD(wParam); break;
        case SB_TOP: pUserData->g_scrollY = 0; break;
        case SB_BOTTOM: pUserData->g_scrollY = CONTENT_HEIGHT - clientHeight; break;
        }

        pUserData->g_scrollY = max(0, min(pUserData->g_scrollY, CONTENT_HEIGHT - clientHeight));

        if (pUserData->g_scrollY != oldScrollY) {
            int deltaY = oldScrollY - pUserData->g_scrollY;
            ScrollWindowEx(hWnd, 0, deltaY, NULL, NULL, NULL, NULL, SW_INVALIDATE);
            UpdateScrollBars(hWnd, pUserData);
        }
        break;
    }

    case WM_HSCROLL: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int clientWidth = clientRect.right - clientRect.left;

        int oldScrollX = pUserData->g_scrollX;

        switch (LOWORD(wParam)) {
        case SB_LINELEFT: pUserData->g_scrollX -= SCROLL_STEP; break;
        case SB_LINERIGHT: pUserData->g_scrollX += SCROLL_STEP; break;
        case SB_THUMBTRACK: pUserData->g_scrollX = HIWORD(wParam); break;
        }

        pUserData->g_scrollX = max(0, min(pUserData->g_scrollX, CONTENT_WIDTH - clientWidth));

        if (pUserData->g_scrollX != oldScrollX) {
            int deltaX = oldScrollX - pUserData->g_scrollX;
            ScrollWindowEx(hWnd, deltaX, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
            UpdateScrollBars(hWnd, pUserData);
        }
        break;
    }

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_UP:
            PostMessage(hWnd, WM_VSCROLL, MAKELONG(SB_LINEUP, 0), 0L);
            break;

        case VK_PRIOR:
            PostMessage(hWnd, WM_VSCROLL, MAKELONG(SB_PAGEUP, 0), 0L);
            break;

        case VK_NEXT:
            PostMessage(hWnd, WM_VSCROLL, MAKELONG(SB_PAGEDOWN, 0), 0L);
            break;

        case VK_DOWN:
            PostMessage(hWnd, WM_VSCROLL, MAKELONG(SB_LINEDOWN, 0), 0L);
            break;

        case VK_LEFT:
            PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_LINELEFT, 0), 0L);
            break;

        case VK_RIGHT:
            PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_LINERIGHT, 0), 0L);
            break;

        case VK_HOME:
            PostMessage(hWnd, WM_VSCROLL, MAKELONG(SB_TOP, 0), 0L);
            break;

        case VK_END:
            PostMessage(hWnd, WM_VSCROLL, MAKELONG(SB_BOTTOM, 0), 0L);
            break;
        }
        break;
    }

    case WM_SIZE: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int newClientWidth = clientRect.right - clientRect.left;
        int newClientHeight = clientRect.bottom - clientRect.top;

        // Просто ограничиваем текущую позицию прокрутки новыми пределами
        int maxScrollX = max(0, CONTENT_WIDTH - newClientWidth);
        int maxScrollY = max(0, CONTENT_HEIGHT - newClientHeight);

        pUserData->g_scrollX = min(pUserData->g_scrollX, maxScrollX);
        pUserData->g_scrollY = min(pUserData->g_scrollY, maxScrollY);

        // Сохраняем текущие размеры
        pUserData->oldClientWidth = newClientWidth;
        pUserData->oldClientHeight = newClientHeight;

        // Обновляем полосы прокрутки
        UpdateScrollBars(hWnd, pUserData);

        // Перерисовываем всё окно
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
    case WM_DESTROY: {

        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
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

    int i = 0;

    do {
        CLASS_NAME[i] = 33 + rand() % 94;
        i++;
        wc.lpszClassName = CLASS_NAME;
    } while (!RegisterClass(&wc) && i < NAME_CAPACITY);

    UserData userData;

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab3-4 - Переключение круги/квадраты",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, &userData
    );

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}