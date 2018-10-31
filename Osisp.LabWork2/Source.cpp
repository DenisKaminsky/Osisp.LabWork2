#include <windows.h>
#include <string>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib") 

using namespace std;
using namespace Gdiplus;

#define SCROLL_SPEED 30
#define MAX_ROWS_COUNT 30
#define MAX_COLUMNS_COUNT 30
#define MAX_LETTER_HEIGHT 35
#define MIN_LETTER_HEIGHT 10
#define MAX_LETTER_WIDTH 20
#define MIN_LETTER_WIDTH 3

ULONG_PTR gdiplusToken;
int rows = 5,columns = 6;//количество строк и столбцов
int top = 0, bottom = 0;//координаты Y для верха и ниха таблицы
int letterWidth = 6, letterHeight = 20;//ширина и высота шрифта
int textSpacing = 0; //межбуквенный интервал
float epsilon = 0;//коэфициент пропорции
string** matrix;//матрица строк

//генерирование матрицы
void GenerateMatrix()
{
	int randcounter = 0;
	int counter = 1;
	string s = "";

	if (columns < 1)
		columns = 1;
	if (columns > MAX_COLUMNS_COUNT)
		columns = MAX_COLUMNS_COUNT;
	if (rows < 1)
		rows = 1;
	if (rows > MAX_ROWS_COUNT)
		rows = MAX_ROWS_COUNT;

	matrix = new string*[rows];
	for (int i = 0; i < rows; i++)
	{
		matrix[i] = new string[columns];
		for (int j = 0; j < columns; j++)
		{
			randcounter = rand() % 8 + 1;
			s += to_string(randcounter);
			for (int k= 0;k<randcounter;k++)
				s+="sometext" + to_string(counter)+" ";
			matrix[i][j] = s;
			s = "";
			counter++;
		}
	}
}

//удаление матрицы
void RemoveMatrix()
{
	for (int i = 0; i < rows; i++)
	{
		delete[] matrix[i];
	}
}

//длина неформатированного текста
int GetTextLength(HDC hdc,int i,int j)
{
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = 1;
	rect.bottom = 1;
	DrawText(hdc, matrix[i][j].c_str(), matrix[i][j].length(), &rect, DT_CALCRECT);
	return rect.right;
}

//
//получение максимальной высоты указанной строки
float GetRowHeight(HDC hdc, int rowNumber,float hx,int borderSize)
{
	int maxLength = 0, maxIndex = 0;
	RECT rect;

	for (int i = 0; i < columns; i++)
	{
		int length = matrix[rowNumber][i].length();
		if (length > maxLength)
		{
			maxLength = length;
			maxIndex = i;
		}
	}
	rect.top = 0;
	rect.left = 0;
	rect.right = (long)hx-2*borderSize;
	rect.bottom = borderSize;
	DrawText(hdc, matrix[rowNumber][maxIndex].c_str(), maxLength, &rect,DT_CALCRECT| DT_WORDBREAK | DT_EDITCONTROL | DT_CENTER);
	return (float)(rect.bottom+2*borderSize);
}

string ParseString(string str)
{
	string tempStr = "";
	char space = (char)0x20;
	int i = 0, count = (int)((textSpacing / 4));

	for (int i = 0; i < count; i++)
		tempStr += space;
	while (i < str.length())
	{
		if (str[i] != space)
		{
			str.insert(i + 1, tempStr);
			i += tempStr.length() + 1;
		}
		else
		{
			i++;
		}
	}			
	return str;
}

