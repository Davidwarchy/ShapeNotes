//Obtained from https://gist.github.com/atoz-programming-tutorials/f0c00244acf913c086f3eb9840dec614#file-win32_gdiplus-cpp-L8 
//run with g++ win32_gdiplus.cpp -lgdiplus -lgdi32

#include <windows.h>
#include <gdiplus.h>//GDI functions for Graphics, Pens and Brushes and more
#include<iostream>
#include<forward_list>

using namespace std;

class Shape {
  public:
    RECT rect;
    UINT shapeID;
    void display(){
      //std::cout<<"The shape id is: "<<shapeID<<std::endl;
    }
};

//VARIABLES GLOBAL
forward_list<Shape*> shapes;

//FUNCTIONS
LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);
void drawCircle(HDC hdc);
LRESULT retrieveShapes(HWND hwnd);
template <typename T> int forward_list_size(const forward_list<T>& lst);//for returning the size of a forward list
void testShapes();
BOOL captureRect(HDC hdcWindow, HDC hdcMemDC, RECT previousRect);
BOOL restoreRect(HDC hdcWindow, HDC hdcMemDC, RECT destRect, RECT srcRect);

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
		0, 0,                // Window initial position
		500, 600,                                    // Window size
		nullptr, nullptr, nullptr, nullptr);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
  
  //add test shapes
  testShapes();

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
	HDC hdcWindow;
  static HDC hdcMemDC;
  HBITMAP hbmScreen;   
	PAINTSTRUCT ps;
  static POINT pt;            //position of mouse
  static RECT rcTarget;       //rectangle formed in last move
  static RECT rcPrevious;     //rectangle formed in previous move
  static BOOL bDrawing;       //is in drawing mode
  static HPEN hPenDefault;    //default pen
  static HBRUSH hBrushDefault; //default brush
  
  
  hPenDefault = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
  hBrushDefault = CreateSolidBrush(RGB(0, 0, 255));
  
	switch (msg) {
    
    case WM_PAINT:
      hdcWindow = BeginPaint(hwnd, &ps);//returns handle to to the display device context
      retrieveShapes(hwnd);
      EndPaint(hwnd, &ps);//ends paint and releases the dc
      ReleaseDC(hwnd, hdcWindow);
      return 0;
      
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
      
    case WM_LBUTTONDOWN:
      pt.x = (LONG) LOWORD(lparam); 
      pt.y = (LONG) HIWORD(lparam); 
      bDrawing = TRUE;
   
      return 0;
      
    case WM_MOUSEMOVE://you want the newest movement shown and the previous disappear.
      if((bDrawing==TRUE) && (param&MK_LBUTTON)){   
        
        hdcWindow = GetDC(hwnd);
        hdcMemDC = CreateCompatibleDC(hdcWindow); 
        if (!hdcMemDC){MessageBox(hwnd, "CreateCompatibleDC has failed", "Failed", MB_OK);}//incase of failure
        hbmScreen = CreateCompatibleBitmap(hdcWindow, rcPrevious.right - rcPrevious.left, rcPrevious.bottom - rcPrevious.top);
        if (!hbmScreen){MessageBox(hwnd, "CreateCompatibleBitmap Failed", "Failed", MB_OK);}
        SelectObject(hdcMemDC, hbmScreen);
        
        captureRect(hdcWindow, hdcMemDC, rcPrevious);
        
        //when mouse moves overdraw previous drawing with system color
        HPEN hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
        HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        SelectObject(hdcWindow, hPen);
        SelectObject(hdcWindow, hBrush);
        Ellipse(hdcWindow, rcPrevious.left, rcPrevious.top, rcPrevious.right, rcPrevious.bottom);
        
        //select the pen and brush for new rectangle
        SelectObject(hdcWindow, hBrushDefault);
        SelectObject(hdcWindow, hPenDefault);

        //validate position of rectangle coordinates
        if ((pt.x < (LONG) LOWORD(lparam)) && (pt.y > (LONG) HIWORD(lparam))) {
            SetRect(&rcTarget, pt.x, HIWORD(lparam), LOWORD(lparam), pt.y); 
        } 

        else if ((pt.x > (LONG) LOWORD(lparam)) && (pt.y > (LONG) HIWORD(lparam))) {
            SetRect(&rcTarget, LOWORD(lparam), 
                HIWORD(lparam), pt.x, pt.y); 
        }

        else if ((pt.x > (LONG) LOWORD(lparam)) && (pt.y < (LONG) HIWORD(lparam))) {
            SetRect(&rcTarget, LOWORD(lparam), pt.y, 
                pt.x, HIWORD(lparam)); 
        }
        else {
            SetRect(&rcTarget, pt.x, pt.y, LOWORD(lparam), 
                HIWORD(lparam)); 
        }
        //restore what was there
        restoreRect(hdcWindow, hdcMemDC, rcPrevious, rcPrevious);
        
        //draw rectangle
        Ellipse(hdcWindow, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);
        rcPrevious = rcTarget;
        rcPrevious = rcTarget;
        ReleaseDC(hwnd, hdcWindow);
      }
      return 0;
    case WM_LBUTTONUP:
      if(bDrawing==TRUE){
        Shape* p  = new Shape();
        shapes.push_front(p);
        p->rect = rcTarget;
        p->shapeID = 2;
      }
      bDrawing = FALSE;
      rcPrevious.top = rcPrevious.bottom = rcPrevious.left = rcPrevious.right = 0;
      return 0;
    default:
      return DefWindowProc(hwnd, msg, param, lparam);
	}
}

//we need
//Shape structure, number of shapes
LRESULT retrieveShapes(HWND hwnd)
{
  int no_of_shapes = forward_list_size(shapes);
  
  //if there are no shapes
  if(no_of_shapes<=0){
    return 0;
  }
  
  //if there are any shapes
	PAINTSTRUCT ps;
	HDC			hdc;
	hdc = BeginPaint(hwnd, &ps);

  for (Shape* a : shapes) Ellipse(hdc, a->rect.left, a->rect.top, a->rect.right ,a->rect.bottom);

	EndPaint(hwnd, &ps);

	return 0;	
}

template <typename T> int forward_list_size(const std::forward_list<T>& lst)
{
   return (std::distance(lst.begin(), lst.end()));
}

void testShapes(){
  Shape* p = new Shape();
  p->rect.top=20,p->rect.bottom=200,p->rect.right=200,p->rect.left=20;
  p->shapeID = 2;
  p->display();
  shapes.push_front(p);
  //cout<<forward_list_size(shapes)<<endl;
}

//function to repaint shapes that have been obstructed or drawn over.
BOOL captureRect(HDC hdcWindow, HDC hdcMemDC, RECT previousRect){
   //make transfer from the window to our memory
   if (!StretchBlt(hdcMemDC,
                    0, 0, previousRect.right, previousRect.bottom,
                    hdcWindow,
                    previousRect.left, previousRect.top, previousRect.right, previousRect.bottom,
                    SRCCOPY))
    {MessageBox(WindowFromDC(hdcWindow), "StretchBlt has failed", "Failed", MB_OK);return TRUE;}else{return FALSE;}
   
}

BOOL restoreRect(HDC hdcWindow, HDC hdcMemDC, RECT destRect, RECT srcRect){
     //make transfer from the memory to our window
   if (!StretchBlt(hdcWindow,
                    destRect.left, destRect.top, destRect.right, destRect.bottom,
                    hdcMemDC,
                    srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                    SRCCOPY))
    {MessageBox(WindowFromDC(hdcWindow), "StretchBlt has failed", "Failed", MB_OK);return TRUE;}else return FALSE;
}