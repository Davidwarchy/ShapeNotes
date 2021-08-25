//Obtained from https://gist.github.com/atoz-programming-tutorials/f0c00244acf913c086f3eb9840dec614#file-win32_gdiplus-cpp-L8 
//run with g++ win32_gdiplus.cpp -lgdiplus -lgdi32

#include <windows.h>
#include <gdiplus.h>//GDI functions for Graphics, Pens and Brushes and more
#include<iostream>
#include<forward_list>

//for shapes menu
#define IDM_MODE_ELLIPSE    1
#define IDM_MODE_RECTANGLE  2
#define IDM_MODE_ROUNDED    3

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
HMENU hMenu;

//FUNCTIONS
LRESULT CALLBACK WindowProcessMessages(HWND hwnd, UINT msg, WPARAM param, LPARAM lparam);
void drawCircle(HDC hdc);
LRESULT retrieveShapes(HWND hwnd);
template <typename T> int forward_list_size(const forward_list<T>& lst);//for returning the size of a forward list
void testShapes();
BOOL captureRect(HDC hdcWindow, HDC hdcMemDC, RECT previousRect);
BOOL restoreRect(HDC hdcWindow, HDC hdcMemDC, RECT destRect, RECT srcRect);
void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,   HBITMAP hBMP, HDC hDC);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
void AddMenus(HWND hwnd);


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
		int(GetSystemMetrics(SM_CXMAXIMIZED)/2),
        int(GetSystemMetrics(SM_CYMAXIMIZED)/2),                                    // Window size
		nullptr, nullptr, nullptr, nullptr);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
  
  //add test shapes
  //testShapes();

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
	HDC hdcWindow;                //drawing context to main window
  HDC hdcMemDCTo;               //drawing context to memory place
  HBITMAP hbmRcTo;              //handle to the bitmap of image we are capturing
  static PBITMAPINFO pbmi;
  static char* lpbitmap;        //pointer to the bytes 
	PAINTSTRUCT ps;               //bitmap being restored
  static POINT pt;              //position of mouse
  static RECT rcTarget;         //rectangle formed in last move
  static RECT rcPrevious;       //rectangle formed in previous move
  static BOOL bDrawing;         //is in drawing mode
  static BOOL bGaugingSize;     //is true if the user is just deciding what size the image is
  
  static HPEN hPenDefault;      //default pen
  static HBRUSH hBrushDefault;  //default brush
  static int shapeMode;
  static COLORREF dColor;
  
	switch (msg) {
    
     case WM_CREATE:
     {
        //add menu
        AddMenus(hwnd);  
        
        shapeMode = IDM_MODE_ELLIPSE;
        break;
     } 
     
     case WM_COMMAND:
        switch(LOWORD(param)) {
          case IDM_MODE_ELLIPSE:
              CheckMenuRadioItem(hMenu, IDM_MODE_ELLIPSE, IDM_MODE_ROUNDED, IDM_MODE_ELLIPSE, MF_BYCOMMAND);
              shapeMode = IDM_MODE_ELLIPSE;
              break;

          case IDM_MODE_RECTANGLE:
              CheckMenuRadioItem(hMenu, IDM_MODE_ELLIPSE, IDM_MODE_ROUNDED, IDM_MODE_RECTANGLE, MF_BYCOMMAND);
              shapeMode = IDM_MODE_RECTANGLE;
              break;

          case IDM_MODE_ROUNDED:
              CheckMenuRadioItem(hMenu, IDM_MODE_ELLIPSE, IDM_MODE_ROUNDED, IDM_MODE_ROUNDED, MF_BYCOMMAND);
              shapeMode = IDM_MODE_ROUNDED;
              break;
        }
       break;
            
    case WM_PAINT:
      hdcWindow = BeginPaint(hwnd, &ps);//returns handle to to the display device context
      //retrieveShapes(hwnd);
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
        
        //###GET RECTANGLE CUT BY MOUSE MOVEMENT FROM POINT OF MOUSEDOWN
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
        
        //###RESTORE IMAGE UNDER RECTANGLE BEFORE PREVIOUS MOVE IF USER IS GAUGING SIZE
        if(bGaugingSize){
            //send color bits form the memory buffer to the hdc
            //results is the number of lines sets in the hdc. we can check whether this is still the same number as the height of the bitmap.
            int result = SetDIBitsToDevice(hdcWindow,                   //handle to the device context we are drawing to
                                           rcPrevious.left,             //$upper left x coord of destination rect
                                           rcPrevious.top,              //upper left y coord of the destination rectangle.
                                           pbmi->bmiHeader.biWidth,     //width of the image in pixels
                                           pbmi->bmiHeader.biHeight,    //heigth of the image in pixels
                                           0,                           //x-coord of lower left corner of the image
                                           0,                           //y-coord of lower left corner of the image
                                           0,                           //starting scanning lines
                                           pbmi->bmiHeader.biHeight,    //number of DIB scan lines contained in the array pointed to by lpvBits
                                           lpbitmap,                    //$a pointer to the color data stored in an array of bytes
                                           pbmi,                        //$a pointer to the BMI structure containing info about he DIB
                                           DIB_RGB_COLORS               //$whether the bmiColors member of the BMI has color indeces
                                           );
            // Free memory.
            GlobalFree((HGLOBAL)lpbitmap);
            
        }
        
        //###CAPTURE IMAGE UNDER RECTANGLE
        //create compatible dc
        hdcMemDCTo = CreateCompatibleDC(hdcWindow);
        if (!hdcMemDCTo)
        {
            MessageBox(hwnd, "CreateCompatibleDC has failed", "Failed", MB_OK);
            return 0;
        }
        
        //create compatible bitmap
        hbmRcTo = CreateCompatibleBitmap(hdcWindow, rcTarget.right - rcTarget.left, rcTarget.bottom - rcTarget.top);
        //!check width and height of the bitmap created at this point. what is it's nature.
        if (!hbmRcTo)
        {
            MessageBox(hwnd, "CreateCompatibleBitmap Failed", "Failed", MB_OK);
            return 0;
        }
        
        //select compatible bitmap into compatible dc
        SelectObject(hdcMemDCTo, hbmRcTo);
        
        //!the pbmi can also show us the dimensions of the bitmap info. we could check the width and the height and the colors being used.
        pbmi = CreateBitmapInfoStruct(hwnd, hbmRcTo);//the hwnd is only for displaying errors.
        
        //get bytes from the bitmap into memory
        HANDLE hDIB = NULL;
        hDIB = GlobalAlloc(GHND, pbmi->bmiHeader.biSizeImage);
        lpbitmap = NULL;    
        lpbitmap = (char*)GlobalLock(hDIB);
        
        if (!BitBlt(hdcMemDCTo,
              0, 0,
              rcTarget.right - rcTarget.left+1, rcTarget.bottom - rcTarget.top+1,
              hdcWindow,
              rcTarget.left, rcTarget.top,
              SRCCOPY))
          {
              cout<<"BitBlt has failed"<<endl;
              return FALSE;
          }
        
        // Gets the bytes from the bitmap, and copies them into a buffer 
        // that's pointed to by lpbitmap (the pointer in the bitmap structure to where the imge bytes pixell bytes are.
        GetDIBits(hdcMemDCTo, hbmRcTo, 0,
        (UINT)pbmi->bmiHeader.biHeight,
        lpbitmap,
        pbmi, DIB_RGB_COLORS);
        
        
        hPenDefault = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        hBrushDefault = CreateSolidBrush(dColor);
        SelectObject(hdcWindow, hPenDefault);
        SelectObject(hdcWindow, hBrushDefault);
        
        //DRAW RECTANGLE
        switch(shapeMode){
            case IDM_MODE_ELLIPSE:
                Ellipse(hdcWindow, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);
                break;
            case IDM_MODE_RECTANGLE:
                Rectangle(hdcWindow, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);
                break;
            case IDM_MODE_ROUNDED:
                RoundRect(hdcWindow, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom, 100, 100);
                break;
        }
                
        rcPrevious = rcTarget;
        bGaugingSize = TRUE;
        ReleaseDC(hwnd, hdcWindow);
        
        //free up
        DeleteDC(hdcMemDCTo);
        DeleteObject(hPenDefault);
        DeleteObject(hBrushDefault);
      }
      return 0;
    case WM_LBUTTONUP:
      if(bDrawing==TRUE){
        // Shape* p  = new Shape();
        // shapes.push_front(p);
        // p->rect = rcTarget;
        // p->shapeID = 2;
      }
      bDrawing = FALSE;
      bGaugingSize = FALSE;
      GlobalFree((HGLOBAL)lpbitmap);
      return 0;
    default:
      return DefWindowProc(hwnd, msg, param, lparam);
	}
}