//рисование таблицы
void DrawTable(HDC hdc,int sx,int sy,int borderSize)
{
	float posX = 0, posY = (float)top;
	float hx = (float)sx / columns;
	float hy = 0, extTextLineLength;
	//int extTextLength, linesCount
	HFONT hFont;
	RECT rect;
	//SIZE sz;
	Graphics *g = new Graphics(hdc);
	Brush *brush = new SolidBrush(Color::RoyalBlue);
	LinearGradientBrush *br = new LinearGradientBrush(Point(0, 10), Point(200, 10), Color(255, 255, 0, 0), Color(255, 0, 0, 255));
	StringFormat *format = new StringFormat();
	Font *font;
	wstring ws;
	RectF *r;
	string str;

	//letterWidth = (int)((hx - 2 * borderSize) / 10)+5;
	//letterHeight = (int)(epsilon*letterWidth);

	textSpacing = (int)((hx - 2 * borderSize)/5);	
	hFont = CreateFont(letterHeight,letterWidth,0, 0, FW_DONTCARE, FALSE, FALSE,
		FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH,
		TEXT("Times New Roman"));
	font = new Font(hdc, hFont);
	/*SelectObject(hdc, hFont);
	SetTextCharacterExtra(hdc, textSpacing);
	GetTextExtentPoint32(hdc, matrix[0][0].c_str(), matrix[0][0].length(), &sz);
	extTextLength = sz.cx;
	linesCount = (int)((GetTextLength(hdc,0,0)/(hx -2*borderSize)) + 1);
	extTextLineLength = (float)extTextLength / linesCount;
	if (extTextLineLength < hx)
		extTextLineLength = hx;*/

	for (int i = 0; i < rows; i++)
	{
		hy = GetRowHeight(hdc,i, hx,borderSize);
		posX = 0;
		for (int j = 0; j < columns; j++)
		{
			MoveToEx(hdc, (int)posX, (int)posY, NULL);
			LineTo(hdc, (int)posX, (int)(posY+hy));
			rect.top = (long)(posY+borderSize);
			rect.left = (long)(posX + borderSize);
			rect.right = (long)(posX + hx -borderSize);//(hx)*(hx)/extTextLineLength
			rect.bottom = (long)(posY+hy - borderSize);	
			str = ParseString(matrix[i][j]);
			//DrawText(hdc, matrix[i][j].c_str(), matrix[i][j].length(), &rect, DT_WORDBREAK | DT_EDITCONTROL | DT_LEFT );
			r = new RectF(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top);
			ws = wstring(str.begin(), str.end());
			const wchar_t* resstr = ws.c_str();
			g->DrawString(resstr, str.length(), font, *r,format,br);
			posX += hx;
		}
		
		MoveToEx(hdc, (int)(posX-1), (int)posY, NULL);
		LineTo(hdc, (int)(posX-1), (int)(posY + hy));

		MoveToEx(hdc, 0, (int)posY, NULL);
		LineTo(hdc, sx, (int)posY);
		posY += hy;
	}
	MoveToEx(hdc, 0, (int)posY, NULL);
	LineTo(hdc, sx, (int)posY);
	bottom = (int)posY;

	DeleteObject(hFont);
}

//обработчик сообщений
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	int step = 0;
	int wheelDelta, MB_RESULT;
	bool isPressed;
	static int width = 0, height = 0;

	switch (message)
	{
	case WM_CREATE:
	{
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		epsilon = (float)tm.tmHeight/tm.tmAveCharWidth;
		ReleaseDC(hWnd, hdc);
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		break;
	}
	case WM_KEYDOWN: // Обработка нажатия клавиши
		isPressed = false;
		if (wParam == 39) //вправо
			if (letterWidth < MAX_LETTER_WIDTH)
			{
				letterWidth++;
				isPressed = true;
			}
		if (wParam == 37) //влево
			if (letterWidth > MIN_LETTER_WIDTH)
			{
				letterWidth++;
				isPressed = true;
			}
		if (wParam == 40) //вниз
			if (letterHeight < MAX_LETTER_HEIGHT)
			{
				letterHeight++;
				isPressed = true;
			}
		if (wParam == 38) //вверх
			if (letterHeight > MIN_LETTER_HEIGHT)
			{
				letterHeight--;
				isPressed = true;
			}
		if (wParam == 27) //если нажали ESC то выходим 
		{
			MB_RESULT = MessageBox(hWnd, "Вы действительно хотите выйти ?", "Выход", MB_YESNO);
			if (MB_RESULT == 6)
				SendMessage(hWnd, WM_DESTROY, wParam, lParam);
		}
		if (isPressed)
			InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *pInfo = (MINMAXINFO *)lParam;
		POINT Min = {(letterWidth+textSpacing)*columns, 500};
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
		DrawTable(hdc, width, height,3);
		ReleaseDC(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		RemoveMatrix();
		Gdiplus::GdiplusShutdown(gdiplusToken);
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
	GenerateMatrix();

	hWnd = CreateWindow("LabWork2Class", "OSISP.LabWork 2",
		WS_OVERLAPPEDWINDOW|WS_VSCROLL, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);	

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
