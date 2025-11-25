#include <windows.h>
//#include <windowsx.h>
#include <iostream>

#define CLASS_NAME "F(DSHNLK#NROUFYE(*nlk234nouo3yf98h23n"
// Константы
#define BUTTON_ID_START 1000

#define BUTTON_HEIGHT 25
#define BUTTON_WIDTH 100
#define BUTTON_OFFSET 10

#define N 5000  // кол-во кнопок
#define M 18     // стартовая активная кнопка

#define startButton (M*2)

#define BSIZE (N>=20 ? 20 : N) //кол-во отображаемых кнопок

struct UserData {
    int currentActive;
    HWND buttons[BSIZE][2];
};

void ScrollButton(HWND hWnd, UserData* pUserData, int scrollStep);
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) return -1;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int indentX = (int)(screenWidth * 0.07);  // 7% от ширины экрана
    int indentY = (int)(screenHeight * 0.07); // 7% от высоты экрана

    int windowHeight = N > screenHeight * 0.7 / (BUTTON_HEIGHT + BUTTON_OFFSET) ?
        (screenHeight / (BUTTON_HEIGHT + BUTTON_OFFSET) * BUTTON_HEIGHT) :
        (N * (BUTTON_HEIGHT + BUTTON_OFFSET) + 5 * BUTTON_OFFSET);

    UserData userData = { 0 };

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Lab7",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VSCROLL,
        indentX,
        indentY,
        BUTTON_WIDTH * 2 + BUTTON_OFFSET * 6,
        windowHeight,// - 2 * indentY,
        NULL, NULL, hInstance, &userData);

    if (hWnd == NULL) return -1;

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

        char buff[25] = { 0 };

        int buttonID = BUTTON_ID_START;

        for (int i = 0; i < BSIZE; i++) {
            sprintf_s(buff, "Кнопка %d", i);
            // нажимная
            pUserData->buttons[i][0] = CreateWindow(
                "BUTTON",
                buff,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                BUTTON_OFFSET,
                BUTTON_OFFSET + i * (BUTTON_OFFSET + BUTTON_HEIGHT),
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                hWnd, (HMENU)(buttonID),
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

            buttonID++;
            // радио
            sprintf_s(buff, "Радио %d", i);
            pUserData->buttons[i][1] = CreateWindow(
                "BUTTON",
                buff,
                WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                BUTTON_WIDTH + 20,
                BUTTON_OFFSET + i * (BUTTON_OFFSET + BUTTON_HEIGHT),
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                hWnd, (HMENU)(buttonID),
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

            buttonID++;
        }
        pUserData->currentActive = startButton + BUTTON_ID_START;

        SendMessage(GetDlgItem(hWnd, startButton + BUTTON_ID_START), BM_SETSTATE, BST_PUSHED, 0);
        SendMessage(GetDlgItem(hWnd, startButton + BUTTON_ID_START + 1), BM_SETCHECK, BST_CHECKED, 0);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUserData);
        return 0;
    }

    case WM_COMMAND: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (HIWORD(wParam) == BN_CLICKED) {

            int baseID = LOWORD(wParam);

            if (LOWORD(wParam) % 2 != 0) {
                baseID = LOWORD(wParam) - 1;
            }

            if (LOWORD(wParam) != pUserData->currentActive) {
                // снятие старого состояния
                SendMessage(GetDlgItem(hWnd, pUserData->currentActive), BM_SETSTATE, BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hWnd, pUserData->currentActive + 1), BM_SETCHECK, BST_UNCHECKED, 0);

                // установка нового
                pUserData->currentActive = baseID;
                SendMessage(GetDlgItem(hWnd, baseID), BM_SETSTATE, BST_PUSHED, 0);
                SendMessage(GetDlgItem(hWnd, baseID + 1), BM_SETCHECK, BST_CHECKED, 0);
                SetFocus(hWnd);
            }
            else {
                SendMessage(GetDlgItem(hWnd, baseID), BM_SETSTATE, BST_PUSHED, 0);
                SetFocus(hWnd);
            }
        }
        return 0;
    }
    case WM_SIZE: {

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE;

        GetScrollInfo(hWnd, SB_VERT, &si);

        si.nMin = 0;
        si.nMax = N * (BUTTON_OFFSET + BUTTON_HEIGHT) - clientRect.bottom;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

        return 0;
    }

    case WM_VSCROLL: {
        UserData* pUserData = (UserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);

        switch (LOWORD(wParam)) {
        case SB_LINEUP: {
            if (si.nPos != si.nMin) {

                ScrollButton(hWnd, pUserData, -2);
                si.nPos -= BUTTON_HEIGHT + BUTTON_OFFSET;
            }
            break;
        }

        case SB_LINEDOWN: {
            if (si.nPos != si.nMax) {

                ScrollButton(hWnd, pUserData, 2);
                si.nPos += BUTTON_HEIGHT + BUTTON_OFFSET;
            }
            break;
        }
        }
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

        return 0;
    }
    case WM_KEYDOWN: {
        switch (LOWORD(wParam))
        {
        case VK_UP:
        {
            PostMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
            break;
        }
        case VK_DOWN:
        {
            PostMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
            break;
        }
        }
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

void ScrollButton(HWND hWnd, UserData* pUserData, int scrollStep) {
    SendMessage(GetDlgItem(hWnd, pUserData->currentActive), BM_SETSTATE, BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hWnd, pUserData->currentActive + 1), BM_SETCHECK, BST_UNCHECKED, 0);

    for (int i = 0; i < BSIZE; i++) {
        char buff[25] = { 0 };

        int newID = (LONG_PTR)(GetWindowLongPtr(pUserData->buttons[i][0], GWLP_ID) + scrollStep);

        //установка нового ID и текста для нажимной
        SetWindowLongPtr(pUserData->buttons[i][0], GWLP_ID, newID);
        sprintf_s(buff, "Кнопка %d", (newID - BUTTON_ID_START) / 2);
        SetWindowText(pUserData->buttons[i][0], buff);

        //установка нового ID и текста для нажимной
        SetWindowLongPtr(pUserData->buttons[i][1], GWLP_ID, newID + 1);
        sprintf_s(buff, "Кнопка %d", (newID - BUTTON_ID_START) / 2);
        SetWindowText(pUserData->buttons[i][1], buff);
    }
    SendMessage(GetDlgItem(hWnd, pUserData->currentActive), BM_SETSTATE, BST_PUSHED, 0);
    SendMessage(GetDlgItem(hWnd, pUserData->currentActive + 1), BM_SETCHECK, BST_CHECKED, 0);
}