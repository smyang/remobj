// finalapp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "finalapp.h"
#include <vector>
#include <algorithm>
#include "vis.h"
#include "vision_utilities.hpp"

#define ALMAMATER
//#define JAPAN
//#define OSAKA
//#define BUTLER

#ifdef ALMAMATER
LPCTSTR image_left = L"IMG_5535.bmp";
LPCTSTR image_right = L"IMG_5536.bmp";
char bundlefilename[] = "bundle.low.out";
char image_left_ppm[] = "IMG_5535.ppm";
char image_right_ppm[] = "IMG_5536.ppm";
#endif


// japan
#ifdef JAPAN
LPCTSTR image_left = L"IMG_1149.bmp";
LPCTSTR image_right = L"IMG_1150.bmp";
char bundlefilename[] = "bundle.sas3.out";
char image_left_ppm[] = "IMG_1149.ppm";
char image_right_ppm[] = "IMG_1150.ppm";
#endif

// osaka
#ifdef OSAKA
LPCTSTR image_left = L"IMG_0437.bmp";
LPCTSTR image_right = L"IMG_0438.bmp";
char bundlefilename[] = "bundle.out";
char image_left_ppm[] = "IMG_0437.ppm";
char image_right_ppm[] = "IMG_0438.ppm";
#endif

#ifdef BUTLER
LPCTSTR image_left = L"IMG_5540.bmp";
LPCTSTR image_right = L"IMG_5541.bmp";
char bundlefilename[] = "bundle.butler.out";
char image_left_ppm[] = "IMG_5540.ppm";
char image_right_ppm[] = "IMG_5541.ppm";
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HBITMAP g_hImage = NULL, g_hImage2 = NULL;
int g_imageWidth, g_imageHeight;
ImageColor g_img1, g_img2;
std::vector<PointPair> leftvec, rightvec;

BundleOut bo;
double g_plane_a, g_plane_b, g_plane_d;
bool g_bShowBoundary = true;

