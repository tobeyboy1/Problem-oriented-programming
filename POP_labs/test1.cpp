#define  STRICT
#define  WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>

#define N 100
#define M 1
#define BUTTON_HEIGHT 25
#define BUTTON_WIDTH 100
#define BUTTON_OFFSET 10

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	//Registering the window class
	LPCTSTR szClass = TEXT("DiskTester32");

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = szClass;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	if (!RegisterClass(&wc)) return -1;

	// Get the screen dimensions
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Calculate the position to center the window
	int windowWidth = 0.5 * screenWidth;
	int windowHeight = 0.5 * screenHeight;
	int windowX = (screenWidth - windowWidth) / 2;
	int windowY = (screenHeight - windowHeight) / 2;

	HWND hWnd = ::CreateWindow(szClass, "OverLapped", WS_OVERLAPPEDWINDOW,
		windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		return -1;
	}

	SendMessage(GetDlgItem(hWnd, M), BM_SETSTATE, BST_PUSHED, 0);
	SendMessage(GetDlgItem(hWnd, M + N), BM_SETCHECK, BST_CHECKED, 0);

	int position = M;
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)&position);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
	}

	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		char buff[20] = { 0 };
		for (int i = 1; i <= N; i++)
		{
			sprintf_s(buff, "Кнопка %d", i);

			CreateWindow
			(
				"BUTTON",
				buff,
				WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON,
				BUTTON_OFFSET,
				BUTTON_OFFSET + (i - 1) * (BUTTON_OFFSET + BUTTON_HEIGHT),
				BUTTON_WIDTH,
				BUTTON_HEIGHT,
				hWnd,
				(HMENU)i,
				(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
				NULL
			);

			sprintf_s(buff, "Радио %d", i);

			CreateWindow
			(
				"BUTTON",
				buff,
				WS_CHILDWINDOW | WS_VISIBLE | BS_RADIOBUTTON,
				BUTTON_WIDTH + 20,
				BUTTON_OFFSET + (i - 1) * (BUTTON_OFFSET + BUTTON_HEIGHT),
				BUTTON_WIDTH,
				BUTTON_HEIGHT,
				hWnd,
				(HMENU)(i + N),
				(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
				NULL
			);
		}
		break;
	}

	case WM_COMMAND:
	{
		SCROLLINFO vscroll;
		vscroll.cbSize = sizeof(SCROLLINFO);
		vscroll.fMask = SIF_RANGE | SIF_POS;

		GetScrollInfo(hWnd, SB_VERT, &vscroll);
		int button_number_onPage = (GetSystemMetrics(SM_CYSCREEN) - BUTTON_OFFSET) / (BUTTON_OFFSET + BUTTON_HEIGHT);
		int first_visiable_button = vscroll.nPos * (N - button_number_onPage) / vscroll.nMax + 1;

		//int prevposition = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		int ButtonID = LOWORD(wParam);

		if (ButtonID > N)
		{
			ButtonID -= N;
		}

		if (ButtonID != GetWindowLongPtr(hWnd, GWLP_USERDATA))
		{
			SendMessage(GetDlgItem(hWnd, GetWindowLongPtr(hWnd, GWLP_USERDATA)), BM_SETSTATE, BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hWnd, GetWindowLongPtr(hWnd, GWLP_USERDATA) + N), BM_SETCHECK, BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hWnd, ButtonID), BM_SETSTATE, BST_PUSHED, 0);
			SendMessage(GetDlgItem(hWnd, ButtonID + N), BM_SETCHECK, BST_CHECKED, 0);

			prevposition = ButtonID + first_visiable_button - 1;
		}
		else
		{
			SendMessage(GetDlgItem(hWnd, ButtonID), BM_SETSTATE, BST_PUSHED, 0);
		}

		prevposition = ButtonID; // Добавленный код для сброса предыдущей кнопки

		return 0;
	}
	case WM_SIZE: {
		int height = N * (BUTTON_OFFSET + BUTTON_HEIGHT) + BUTTON_OFFSET;
		SCROLLINFO scrInfo;
		scrInfo.cbSize = sizeof(SCROLLINFO);

		scrInfo.nPage = HIWORD(lParam); //размер страницы устанавливаем равным высоте окна

		scrInfo.nMin = 0; //диапазон прокрутки устанавливаем по размеру содержимого
		scrInfo.nMax = height; //(вместо CONTENT_HEIGHT подставь нужное значение)

		scrInfo.fMask = SIF_RANGE | SIF_PAGE; //применяем новые параметры
		SetScrollInfo(hWnd, SB_VERT, &scrInfo, TRUE);
		return 0;
	}

	case WM_VSCROLL: {
		SCROLLINFO scrInfo;
		scrInfo.cbSize = sizeof(SCROLLINFO);

		scrInfo.fMask = SIF_ALL; //получаем текущие параметры scrollbar-а
		GetScrollInfo(hWnd, SB_VERT, &scrInfo);

		int currentPos = scrInfo.nPos; //запоминаем текущее положение содержимого

		switch (LOWORD(wParam)) { //определяем действие пользователя и изменяем положение
		case SB_LINEUP: //клик на стрелку вверх
			scrInfo.nPos -= 10;
			break;
		case SB_LINEDOWN: //клик на стрелку вниз 
			scrInfo.nPos += 10;
			break;
		case SB_PAGEUP:
			scrInfo.nPos -= scrInfo.nPage;
			break;
		case SB_PAGEDOWN:
			scrInfo.nPos += scrInfo.nPage;
			break;
		case SB_THUMBTRACK: //перетаскивание ползунка
			scrInfo.nPos = scrInfo.nTrackPos;
			break;
		case SB_TOP:
			scrInfo.nPos = scrInfo.nMin;
			break;
		case SB_BOTTOM:
			scrInfo.nPos = scrInfo.nMax;
			break;
		default: return 0; //все прочие действия (например нажатие PageUp/PageDown) игнорируем
		}

		scrInfo.fMask = SIF_POS; //пробуем применить новое положение
		SetScrollInfo(hWnd, SB_VERT, &scrInfo, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &scrInfo); //(см. примечание ниже)

		int yScroll = currentPos - scrInfo.nPos; // вычисляем величину прокрутки
		ScrollWindow(hWnd, 0, yScroll, NULL, NULL); //выполняем прокрутку
		return 0;
	}
	case WM_KEYDOWN:
	{
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
		case VK_END:
		{
			PostMessage(hWnd, WM_VSCROLL, SB_BOTTOM, NULL);
			break;
		}
		case VK_HOME:
		{
			PostMessage(hWnd, WM_VSCROLL, SB_TOP, NULL);
			break;
		}
		case VK_PRIOR:
		{
			PostMessage(hWnd, WM_VSCROLL, SB_PAGEUP, NULL);
			break;
		}
		case VK_NEXT:
		{
			PostMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, NULL);
			break;
		}
		}
		break;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
