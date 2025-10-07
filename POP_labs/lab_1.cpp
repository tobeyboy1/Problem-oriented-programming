///////////////////////////////////////////////////////////
//	Test.cpp
//
#define STRICT //������� �������� �����
#define WIN32_LEAN_AND_MEAN // ��������� ����� ������������ ��������� Windows

#define g_MainWndKey "fdsuichewiu9iod2n3lhd90sc"
#define g_TempWndKey "ek23beljf903-ejl;3d"
#define g_ChildWndKey "fdsofn4orh3490fu90ven43"


#include <Windows.h>
#include "lab_1.h"

HWND g_hMainWnd;
HWND g_hTempWnd;
HWND g_hChildWnd;
bool g_IsTopmost = false;

int WINAPI  WinMain(

    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow //�������� �� ����� ���������/����������� ����
)
{

    if (!RegisterWndsProc(hInstance))
        return -1;

    if (!CreateAppWnds(hInstance, nCmdShow))
        return -1;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { 
        DispatchMessage(&msg);
    }

    return 0;
}

BOOL RegisterWndsProc(HINSTANCE hInstance) {

    WNDCLASS wc = { 0 }; //������������� ���� ��������� �������� ����������
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    //�������
    wc.lpfnWndProc = MainWndProc; //��������� �� �-� ��������� ���������
    wc.hInstance = hInstance; //����������
    wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
    wc.lpszClassName = g_MainWndKey; //��� ������ ����

    if (!RegisterClass(&wc)) // ����������� ������ � �������
        return FALSE;

    //���������
    wc.style = CS_DBLCLKS; //��� ��������� ������� �������
    wc.lpfnWndProc = TempWndProc;
    wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
    wc.lpszClassName = g_TempWndKey;

    if (!RegisterClass(&wc))
        return FALSE;

    //��������
    wc.lpfnWndProc = ChildWndProc;
    wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
    wc.lpszClassName = g_ChildWndKey;

    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;
}

BOOL CreateAppWnds(HINSTANCE hInstance, int nCmdShow) {

    //�������
    g_hMainWnd = CreateWindow(g_MainWndKey, "MainWindow", WS_OVERLAPPEDWINDOW,
        600, 100, 800, 600, NULL, NULL, hInstance, NULL);

    if (!g_hMainWnd)
        return FALSE;

    //���������
    g_hTempWnd = CreateWindow(g_TempWndKey, "TempWindow", WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_VISIBLE, // WS_EX_NOACTIVATE ����� ������ � ������ �����; POPUPWINDOW �������� BORDER � SYSMENU
        100, 100, 400, 300, g_hMainWnd, NULL, hInstance, NULL); //4 � ����� - ���������� ������������� ��� ���� ���������

    if (!g_hTempWnd)
        return FALSE;

    //��������
    g_hChildWnd = CreateWindow(g_ChildWndKey, "ChildWindow", WS_CHILD | WS_BORDER | WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
        50, 50, 200, 150, g_hMainWnd, NULL, hInstance, NULL);

    if (!g_hChildWnd)
        return FALSE;

    ShowWindow(g_hMainWnd, nCmdShow); //SW_SHOW ������������ ��� ������ ������ SHOW_WINDOW
    //ShowWindow(g_hTempWnd, SW_SHOW); //���������� ���� � ������� ������� � ���������
    //ShowWindow(g_hChildWnd, SW_SHOW);

    return TRUE;
}

void SetNewParent(HWND hNewParent) {

    if (g_hChildWnd && hNewParent) {

        HWND hCurrentParent = GetParent(g_hChildWnd); //��������� �������� ��������

        if (hCurrentParent != hNewParent) {
            SetParent(g_hChildWnd, hNewParent);

            //������������� ������������ ������ ��������
            UpdatePosition(hNewParent);
        }
    }
}

void UpdatePosition(HWND hParent) {

    HWND hCurrentParent = GetParent(g_hChildWnd); //��������� �������� ��������

    if (hCurrentParent == hParent) {

        RECT rcParent, rcChild;
        GetClientRect(hParent, &rcParent);
        GetWindowRect(g_hChildWnd, &rcChild);

        int width = rcChild.right - rcChild.left;
        int height = rcChild.bottom - rcChild.top;
        int x = (rcParent.right - width) / 2;
        int y = (rcParent.bottom - height) / 2;

        SetWindowPos(g_hChildWnd, NULL, x, y, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE); //NOZORDER ���������� ������� ������� Z (���� ����)
    }
}

void ToggleTopmost() {

    SetWindowPos(g_hTempWnd, g_IsTopmost ? HWND_NOTOPMOST : HWND_TOPMOST,
        NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE);

    g_IsTopmost = !g_IsTopmost;

}


//=========================================================

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_LBUTTONDOWN:
    {
        if (!IsWindow(g_hChildWnd)) //������ �� ������� �������� �� ���������� (����� � WM_SIZING) ��� ������ �������� ������ � UpdatePosition
            return 0;

        SetNewParent(hWnd);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
       // FillRect(hdc, &ps.rcPaint, HBRUSH(COLOR_WINDOW + 1));
        EndPaint(hWnd, &ps);
        //UpdatePosition(hWnd);

    }
    return 0;

    case WM_SIZING: //�� ������ ��� ��� �����, ������ ��� ��������� ���� (���� ��� ������ � �������� ������ � WM_PAINT)
    {
        if (!IsWindow(g_hChildWnd))
            return 0;

        UpdatePosition(hWnd);

        return 0;
    }
    case WM_DESTROY:

        PostQuitMessage(0); //����� �� ���������� ��� �������� ����

        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam); //��������� ���������������� ���������� ���� �� ���������
}

LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_LBUTTONDOWN:
    {
        if (!IsWindow(g_hChildWnd))
            return 0;

        SetNewParent(hWnd);
        return 0;
    }
    case WM_LBUTTONDBLCLK:

        ToggleTopmost();
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        //FillRect(hdc, &ps.rcPaint, HBRUSH(COLOR_WINDOW + 2));
        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_SIZING:
    {
        if (!IsWindow(g_hChildWnd))
            return 0;

        UpdatePosition(hWnd);
        return 0;
    }
    case WM_DESTROY:
        //�� ������� �� ���������� ��� �������� ����
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        //FillRect(hdc, &ps.rcPaint, HBRUSH(COLOR_WINDOW - 2));
        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_DESTROY:

        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
//=========================================================