//we need
//Shape structure, number of shapes
LRESULT retrieveShapes(HWND hwnd)
{
  
  static HPEN hPenDefault;      //default pen
  static HBRUSH hBrushDefault;  //default brush
    hPenDefault = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
  hBrushDefault = CreateSolidBrush(RGB(0, 0, 255));
  
  int no_of_shapes = forward_list_size(shapes);
  
  //if there are no shapes
  if(no_of_shapes<=0){
    return 0;
  }
  cout<<no_of_shapes<<endl;
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



PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)//window handle is only for displaying messages
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        //errhandler("GetObject", hwnd); 
        MessageBox(hwnd,"cant retrieve bitmap color format, width and height", "Failed", MB_OK);

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

     if (cClrBits < 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (1<< cClrBits)); 

     // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
     pbmi->bmiHeader.biClrImportant = 0; 
     return pbmi; 
 }
 
 void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, 
                  HBITMAP hBMP, HDC hDC) 
 { 
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    BYTE *hp;                   // byte pointer  
    DWORD dwTmp; 
    static int fileNum;

    pbih = (PBITMAPINFOHEADER) pbi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits) 
         //errhandler("GlobalAlloc", hwnd); 
          MessageBox(hwnd, "Error in lpbits", "Failed", MB_OK);
          return;

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.
    //The GetDIBits function retrieves the bits of the specified
    //compatible bitmap and copies them into a buffer as a DIB 
    //using the specified format.
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
        DIB_RGB_COLORS)) 
    {
        //errhandler("GetDIBits", hwnd); 
        MessageBox(hwnd, "can't retrieve color table", "Failed", MB_OK);
    }

    // Create the .BMP file.  
    hf = CreateFile(pszFile, 
                   GENERIC_READ | GENERIC_WRITE, 
                   (DWORD) 0, 
                    NULL, 
                   CREATE_ALWAYS, 
                   FILE_ATTRIBUTE_NORMAL, 
                   (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
        //errhandler("CreateFile", hwnd); 
        MessageBox(hwnd, "can't create bmp file!", "Failed", MB_OK);
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
    // Compute the size of the entire file.  
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 pbih->biSize + pbih->biClrUsed 
                 * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    pbih->biSize + pbih->biClrUsed 
                    * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
       //errhandler("WriteFile", hwnd);
       MessageBox(hwnd, "couldn't write header into the bitmpa file", "Failed", MB_OK);      
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                  + pbih->biClrUsed * sizeof (RGBQUAD), 
                  (LPDWORD) &dwTmp, ( NULL)))
        //errhandler("WriteFile", hwnd);
        MessageBox(hwnd, "couldn't copy bitmapinfo header and rgb quad array ", "Failed", MB_OK);

    // Copy the array of color indices into the .BMP file.  
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
           //errhandler("WriteFile", hwnd); 
         MessageBox(hwnd, "couldn't copy array of color indices to the bmp file", "Failed", MB_OK);

    // Close the .BMP file.  
     if (!CloseHandle(hf)) 
           //errhandler("CloseHandle", hwnd); 
            MessageBox(hwnd, "couldn't close the bmp file", "Failed", MB_OK);

    // Free memory.  
    GlobalFree((HGLOBAL)lpBits);
}

void AddMenus(HWND hwnd) {

    HMENU hMenubar;

    hMenubar = CreateMenu();
    hMenu = CreateMenu();

    AppendMenuW(hMenu, MF_STRING, IDM_MODE_ELLIPSE, L"&Ellipse");
    AppendMenuW(hMenu, MF_STRING, IDM_MODE_RECTANGLE, L"&Rectangle");
    AppendMenuW(hMenu, MF_STRING, IDM_MODE_ROUNDED, L"&Rounded Rectangle");

    CheckMenuRadioItem(hMenu, IDM_MODE_ELLIPSE, IDM_MODE_ROUNDED, IDM_MODE_ELLIPSE, MF_BYCOMMAND);

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hMenu, L"&Shapes");
    SetMenu(hwnd, hMenubar);
}