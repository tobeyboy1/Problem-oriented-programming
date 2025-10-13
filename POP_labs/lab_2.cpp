#define STRICT //строгая проверка типов
#define WIN32_LEAN_AND_MEAN // исключает редко используемые заголовки Windows

#include <Windows.h>
#include <cmath>
#include <iostream>


#define DIVISION_COUNT 9  // можно изменить на 4, 6, 9 и более

struct UserData
{
    int hoverRectIndex;
    int rowsCount;
    int colsCount;
};


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void CalculateGridRects(HWND hWnd, RECT* rects);

UserData* GetUserData(HWND hwnd);


int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow //отвечает за показ свёрнутого/развёрнутого окна)
)
{

    std::string name = "StartSet"; //начальное имя опционально
    const char* CLASS_NAME;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    srand(time(NULL));

    do {
        name += 33 + rand() % 94;
        CLASS_NAME = name.c_str();

        wc.lpszClassName = CLASS_NAME;

    } while (!RegisterClass(&wc));

    UserData UserData[1] = { -1, 0, 0 };

    HWND hWnd = CreateWindow(
        CLASS_NAME,
        "MainWnd",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, &UserData
    );

    if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);
    //UpdateWindow(hWnd);  //можно вернуть чтобы при появлении окна оно сразу отобразилось корректно

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return 0;
}

UserData* GetUserData(HWND hwnd)
{
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    return (UserData*)(ptr);
}

// функция для вычисления прямоугольников сетки
void CalculateGridRects(HWND hWnd, RECT* rects) {

    UserData* pData = GetUserData(hWnd);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // вычисление оптимальных размеров сетки
    int rows, cols;

    rows = (int)sqrt(DIVISION_COUNT);
    cols = DIVISION_COUNT / rows;

    // корректировка, если есть остаток
    while (rows * cols < DIVISION_COUNT) {
        if (cols <= rows) {
            cols++;
        }
        else {
            rows++;
        }
    }

    pData->colsCount = cols;
    pData->rowsCount = rows;

    int cellWidth = width / cols;
    int cellHeight = height / rows;

    // заполнение массива прямоугольников
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;
            if (index < DIVISION_COUNT) {
                rects[index].left = j * cellWidth;
                rects[index].top = i * cellHeight;
                rects[index].right = (j + 1) * cellWidth;
                rects[index].bottom = (i + 1) * cellHeight;

                //растяжение до границ окна
                if (j == cols - 1) {
                    rects[index].right = clientRect.right;
                }

                if (i == rows - 1) {
                    rects[index].bottom = clientRect.bottom;
                }
            }
        }
    }
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {

    case WM_CREATE: {

        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        UserData* pUserData = (UserData*)(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUserData);

        return 0;
    }

    case WM_MOUSEMOVE: {


        int newHoverIndex = -1;
        UserData* pData = GetUserData(hWnd);

        POINT pt = { LOWORD(lParam), HIWORD(lParam) }; //извлечение координат мыши
        RECT rects[DIVISION_COUNT];
        CalculateGridRects(hWnd, rects);

        for (int i = 0; i < DIVISION_COUNT; i++) {
            if (PtInRect(&rects[i], pt)) { //проверка находится ли точка в координатах выбранного прямоугольника
                newHoverIndex = i;
                break;
            }
        }

        if (newHoverIndex != pData->hoverRectIndex) {

            pData->hoverRectIndex = newHoverIndex;
            InvalidateRect(hWnd, NULL, TRUE); // перерисовка всего окна
        }
        return 0;
    }

    case WM_NCMOUSEMOVE: {

        UserData* pData = GetUserData(hWnd);

        // курсор покинул клиентскую область
        if (pData->hoverRectIndex != -1) {

            pData->hoverRectIndex = -1;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_PAINT: {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        //вычисление прямоугольников сетки
        RECT rects[DIVISION_COUNT];
        CalculateGridRects(hWnd, rects);

        //создание кисти
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);

        UserData* pData = GetUserData(hWnd);

        //заливка прямоугольника
        HBRUSH hBrush = CreateSolidBrush(RGB(200, 220, 255));
        FillRect(hdc, &rects[pData->hoverRectIndex], hBrush);
        DeleteObject(hBrush);


        //отрисовка горизонт.
        for (int i = 1; i < pData->rowsCount + 1; i++) {
            MoveToEx(hdc, clientRect.left, clientRect.bottom * i / pData->rowsCount, NULL);  //установка позиции в указанную точку
            LineTo(hdc, clientRect.right, clientRect.bottom * i / pData->rowsCount);
        }

        //отрисовка вертикал.
        for (int i = 1; i < pData->colsCount + 1; i++) {
            MoveToEx(hdc, clientRect.right * i / pData->colsCount, clientRect.top, NULL);
            LineTo(hdc, clientRect.right * i / pData->colsCount, clientRect.bottom);
        }

        //удаление кисти
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_SIZE:
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;

    case WM_DESTROY: {

        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}