#define _USE_MATH_DEFINES
#include <windows.h>
#include <atlimage.h>
#include <math.h>
#include <gdiplus.h>

#define BACKGROUND_COLOR GetSysColor(COLOR_WINDOW)
#define SCROLL_SPEED 10

const int rows = 6;
const int columns = 5;
int top = 0, bottom = 0;

const TCHAR* matrix[rows][columns] = { {"sometext1 sometext1 sometext1 sometext1 sometext1","sometext2","sometext3","sometext4","sometext5"},
									   {"sometext6","sometext7","sometext8","sometext9","sometext10"},
									   { "sometext11","sometext12","sometext13","sometext14","sometext15"}, 
									   { "sometext16","sometext17","sometext18","sometext19","sometext20"}, 
									   { "sometext21","sometext22","sometext23","sometext24","sometext25"}, 
									   { "sometext26","sometext27","sometext28 sometext28 sometext28 sometext28 sometext28 sometext28 sometext28","sometext29","sometext30"}, 
									};

SIZE GetWindowsSize(HBITMAP hBitmap)
{
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	SIZE result;
	result.cx = bitmap.bmWidth;
	result.cy = bitmap.bmHeight;
	return result;
}

float GetRowSize(HDC hdc, int rowNumber,float hx)
{
	int maxLength = 0;
	int maxIndex = 0;
	for (int i = 0; i < columns; i++)
	{
		int length = lstrlen(matrix[rowNumber][i]);
		if (length > maxLength)
		{
			maxLength = length;
			maxIndex = i;
		}
	}
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = hx-2;
	rect.bottom = 1;
	DrawText(hdc, matrix[rowNumber][maxIndex], maxLength, &rect,DT_CALCRECT| DT_WORDBREAK| DT_EDITCONTROL | DT_CENTER);
	int textRows = (int)((rect.right / (hx - 2)) + 1);
	return (float)(textRows*rect.bottom+2);
}

void DrawTable(HDC hdc,int sx,int sy)
{
	float posX = 0, posY = (float)top;
	float hx = (float)sx / columns;
	float hy = 0;

	for (int i = 0; i < rows; i++)
	{
		hy = GetRowSize(hdc,i, hx);
		posX = 0;
		for (int j = 0; j < columns; j++)
		{
			MoveToEx(hdc, posX, posY, NULL);
			LineTo(hdc, posX, posY+hy);
			RECT rect;
			rect.top = posY+1;
			rect.left = posX + 1;
			rect.right = posX + hx - 1;
			rect.bottom = posY+hy - 1;
			DrawText(hdc, matrix[i][j], lstrlen(matrix[i][j]), &rect, DT_WORDBREAK | DT_EDITCONTROL| DT_CENTER|DT_VCENTER);
			posX += hx;
		}
		
		MoveToEx(hdc, posX, posY, NULL);
		LineTo(hdc, posX, posY + hy);

		MoveToEx(hdc, 0, posY, NULL);
		LineTo(hdc, sx, posY);
		posY += hy;
	}
	MoveToEx(hdc, 0, posY, NULL);
	LineTo(hdc, sx, posY);
	bottom = posY;
}

//обработчик сообщений
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static HBRUSH solidBrush = CreateSolidBrush(RGB(0, 255, 0));
	PAINTSTRUCT ps;

	int step = 0;
	static int width, height;

	switch (message)
	{
	case WM_SIZE: 
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		ScrollWindow(hWnd, 0, -top, NULL, NULL);
		UpdateWindow(hWnd);
		top = 0;
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_VSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_LINEUP:
			step = SCROLL_SPEED;
			break;
		case SB_LINEDOWN:
			step = -SCROLL_SPEED;
			break;
		}
		top += step;
		if ((top != SCROLL_SPEED))
		{
			ScrollWindow(hWnd, 0, step, NULL, NULL);
			UpdateWindow(hWnd);
		}
		else
			top -= step;
		break;
	//обработка сообщений перерисовки
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);//дискриптор контекста устройства клиентской области окна
		DrawTable(hdc, width, height);
		ReleaseDC(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DeleteObject(solidBrush);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG msg;
	//инициализация структуры класса окна
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_CROSS);//IDC_ARROW
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "LabWork2Class";
	wcex.hIconSm = wcex.hIcon;
	
	RegisterClassEx(&wcex);//регистрация окна

	hWnd = CreateWindow("LabWork2Class", "OSISP.LabWork 2",
		WS_OVERLAPPEDWINDOW|WS_VSCROLL, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);	

	
	//отображение окна
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	SetScrollRange(hWnd, SB_CTL, 0, 10, FALSE);
	//прием сообщений
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
