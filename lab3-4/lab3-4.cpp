#include <windows.h>
#include <time.h>
//Проверка
#define NAME_CAPACITY 100

#define CIRCLE_RADIUS 20
#define CIRCLE_COUNT 5000
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SCROLL_STEP 20
#define CONTENT_WIDTH 2200
#define CONTENT_HEIGHT 2000

// Идентификаторы меню
#define IDM_CIRCLE 1001
#define IDM_SQUARE 1002

struct Circle {
    int x, y;
    COLORREF color;
};

struct UserData {
    Circle* pCirclesCord;
};

COLORREF GetRandomColor();
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

    UserData userData = {};

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab3-4 - Scroll/Menu",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, &userData
    );

    if (hWnd == NULL) return 0;

    //создание меню
    HMENU hMenu = CreateMenu();
    HMENU hShapeMenu = CreatePopupMenu();

    //создание подменю
    AppendMenu(hShapeMenu, MF_STRING | MFS_GRAYED, IDM_CIRCLE, "Круги");
    AppendMenu(hShapeMenu, MF_STRING, IDM_SQUARE, "Квадраты");

    //добавление подменю в меню
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hShapeMenu, "Фигуры");

    //установка меню для окна
    SetMenu(hWnd, hMenu);

    ShowWindow(hWnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}

COLORREF GetRandomColor() {
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_CREATE: {

        srand(time(NULL));

        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        UserData* pUserData = (UserData*)(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUserData);

        // инициализация UserData
        pUserData->pCirclesCord = new Circle[CIRCLE_COUNT];

        for (int i = 0; i < CIRCLE_COUNT; i++) {
            Circle circle;
            circle.x = rand() % CONTENT_WIDTH;
            circle.y = rand() % CONTENT_HEIGHT;
            circle.color = GetRandomColor();
            pUserData->pCirclesCord[i] = circle;
        }

        return 0;
    }
    case WM_PAINT: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;

        GetScrollInfo(hWnd, SB_HORZ, &si);
        int currentScrollX = si.nPos;

        GetScrollInfo(hWnd, SB_VERT, &si);
        int currentScrollY = si.nPos;

        // вычисление видимой области
        RECT visibleRect;
        visibleRect.left = ps.rcPaint.left + currentScrollX;
        visibleRect.top = ps.rcPaint.top + currentScrollY;
        visibleRect.right = ps.rcPaint.right + currentScrollX;
        visibleRect.bottom = ps.rcPaint.bottom + currentScrollY;


        //получение состояния меню
        BOOL drawCircles = GetMenuState(GetSubMenu(GetMenu(hWnd), 0), IDM_CIRCLE, MF_BYCOMMAND) == MFS_GRAYED;

        for (int i = 0; i < CIRCLE_COUNT; i++) {
            const Circle& circle = pUserData->pCirclesCord[i];
            RECT intersection;

            RECT circleBounds = {};

            circleBounds.left = circle.x - CIRCLE_RADIUS;
            circleBounds.top = circle.y - CIRCLE_RADIUS;
            circleBounds.right = circle.x + CIRCLE_RADIUS;
            circleBounds.bottom = circle.y + CIRCLE_RADIUS;


            if (IntersectRect(&intersection, &circleBounds, &visibleRect)) {

                HBRUSH hBrush = CreateSolidBrush(circle.color);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);


                if (drawCircles) {
                    // круг
                    Ellipse(hdc,
                        circleBounds.left - currentScrollX,
                        circleBounds.top - currentScrollY,
                        circleBounds.right - currentScrollX,
                        circleBounds.bottom - currentScrollY);
                }
                else {
                    // квадрат
                    Rectangle(hdc,
                        circleBounds.left - currentScrollX,
                        circleBounds.top - currentScrollY,
                        circleBounds.right - currentScrollX,
                        circleBounds.bottom - currentScrollY);
                }

                SelectObject(hdc, hOldBrush);
                SelectObject(hdc, hOldPen);
                DeleteObject(hBrush);
                DeleteObject(hPen);
            }
        }

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_COMMAND: {

        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;

        switch (LOWORD(wParam)) {
        case IDM_CIRCLE: {

            HMENU hMenu = GetMenu(hWnd);

            // состояние для кругов
            mii.fState = MFS_GRAYED;
            SetMenuItemInfo(hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MFS_ENABLED;
            SetMenuItemInfo(hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        case IDM_SQUARE: {

            HMENU hMenu = GetMenu(hWnd);

            // состояние для квадратов
            mii.fState = MFS_ENABLED;
            SetMenuItemInfo(hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MFS_GRAYED;
            SetMenuItemInfo(hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        }
        return 0;
    }

    case WM_VSCROLL: {

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int clientHeight = clientRect.bottom;

        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);

        int yPos = si.nPos;

        switch (LOWORD(wParam)) {
        case SB_LINEUP: si.nPos -= SCROLL_STEP; break;
        case SB_LINEDOWN: si.nPos += SCROLL_STEP; break;
        case SB_PAGEUP: si.nPos = 0; break;
        case SB_PAGEDOWN: si.nPos = CONTENT_HEIGHT - clientHeight; break;
        case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
        case SB_TOP: si.nPos = 0; break;
        case SB_BOTTOM: si.nPos = CONTENT_HEIGHT - clientHeight; break;
        }

        si.fMask = SIF_POS;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hWnd, SB_VERT, &si);

        if (si.nPos != yPos) {
            ScrollWindowEx(hWnd, 0, yPos - si.nPos, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
        }
        return 0;
    }

    case WM_HSCROLL: {


        //RECT clientRect;
        //GetClientRect(hWnd, &clientRect);

        //int clientWidth = clientRect.right - clientRect.left;

        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_HORZ, &si);

        int xPos = si.nPos;

        switch (LOWORD(wParam)) {
        case SB_LINELEFT: si.nPos -= SCROLL_STEP; break;
        case SB_LINERIGHT: si.nPos += SCROLL_STEP; break;
        case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
        }


        si.fMask = SIF_POS;
        SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
        GetScrollInfo(hWnd, SB_HORZ, &si);

        if (si.nPos != xPos) {
            ScrollWindowEx(hWnd, xPos - si.nPos, 0, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
        }
        return 0;
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
        return 0;
    }

    case WM_SIZE: {

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        SCROLLINFO si;

        //горизонталь
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

        GetScrollInfo(hWnd, SB_HORZ, &si);
        int xMaxScroll = max(CONTENT_WIDTH - clientRect.right, 0);
        int xCurrentScroll = min(si.nPos, xMaxScroll);

        si.nMin = 0;
        si.nMax = CONTENT_WIDTH - clientRect.right;
        si.nPage = 0;
        si.nPos = xCurrentScroll;
        SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

        //вертикаль
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

        GetScrollInfo(hWnd, SB_VERT, &si);
        int yMaxScroll = max(CONTENT_HEIGHT - clientRect.bottom, 0);
        int yCurrentScroll = min(si.nPos, yMaxScroll);

        si.nMin = 0;
        si.nMax = CONTENT_HEIGHT - clientRect.bottom;
        si.nPage = 0;
        si.nPos = yCurrentScroll;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

        /*SCROLLINFO si;

        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.cbSize = sizeof(SCROLLINFO);
        si.nMin = 0;

        si.nMax = CONTENT_HEIGHT;
        si.nPage = clientRect.bottom;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

        si.nMax = CONTENT_WIDTH;
        si.nPage = clientRect.right;
        SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);*/

        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    case WM_DESTROY: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (pUserData->pCirclesCord) {
            delete[] pUserData->pCirclesCord;
            pUserData->pCirclesCord = nullptr;
        }

        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
