//Obtained from https://gist.github.com/atoz-programming-tutorials/f0c00244acf913c086f3eb9840dec614#file-win32_gdiplus-cpp-L8 
//run with g++ win32_gdiplus.cpp -lgdiplus -lgdi32

#include <windows.h>
#include <gdiplus.h>
#include <windowsx.h>//GET_X_LPARAM, GET_Y_LPARAM
#include <iostream>

LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);
void draw(HDC hdc);
void drawCircle(HDC hdc, int xPos, int yPos);
void drawStuff(HWND hWnd, HDC hdc, int xPos, int yPos);

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
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);//returns handle to to the display device context
		draw(hdc);
		EndPaint(hwnd, &ps);//ends paint and releases the dc
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
  case WM_LBUTTONUP:
    int yPos;
    int xPos;
    hdc = BeginPaint(hwnd, &ps);
    xPos = GET_X_LPARAM(lparam); 
    yPos = GET_Y_LPARAM(lparam);
    drawCircle(hdc, xPos, yPos);
    //drawStuff(hwnd, hdc, 0, 0);
    EndPaint(hwnd, &ps);
    return 0;
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

	Gdiplus::Bitmap bmp(L"tilespan.png");
	gf.DrawImage(&bmp, 430, 10);
	gf.FillEllipse(&brush, 50, 400, 200, 100);
}

void drawCircle(HDC hdc, int xPos, int yPos) {
	Gdiplus::Graphics gf(hdc);
	Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0)); 
  Gdiplus::Pen pen(Gdiplus::Color(255, 255, 0, 0));
  std::cout <<"x: "<< xPos << std::endl <<"y: "<<yPos <<std::endl;
  
	gf.FillEllipse(&brush, xPos, yPos, 200, 100);
  gf.DrawRectangle(&pen, xPos, yPos, 100, 150);
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
  