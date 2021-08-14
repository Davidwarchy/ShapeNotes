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
      std::cout<<"The shape id is: "<<shapeID<<std::endl;
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
	HDC hdc;
	PAINTSTRUCT ps;
  static POINT pt;
  static RECT rcTarget;
  static BOOL bDrawing;
  static RECT rcTemp; 
  static HPEN hPenDefault = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
  static HBRUSH hBrushDefault = CreateSolidBrush(RGB(0, 0, 255));
  
	switch (msg) {
    
    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);//returns handle to to the display device context
      retrieveShapes(hwnd);
      EndPaint(hwnd, &ps);//ends paint and releases the dc
      ReleaseDC(hwnd, hdc);
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
        hdc = GetDC(hwnd);
        
        HPEN hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
        HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);
        Rectangle(hdc, rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom);
        
        SelectObject(hdc, hBrushDefault);
        SelectObject(hdc, hPenDefault);

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

        //draw rectangle
        Rectangle(hdc, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);
        rcTemp = rcTarget;
        ReleaseDC(hwnd, hdc);
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
      rcTemp.top = rcTemp.bottom = rcTemp.left = rcTemp.right = 0;
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
  cout<<forward_list_size(shapes)<<endl;
}