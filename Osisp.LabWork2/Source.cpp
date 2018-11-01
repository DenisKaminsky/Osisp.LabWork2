#include <windows.h>
#include <string>

using namespace std;

#define SCROLL_SPEED 30
#define MAX_ROWS_COUNT 30
#define MAX_COLUMNS_COUNT 30
#define MAX_LETTER_HEIGHT 35
#define MIN_LETTER_HEIGHT 10
#define MAX_LETTER_WIDTH 20
#define MIN_LETTER_WIDTH 3
#define MIN_LINE_SPACING 3
#define MAX_LINE_SPACING 30

int rows = 5,columns = 6;//количество строк и столбцов
int top = 0, bottom = 0;//координаты Y для верха и ниха таблицы
int letterWidth = 6, letterHeight = 20;//ширина и высота шрифта
int textSpacing = 0; //межбуквенный интервал
int delta = 0;
int linesSpacing = 10, letterAngle = 0;
float epsilon = 0;//коэфициент пропорции
string** matrix;//матрица строк
COLORREF rainbow[7] = { RGB(255,0,0), RGB(255,128,0), RGB(255,255,0),RGB(0,255,0),RGB(0,255,255),RGB(0,0,255),RGB(128,0,128)};

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
				s+="sometext" + to_string(counter);
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

//получение максимальной высоты указанной строки
float GetRowHeight(int rowNumber, float hx)
{
	int maxLength = 0;
	int lettersCount = 0, length = 0;
	int rows = 0;

	for (int i = 0; i < columns; i++)
	{
		lettersCount = matrix[rowNumber][i].length();
		length = lettersCount*letterWidth + (lettersCount - 1)*textSpacing;
		if (length > maxLength)
		{
			maxLength = length;
		}
	}
	rows = (int)((maxLength / hx) +1);
	return rows*(letterHeight)+linesSpacing*(rows);
}


void TransformLetter(HDC hdc, double angle, int x, int y,string s)
{
	HDC newDC = CreateCompatibleDC(hdc);
	XFORM xf;
	int prevGraphicsMode = SetGraphicsMode(newDC, GM_ADVANCED);
	HBITMAP bmp = CreateCompatibleBitmap(hdc, 40, 40);
	HGDIOBJ prevobj = SelectObject(newDC, bmp);

	TextOut(newDC, 0, 0, s.c_str(), 1);
	xf.eM11 = 1;
	xf.eM12 = 0;
	xf.eM21 = 0;
	xf.eM22 = 1;
	xf.eDx = (float)-(letterWidth / 2);
	xf.eDy = (float)-(letterHeight / 2);
	ModifyWorldTransform(newDC, &xf, MWT_RIGHTMULTIPLY);

	xf.eM11 = (float)cos(angle);
	xf.eM12 = (float)sin(angle);
	xf.eM21 = (float)-sin(angle);
	xf.eM22 = (float)cos(angle);
	xf.eDx = 0;
	xf.eDy = 0;
	ModifyWorldTransform(newDC, &xf, MWT_RIGHTMULTIPLY);

	xf.eM11 = 1;
	xf.eM12 = 0;
	xf.eM21 = 0;
	xf.eM22 = 1;
	xf.eDx = (float)(letterWidth / 2);
	xf.eDy = (float)(letterHeight / 2);
	ModifyWorldTransform(newDC, &xf, MWT_RIGHTMULTIPLY);
	BitBlt(hdc,x,y,letterWidth,letterHeight,newDC,0,0, SRCCOPY);
	SetGraphicsMode(newDC, prevGraphicsMode);
	SelectObject(newDC, prevobj);
	DeleteDC(newDC);
}

void WrapString(HDC hdc,string str, RECT r)
{
	int x = r.left, y = r.top;
	string tempStr = "";

	for (int i = 0; i < str.length(); i++)
	{
		if ((x+letterWidth) >= r.right)
		{
			y += (letterHeight + linesSpacing);
			x = r.left;
		}
		tempStr += str[i];
		SetTextColor(hdc, rainbow[i % 7]);
		//TransformLetter(hdc, letterAngle, x, y,tempStr);
		TextOut(hdc, x, y, tempStr.c_str(), 1);
		x += (textSpacing + letterWidth);
		tempStr = "";
	}
}

//рисование таблицы
void DrawTable(HDC hdc,int sx,int sy,int borderSize)
{
	float posX = 0, posY = (float)top;
	float hx = (float)sx / columns;
	float hy = 0;
	HFONT hFont;
	RECT rect;
	string str;

	textSpacing = (int)((hx - 2 * borderSize) / 4);
	hFont = CreateFont(letterHeight,letterWidth,letterAngle, 0, FW_DONTCARE, FALSE, FALSE,
		FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH,
		TEXT("Times New Roman"));
	SelectObject(hdc, hFont);

	for (int i = 0; i < rows; i++)
	{
		hy = GetRowHeight(i, hx);
		posX = 0;
		for (int j = 0; j < columns; j++)
		{
			MoveToEx(hdc, (int)posX, (int)posY, NULL);
			LineTo(hdc, (int)posX, (int)(posY+hy));
			rect.top = (long)(posY+borderSize);
			rect.left = (long)(posX + borderSize);
			rect.right = (long)(posX + hx -borderSize);
			rect.bottom = (long)(posY+hy - borderSize);	
			//DrawText(hdc, matrix[i][j].c_str(), matrix[i][j].length(), &rect, DT_WORDBREAK | DT_EDITCONTROL | DT_LEFT);
			WrapString(hdc, matrix[i][j], rect);
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
		break;
	}
	case WM_KEYDOWN: // Обработка нажатия клавиши
		isPressed = false;
		//наклон букв
		if (wParam == 68)
		{
			letterAngle += 100;
			isPressed = true;
		}
		if (wParam == 65)
		{
			letterAngle -= 100;
			isPressed = true;
		}
		//межстрочный интервал
		if (wParam == 87)
		{
			if (linesSpacing < MAX_LINE_SPACING)
			{
				linesSpacing++;
				isPressed = true;
			}
		}
		if (wParam == 83)
		{
			if (linesSpacing > MIN_LINE_SPACING)
			{
				linesSpacing--;
				isPressed = true;
			}
		}
		//межбуквенный интервал
		/*if (wParam == 49)
		{
			delta++;
			isPressed = true;
		}*/
		//размеры букв
		if (wParam == 39) //вправо
			if (letterWidth < MAX_LETTER_WIDTH)
			{
				letterWidth++;
				isPressed = true;
			}
		if (wParam == 37) //влево
			if (letterWidth > MIN_LETTER_WIDTH)
			{
				letterWidth--;
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
		//POINT Min = {(letterWidth+textSpacing)*columns, 500};
		//pInfo->ptMinTrackSize = Min;
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
