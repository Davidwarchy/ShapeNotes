//Obtained from https://gist.github.com/atoz-programming-tutorials/f0c00244acf913c086f3eb9840dec614#file-win32_gdiplus-cpp-L8 
//run with g++ win32_gdiplus.cpp -lgdiplus -lgdi32

#include <windows.h>
#include <gdiplus.h>//GDI functions for Graphics, Pens and Brushes and more
#include <windowsx.h>//GET_X_LPARAM, GET_Y_LPARAM
#include <iostream>//for printing to console
#include <tuple>//returning a bunch of numbers

#define MAX_LOADSTRING 100
#define UNICODE

BOOL fDraw = FALSE; 
POINT ptPrevious; 

/* Global Variables **********************************************************/
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];	

typedef struct tagShape
{
	RECT rect;
	UINT shapeID;
} Shape;


const UINT SHAPE_COUNT = 5;
Shape Shapes[SHAPE_COUNT]; 
UINT nCurrentIndex = 0;

bool  isRubberBand = false;
POINT ptStart;
POINT ptCurrent;

LRESULT OnPaint       (HWND hwnd);
LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);
void draw(HDC hdc);
void drawCircle(HDC hdc, int pt1x, int pt1y, int pt2x, int pt2y);
void drawStuff(HWND hWnd, HDC hdc, int xPos, int yPos);
std::tuple<int, int, int, int> dim(int pt1x, int pt1y, int pt2x, int pt2y);
void paintMouseOnRect(HDC hdc);

int WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, PSTR cmdLine, INT cmdCount) {
	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	// Register the window class
	const char *CLASS_NAME = "myWin32WindowClass";
	WNDCLASS wc{};
	wc.hInstance = currentInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProcessMessages;
	RegisterClass(&wc);

	// Create the window
	HWND hwnd = CreateWindow(CLASS_NAME, "Win32 Drawing with GDI+ Tutorial",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,            // Window style
		CW_USEDEFAULT, CW_USEDEFAULT,                // Window initial position
		800, 600,                                    // Window size
		nullptr, nullptr, nullptr, nullptr);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

	// Window loop
	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg) {
	case WM_CREATE:
		{
			//C: Set the initial drawing mode.
			HMENU hMenu = GetMenu(hwnd);
			HMENU hMenuShapes = GetSubMenu(hMenu, 1);
			CheckMenuRadioItem(hMenuShapes, 1, 2, 3, MF_BYCOMMAND);
		}
	case WM_PAINT:
    return OnPaint(hwnd);
		hdc = BeginPaint(hwnd, &ps);//returns handle to to the display device context
		draw(hdc);
    paintMouseOnRect(hdc);
		EndPaint(hwnd, &ps);//ends paint and releases the dc
    ReleaseDC(hwnd, hdc);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
    
 case WM_LBUTTONDOWN: 
      ClipCursor
      fDraw = TRUE; 
      ptPrevious.x = LOWORD(lparam); 
      ptPrevious.y = HIWORD(lparam); 
      return 0L; 
   
  case WM_LBUTTONUP: 
      if (fDraw) 
      { 
          hdc = GetDC(hwnd); 
          MoveToEx(hdc, ptPrevious.x, ptPrevious.y, NULL); 
          LineTo(hdc, LOWORD(lparam), HIWORD(lparam)); 
          ReleaseDC(hwnd, hdc); 
      } 
      fDraw = FALSE; 
      return 0L; 
   
  case WM_MOUSEMOVE: 
      if (fDraw) 
      { 
          hdc = GetDC(hwnd); 
          FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
          MoveToEx(hdc, ptPrevious.x, ptPrevious.y, NULL); 
          drawCircle(hdc, ptPrevious.x, ptPrevious.y, LOWORD(lparam), HIWORD(lparam));
          std::cout<<" "<<ptPrevious.x<<" "<< ptPrevious.y<<" "<< LOWORD(lparam)<<" "<<HIWORD(lparam)<<std::endl;
          LineTo(hdc, ptPrevious.x, ptPrevious.y); 
          ReleaseDC(hwnd, hdc); 
      } 
      return 0L;
	default:
		return DefWindowProc(hwnd, msg, param, lparam);
	}
}

