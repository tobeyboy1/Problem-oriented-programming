#define _USE_MATH_DEFINES

#include <windows.h>
#include <math.h>
#include <time.h>
#include "resource.h"

#define NAME_CAPACITY 100
#define TIMER_ID 1001

#define ROTATION_STEP 30
#define FONT_HEIGHT_CHANGE 6
#define START_FONT_HEIGHT 60
#define TIMER_INTERVAL 1000

#define MESSAGE "Test string"

struct UserData {
    int rotationAngle;
    int fontSize;
};

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    srand(time(NULL));

    char CLASS_NAME[NAME_CAPACITY]{};

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);

    int i = 0;

    do {
        CLASS_NAME[i] = 33 + rand() % 94;
        i++;
        wc.lpszClassName = CLASS_NAME;
    } while (!RegisterClass(&wc) && i < NAME_CAPACITY);

    UserData userData = { 0 };

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab5",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        UserData* pUserData = (UserData*)(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUserData);

        pUserData->fontSize = START_FONT_HEIGHT;

        PostMessage(hWnd, WM_COMMAND, MAKELONG(ID_START, 0), NULL);
        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_EXIT:
            PostMessage(hWnd, WM_DESTROY, NULL, NULL);
            break;

        case ID_STOP:
            KillTimer(hWnd, TIMER_ID);
            EnableMenuItem(GetMenu(hWnd), ID_STOP, MF_GRAYED);
            EnableMenuItem(GetMenu(hWnd), ID_START, MF_ENABLED);
            break;

        case ID_START:
            SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
            EnableMenuItem(GetMenu(hWnd), ID_STOP, MF_ENABLED);
            EnableMenuItem(GetMenu(hWnd), ID_START, MF_GRAYED);
            break;
        }
        return 0;
    }

    case WM_TIMER: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        pUserData->rotationAngle = (pUserData->rotationAngle + ROTATION_STEP) % 360;
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    case WM_PAINT: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        FLOAT centerX = clientRect.right / 2;
        FLOAT centerY = clientRect.bottom / 2;

        HFONT hFont = CreateFont(
            pUserData->fontSize, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"
        );

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        SetTextAlign(hdc, TA_CENTER | TA_BASELINE); //выравнивание текста

        SetGraphicsMode(hdc, GM_ADVANCED); //позволяет видоизменять объекты для отрисовки

        double radians = pUserData->rotationAngle * M_PI / 180.0;

        XFORM xform;
        xform.eM11 = (FLOAT)cos(radians);
        xform.eM12 = (FLOAT)sin(radians);
        xform.eM21 = (FLOAT)-sin(radians);
        xform.eM22 = (FLOAT)cos(radians);
        xform.eDx = centerX; //смещение
        xform.eDy = centerY;

        SetWorldTransform(hdc, &xform);

        TextOut(hdc, 0, pUserData->fontSize / 4, MESSAGE, strlen(MESSAGE));

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_LBUTTONUP: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        pUserData->fontSize += FONT_HEIGHT_CHANGE;
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    case WM_RBUTTONUP: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (pUserData->fontSize > FONT_HEIGHT_CHANGE) {
            pUserData->fontSize -= FONT_HEIGHT_CHANGE;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_SIZE: {
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    case WM_DESTROY: {
        KillTimer(hWnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}