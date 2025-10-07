#pragma once

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TempWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void SetNewParent(HWND hNewParent);
void UpdatePosition(HWND hParent);
void ToggleTopmost();

BOOL RegisterWndsProc(HINSTANCE hInstance);
BOOL CreateAppWnds(HINSTANCE hInstance, int nCmdShow);

