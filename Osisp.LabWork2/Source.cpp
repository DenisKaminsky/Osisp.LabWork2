#include <windows.h>
#include <string>

using namespace std;

#define SCROLL_SPEED 30

const int rows = 5;
const int columns = 6;
int top = 0, bottom = 0;

string matrix[rows][columns] = {};

void GenerateMatrix()
{
	int randcounter = 0;
	int counter = 1;
	string s = "";

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			randcounter = rand() % 8 + 1;
			for (int k= 0;k<randcounter;k++)
				s+="sometext" + to_string(counter)+" ";
			matrix[i][j] = s;
			s = "";
			counter++;
		}
	}
}

float GetRowSize(HDC hdc, int rowNumber,float hx)
{
	int maxLength = 0;
	int maxIndex = 0;
	for (int i = 0; i < columns; i++)
	{
		int length = matrix[rowNumber][i].length();
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
	DrawText(hdc, matrix[rowNumber][maxIndex].c_str(), maxLength, &rect,DT_CALCRECT| DT_WORDBREAK| DT_EDITCONTROL | DT_CENTER);
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
			LPCSTR str = matrix[i][j].c_str();
			DrawText(hdc, matrix[i][j].c_str(), matrix[i][j].length(), &rect, DT_WORDBREAK | DT_EDITCONTROL| DT_CENTER|DT_VCENTER);
			posX += hx;
		}
		
		MoveToEx(hdc, posX-1, posY, NULL);
		LineTo(hdc, posX-1, posY + hy);

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
	int wheelDelta;
	static int width, height;

	switch (message)
	{
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *pInfo = (MINMAXINFO *)lParam;
		POINT Min = {columns*20, 500};
		pInfo->ptMinTrackSize = Min;
		break;
	}
	case WM_SIZE: 
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		ScrollWindow(hWnd, 0, -top, NULL, NULL);
		UpdateWindow(hWnd);
		top = 0;
		InvalidateRect(hWnd, NULL, TRUE);		
		break;
	case WM_MOUSEWHEEL:
		wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (wheelDelta > 0)
			SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
		if (wheelDelta < 0)
			SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
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
		if ( ((top != SCROLL_SPEED) && (step>0)) || ((step<0) && (bottom >= height)) )
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

	GenerateMatrix();

	//отображение окна
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);	

	//прием сообщений
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
