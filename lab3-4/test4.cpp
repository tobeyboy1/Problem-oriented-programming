#include <windows.h>
#include <time.h>

#define NAME_CAPACITY 100

#define CIRCLE_RADIUS 20
#define CIRCLE_COUNT 500
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
    Circle* pCirclesCord; //обычный массив
    int currentScrollX; //
    int currentScrollY;
    int oldClientWidth;    
    int oldClientHeight;  // убрать  
    HMENU hMenu;
};

COLORREF GetRandomColor();
void UpdateScrollBars(HWND hWnd, UserData* pUserData);
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

void UpdateScrollBars(HWND hWnd, UserData* pUserData) {

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;

    // вертикальная прокрутка
    si.nMin = 0;
    si.nMax = CONTENT_HEIGHT;
    si.nPage = 0;
    si.nPos = pUserData->currentScrollY; //сlientHeight
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

    // горизонтальная прокрутка
    si.nMin = 0;
    si.nMax = CONTENT_WIDTH;
    si.nPage = clientWidth;
    si.nPos = pUserData->currentScrollX;
    SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {

        srand(time(NULL));

        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        UserData* pUserData = (UserData*)(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUserData);

        // инициализация UserData
        pUserData->currentScrollX = 0;
        pUserData->currentScrollY = 0;
        pUserData->oldClientWidth = WINDOW_WIDTH;     
        pUserData->oldClientHeight = WINDOW_HEIGHT;
        pUserData->pCirclesCord = new Circle[CIRCLE_COUNT];

        for (int i = 0; i < CIRCLE_COUNT; i++) {
            Circle circle;
            circle.x = rand() % CONTENT_WIDTH;
            circle.y = rand() % CONTENT_HEIGHT;
            circle.color = GetRandomColor();

            /*circle.bounds.left = circle.x - CIRCLE_RADIUS;
            circle.bounds.top = circle.y - CIRCLE_RADIUS;
            circle.bounds.right = circle.x + CIRCLE_RADIUS;
            circle.bounds.bottom = circle.y + CIRCLE_RADIUS;*/

            pUserData->pCirclesCord[i] = circle;
        }

        //создание меню
        pUserData->hMenu = CreateMenu();
        HMENU hShapeMenu = CreatePopupMenu();

        //создание подменю
        AppendMenu(hShapeMenu, MF_STRING | MFS_GRAYED, IDM_CIRCLE, "Круги");
        AppendMenu(hShapeMenu, MF_STRING, IDM_SQUARE, "Квадраты");

        //добавление подменю в меню
        AppendMenu(pUserData->hMenu, MF_POPUP, (UINT_PTR)hShapeMenu, "Фигуры");

        //установка меню для окна
        SetMenu(hWnd, pUserData->hMenu);

        return 0;
    }
    case WM_PAINT: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // вычисление видимой области
        RECT visibleRect;
        visibleRect.left = ps.rcPaint.left + pUserData->currentScrollX;
        visibleRect.top = ps.rcPaint.top + pUserData->currentScrollY;
        visibleRect.right = ps.rcPaint.right + pUserData->currentScrollX;
        visibleRect.bottom = ps.rcPaint.bottom + pUserData->currentScrollY;


        //получение состояния меню
        BOOL drawCircles = GetMenuState(GetSubMenu(pUserData->hMenu, 0), IDM_CIRCLE, MF_BYCOMMAND) == MFS_GRAYED;

        for (int i = 0; i < CIRCLE_COUNT; i++) {
            const Circle& circle = pUserData->pCirclesCord[i];
            RECT intersection;

            RECT circleBounds = {};
            
            circleBounds.left = circle.x - CIRCLE_RADIUS;
            circleBounds.top = circle.y - CIRCLE_RADIUS;
            circleBounds.right = circle.x + CIRCLE_RADIUS;
            circleBounds.bottom = circle.y + CIRCLE_RADIUS;


            if (IntersectRect(&intersection, &circleBounds, &visibleRect)) {

                // вычисление координат фигур с учётом прокрутки
                int screenX = circle.x - pUserData->currentScrollX; 
                int screenY = circle.y - pUserData->currentScrollY;

                HBRUSH hBrush = CreateSolidBrush(circle.color);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);


                if (drawCircles) {
                    // круг
                    Ellipse(hdc,
                        screenX - CIRCLE_RADIUS,
                        screenY - CIRCLE_RADIUS,
                        screenX + CIRCLE_RADIUS,
                        screenY + CIRCLE_RADIUS);
                }
                else {
                    // квадрат
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
        return 0;
    }

    case WM_COMMAND: {

        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;

        switch (LOWORD(wParam)) {
        case IDM_CIRCLE:

            // состояние для кругов
            mii.fState = MFS_GRAYED;
            SetMenuItemInfo(pUserData->hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MFS_ENABLED;
            SetMenuItemInfo(pUserData->hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hWnd, NULL, TRUE); 
            return 0;

        case IDM_SQUARE:

            // состояние для квадратов
            mii.fState = MFS_ENABLED;
            SetMenuItemInfo(pUserData->hMenu, IDM_CIRCLE, FALSE, &mii);

            mii.fState = MFS_GRAYED;
            SetMenuItemInfo(pUserData->hMenu, IDM_SQUARE, FALSE, &mii);

            InvalidateRect(hWnd, NULL, TRUE); 
            return 0;
        }
        return 0;
    }
    case WM_VSCROLL: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int clientHeight = clientRect.bottom - clientRect.top;

        int oldScrollY = pUserData->currentScrollY;

        switch (LOWORD(wParam)) {
        case SB_LINEUP: pUserData->currentScrollY -= SCROLL_STEP; break;
        case SB_LINEDOWN: pUserData->currentScrollY += SCROLL_STEP; break;
        case SB_PAGEUP: pUserData->currentScrollY -= clientHeight; break;
        case SB_PAGEDOWN: pUserData->currentScrollY += clientHeight; break;
        case SB_THUMBTRACK: pUserData->currentScrollY = HIWORD(wParam); break;
        case SB_TOP: pUserData->currentScrollY = 0; break;
        case SB_BOTTOM: pUserData->currentScrollY = CONTENT_HEIGHT - clientHeight; break;
        }

        pUserData->currentScrollY = max(0, min(pUserData->currentScrollY, CONTENT_HEIGHT - clientHeight));

        if (pUserData->currentScrollY != oldScrollY) {
            int deltaY = oldScrollY - pUserData->currentScrollY;
            ScrollWindowEx(hWnd, 0, deltaY, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
            UpdateScrollBars(hWnd, pUserData);
        }
        return 0;
    }

    case WM_HSCROLL: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        //Добавить GetScrollInfo
        int clientWidth = clientRect.right - clientRect.left;

        int oldScrollX = pUserData->currentScrollX;

        switch (LOWORD(wParam)) {
        case SB_LINELEFT: pUserData->currentScrollX -= SCROLL_STEP; break;
        case SB_LINERIGHT: pUserData->currentScrollX += SCROLL_STEP; break;
        case SB_THUMBTRACK: pUserData->currentScrollX = HIWORD(wParam); break;
        }

        pUserData->currentScrollX = max(0, min(pUserData->currentScrollX, CONTENT_WIDTH - clientWidth));

        if (pUserData->currentScrollX != oldScrollX) {
            int deltaX = oldScrollX - pUserData->currentScrollX;
            ScrollWindowEx(hWnd, deltaX, 0, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
            UpdateScrollBars(hWnd, pUserData); //SetScrollInfo для позиции ползунка
            
        }
        //отсюда вызвать invalidateRect
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
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int newClientWidth = clientRect.right - clientRect.left;
        int newClientHeight = clientRect.bottom - clientRect.top;

        // ограничение текущей позиции прокрутки новыми пределами
        int maxScrollX = max(0, CONTENT_WIDTH - newClientWidth);
        int maxScrollY = max(0, CONTENT_HEIGHT - newClientHeight);

        pUserData->currentScrollX = min(pUserData->currentScrollX, maxScrollX);
        pUserData->currentScrollY = min(pUserData->currentScrollY, maxScrollY);

        // сохранение текущих размеров
        pUserData->oldClientWidth = newClientWidth;
        pUserData->oldClientHeight = newClientHeight;

        //утсановить nMax для вертикали и горизонтали
        UpdateScrollBars(hWnd, pUserData);

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
        break;
    }
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