bool compare_y(const PointPair& p1, const PointPair& p2)
{
	if (p1.leftim_point.y == p2.leftim_point.y)
		return p1.leftim_point.x < p2.leftim_point.x;

	return p1.leftim_point.y < p2.leftim_point.y;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FINALAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FINALAPP));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FINALAPP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FINALAPP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   bo.parse_file(bundlefilename);
   get_least_square_plane(bo.points_3d, bo.num_points, &g_plane_a, &g_plane_b, &g_plane_d);

    if (readImageColor(&g_img1, image_left_ppm)==0)
        printf("Gray-scale image \"%s\" properly read\n", image_left_ppm);
    else {
        printf("Error reading binary image \"%s\"\n", image_left_ppm);
        return -1;
    }

    if (readImageColor(&g_img2, image_right_ppm)==0)
        printf("Gray-scale image \"%s\" properly read\n", image_right_ppm);
    else {
        printf("Error reading binary image \"%s\"\n", image_right_ppm);
        return -1;
    }

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1300, 800, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;


	static std::vector<PointPair> points;
	static Position scene_point;
	static POINTS point_im1, point_im2;

			RECT text_rect;
			text_rect.left = 0;
			text_rect.right = text_rect.left + 500;
			text_rect.top = 480;
			text_rect.bottom = text_rect.top + 100;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
    case WM_CREATE:
        g_hImage = (HBITMAP)LoadImage(hInst, image_left, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		g_hImage2 = (HBITMAP)LoadImage(hInst, image_right, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if(g_hImage == NULL)
            MessageBox(hWnd, L"Could not load image!", L"Error", MB_OK | MB_ICONEXCLAMATION);
		if(g_hImage2 == NULL)
            MessageBox(hWnd, L"Could not load image!", L"Error", MB_OK | MB_ICONEXCLAMATION);
    break;

	case WM_PAINT:
		{
			BITMAP bm;
			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hWnd, &ps);

			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, g_hImage);

			// blit the left image
			GetObject(g_hImage, sizeof(bm), &bm);
			BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

			int wid = bm.bmWidth;
			g_imageWidth = bm.bmWidth;
			g_imageHeight = bm.bmHeight;

			// blit the right image
			SelectObject(hdcMem, g_hImage2);
			GetObject(g_hImage2, sizeof(bm), &bm);
			BitBlt(hdc, wid, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

			TCHAR str[1024];
			char str2c[1024];
			TCHAR str2[1024];

			/*swprintf(str, L"Cursor: (%d %d)\n", mx, my);
			sprintf(str2c, "Scene Plane: (%lf) X + (%lf) Y + Z + (%lf) = 0\n", g_plane_a, g_plane_b, g_plane_d);
			mbstowcs(str2, str2c, 100);
			wcscat(str, str2);
			DrawText(hdc, str, wcslen(str), &text_rect, DT_LEFT);*/
			//text_rect
			//DrawText(hdc, str, wcslen(str), &text_rect, DT_LEFT);

			HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 25, 5));
			SelectObject(hdc, hPen);
			if (g_bShowBoundary) {
				for (int i=1;i<points.size();i++) {
					MoveToEx(hdc, (int) points[i].leftim_point.x, (int) points[i].leftim_point.y, (LPPOINT) NULL);
					LineTo(hdc, (int) points[i-1].leftim_point.x, (int) points[i-1].leftim_point.y);
					MoveToEx(hdc, (int) points[i].rightim_point.x + g_imageWidth, (int) points[i].rightim_point.y, (LPPOINT) NULL);
					LineTo(hdc, (int) points[i-1].rightim_point.x + g_imageWidth, (int) points[i-1].rightim_point.y);
				}
			}
			DeleteObject(hPen);

			for (int i=0;i<leftvec.size();i++) {
				int left_x  = leftvec[i].leftim_point.x;
				int right_x = rightvec[i].leftim_point.x;
				int y = leftvec[i].leftim_point.y;

				for (int j=left_x; j<=right_x; j++) {
					int im2x = interpolate(left_x, right_x, j, leftvec[i].rightim_point.x, rightvec[i].rightim_point.x);
					int im2y = interpolate(left_x, right_x, j, leftvec[i].rightim_point.y, rightvec[i].rightim_point.y);
					int r = getPixelColor(&g_img2, im2y, im2x, 1);
					int g = getPixelColor(&g_img2, im2y, im2x, 2);
					int b = getPixelColor(&g_img2, im2y, im2x, 3);
					hPen = CreatePen(PS_SOLID, 1, RGB(r, g, b));
					
					SelectObject(hdc, hPen);
					MoveToEx(hdc, j, y, NULL);
					LineTo(hdc, j+1, y+1);
					DeleteObject(hPen);
				}
			}
			
			SelectObject(hdcMem, hbmOld);
			DeleteDC(hdcMem);

			EndPaint(hWnd, &ps);
	    }
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
    case WM_MOUSEMOVE:
	{
		int mx=0, my=0;
		int m2x, m2y;
		int last_mx, last_my;
		int last_m2x, last_m2y;
		static Position lastp3d(0,0,0);

		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		InvalidateRect(hWnd, &text_rect, TRUE);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		TCHAR str[4096];
		char str2c[4096];
		TCHAR str2[4096];

		if (wParam && MK_LBUTTON) {
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 25, 5));
			SelectObject(hdc, hPen);

			PointPair pp;
			pp.leftim_point = MAKEPOINTS(lParam);
			point_im1.x = pp.leftim_point.x;
			point_im1.y = pp.leftim_point.y;

			Position p3d;
			Plane plane;
			plane.a = g_plane_a;
			plane.b = g_plane_b;
			plane.d = g_plane_d;

			image_point_to_scene_point(mx-g_imageWidth/2, g_imageHeight-my-g_imageHeight/2, &p3d, &bo.camera[0], plane);

			// XXX: strange bug here, workaround
			if (abs(p3d.x)>100) {
				p3d = lastp3d;
			}
			lastp3d = p3d;

			scene_point.x = p3d.x;
			scene_point.y = p3d.y;
			scene_point.z = p3d.z;

			double resx, resy;
			scene_point_to_image_point(&p3d, &bo.camera[1], &resx, &resy);
			m2x = (int)(resx+g_imageWidth/2);
			m2y = (int)(g_imageHeight-resy-g_imageHeight/2);

			pp.rightim_point.x = point_im2.x = m2x;
			pp.rightim_point.y = point_im2.y = m2y;
			m2x+=g_imageWidth; // shift to right image

			//pp.rightim_point = 
			points.push_back(pp);

			if (points.size()<=1) {
				last_mx = mx;
				last_my = my;
				last_m2x = m2x;
				last_m2y = m2y;
			}else {
				last_mx = points[points.size()-2].leftim_point.x;
				last_my = points[points.size()-2].leftim_point.y;
				last_m2x = points[points.size()-2].rightim_point.x +g_imageWidth; // shift to right image
				last_m2y = points[points.size()-2].rightim_point.y;
			}

			RECT r;
			r.left = min(last_mx, mx)-10;
			r.right = max(last_mx, mx)+10;
			r.top = min(last_my, my)-10;
			r.bottom = max(last_my, my)+10;

			// draw the connecting line segment
			MoveToEx(hdc, (int)last_mx, (int) last_my, (LPPOINT) NULL); 
			LineTo(hdc, (int)mx, (int)my); 
			MoveToEx(hdc, (int)last_m2x, (int) last_m2y, (LPPOINT) NULL); 
			LineTo(hdc, (int)m2x, (int)m2y); 

			InvalidateRect(hWnd, &r, FALSE);

			r.left = min(last_m2x, m2x)-10;
			r.right = max(last_m2x, m2x)+10;
			r.top = min(last_m2y, m2y)-10;
			r.bottom = max(last_m2y, m2y)+10;
			InvalidateRect(hWnd, &r, FALSE);

			wsprintf(str, L"Cursor: (%d %d) LButton Pressed, (%d %d)\n", mx, my, last_mx, last_my);
		}else{
			wsprintf(str, L"Cursor: (%d %d)\n", mx, my);
		}

		sprintf(str2c, "Scene Plane: (%lf) X + (%lf) Y + Z + (%lf) = 0\n", g_plane_a, g_plane_b, g_plane_d);
		mbstowcs(str2, str2c, 200);
		wcscat(str, str2);
		sprintf(str2c, "Scene Point: (%lf, %lf, %lf)\n", scene_point.x, scene_point.y, scene_point.z);
		mbstowcs(str2, str2c, 300);
		wcscat(str, str2);
		
		sprintf(str2c, "Left Image Point: (%d, %d)\nRight Image Point: (%d, %d)\n", point_im1.x, point_im1.y, point_im2.x, point_im2.y);
		mbstowcs(str2, str2c, 200);
		wcscat(str, str2);

		DrawText(hdc, str, wcslen(str), &text_rect, DT_LEFT);

		EndPaint(hWnd, &ps);
		UpdateWindow(hWnd);
		
		break;
	}
	case WM_LBUTTONDOWN:
	{
		g_bShowBoundary = true;
		points.clear();
		leftvec.clear();
		rightvec.clear();
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, 0, 0);
		break;
	}
	case WM_LBUTTONUP:
	{
		g_bShowBoundary = false;
		points.push_back(points[0]);
		std::vector<PointPair> newvec;
		
		for (int i=1;i<points.size();i++) {
			int y_start = points[i-1].leftim_point.y;
			int y_end   = points[i  ].leftim_point.y;
			int x_start = points[i-1].leftim_point.x;
			int x_end   = points[i  ].leftim_point.x;

			int y2_start = points[i-1].rightim_point.y;
			int y2_end   = points[i  ].rightim_point.y;
			int x2_start = points[i-1].rightim_point.x;
			int x2_end   = points[i  ].rightim_point.x;

			int inc;
			if (y_start > y_end) 
				inc = -1;
			else 
				inc = 1;

			for (int j=y_start; (y_start>y_end)?(j>=y_end):(j<=y_end);j+=inc) {
				PointPair pp;
				pp.leftim_point.x = interpolate(y_start,y_end,j,x_start,x_end);
				pp.leftim_point.y = j;
				pp.rightim_point.x = interpolate(y_start,y_end,j,x2_start,x2_end);
				pp.rightim_point.y = interpolate(y_start,y_end,j,y2_start,y2_end);
				newvec.push_back(pp);
			}
		}

		points=newvec;
		std::sort(points.begin(), points.end(), compare_y);

		int counter = -1;
		for (int i=0;i<points.size();) {
			int j;
			for (j=i+1;j<points.size();j++) {
				if (points[j].leftim_point.y != points[i].leftim_point.y)
					break;
			}
			leftvec.push_back(points[i]);
			rightvec.push_back(points[j-1]);
			i=j;
		}
		points=leftvec;
		for (int i=0;i<rightvec.size();i++)
			points.push_back(rightvec[i]);

		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, 0, 0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