void draw(HDC hdc) {
	Gdiplus::Graphics gf(hdc);
	Gdiplus::Pen pen(Gdiplus::Color(255, 255, 0, 0));  
  
  // For lines, rectangles and curves
	Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 255, 0));      
  // For filled shapes
	gf.DrawLine(&pen, 0, 0, 500, 500);
	gf.FillRectangle(&brush, 0, 0, 100, 100);
	gf.DrawRectangle(&pen, 0, 0, 100, 150);

	Gdiplus::Bitmap bmp(L"images\\tilespan.png");
	gf.DrawImage(&bmp, 430, 10);
	gf.FillEllipse(&brush, 50, 400, 200, 100);
}

void drawCircle(HDC hdc, int pt1x, int pt1y, int pt2x, int pt2y) {
  using namespace std;
  int x,y,w,h;
  tie(x,y,w,h)  = dim(pt1x, pt1y, pt2x, pt2y);
	Gdiplus::Graphics gf(hdc);
	Gdiplus::Pen pen(Gdiplus::Color(255, 255, 0, 0)); 
	gf.DrawEllipse(&pen, x, y, w, h);
}

void drawStuff(HWND hWnd, HDC hdc, int xPos, int yPos){
  RECT rect;
  GetClientRect(hWnd, &rect);
  rect.left += 10; 
  rect.top += 10; 
  rect.bottom -= 10;
  int i;
  
  Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
  
  for (i = 0; i < 6; i++) 
      { 
          MoveToEx(hdc, rect.left + (i * 20) +xPos, rect.top + yPos, NULL); 
          LineTo(hdc, rect.left + (i * 20), rect.bottom);  
      }
}

std::tuple<int, int, int, int>dim(int pt1x, int pt1y, int pt2x, int pt2y){
  int x, y, h,w;

  if(pt2x < pt1x){
    x = pt2x;
    w = pt1x - pt2x;
  }else{
    x = pt1x;
    w = pt2x - pt1x;
  }
  
  if(pt2y < pt1y){
    y = pt2y;
    h = pt1y - pt2y;
  }else{
    y = pt1y;
    h = pt2y - pt1y;
  }
  
  return std::make_tuple(x, y, w, h);
}

void paintMouseOnRect(HDC hdc);  
void paintMouseOnRect(HDC hdc)
{
	HPEN hpen, hpenOld;
	HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
	hpen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
	hpenOld = (HPEN)SelectObject(hdc, hpen);
	SetBkColor(hdc, RGB(255, 255, 255));  //set the pen background
	SetBkMode(hdc, OPAQUE);  //set the pen BG mode to overwrite current background

  // do something...
	int top = 0;
	int left = 0;
	int buttom = 400;
	int right = 400;
	Rectangle(hdc, left, top, right, buttom);

	//return the pen
	SelectObject(hdc, hpenOld);
	DeleteObject(hpen);
	SelectObject(hdc, oldBrush);
	DeleteObject(oldBrush);
	DeleteObject(hpenOld);
}

//this is a handy piece of code... but it doesn't store anything we paint on the canvas. i think there might lie a clue about how one could operate their program with this.
//this only paints shaes already stored.
LRESULT OnPaint       (HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	hdc = ::BeginPaint(hwnd, &ps);

	UINT index;
	for (index = 0; index < SHAPE_COUNT; index++)
	{ 
    std::cout<<Shapes[index].shapeID<<std::endl;
		if (0 == Shapes[index].shapeID)
		{
			::Rectangle	(	
						hdc, 
						Shapes[index].rect.left, 
						Shapes[index].rect.top, 
						Shapes[index].rect.right,
						Shapes[index].rect.bottom
						);
		}
		else
		{
			::Ellipse	(	
						hdc, 
						Shapes[index].rect.left, 
						Shapes[index].rect.top, 
						Shapes[index].rect.right,
						Shapes[index].rect.bottom
						);
		}
	}

	::EndPaint(hwnd, &ps);

	return 0;	
}
