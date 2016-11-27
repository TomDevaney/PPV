#include "Window.h"

//function prototype
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void Window::Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RegisterWindowClass(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	CreateWind(hInstance);
	ShowWind(nCmdShow);
}

int Window::Update(WPARAM& wparam)
{
	MSG msg;
	bool handledMsg = false;

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_KEYDOWN)
		{
			handledMsg = true;
			wparam = msg.wParam;
		}
		
		if (msg.message == WM_MOUSEMOVE)
		{
			handledMsg = true;
		}
		
		if (!handledMsg) //let windows handle message
		{
			//send message to windowproc
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.message;
}

void Window::RegisterWindowClass(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//clear out window class
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	//fill in window class struct
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW; //so if we move the window vertically or horizontally, it will be redrawn
	wc.lpfnWndProc = WindowProc; //points to the window procedure, which will handle messages
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); //loads the cursor image
	//wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 255));
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW; //I'm not sure if it's needed because directx render target view will color window
	wc.lpszClassName = "WindowClass1";

	//register the window class
	RegisterClassEx(&wc);
}

void Window::CreateWind(HINSTANCE hInstance)
{
	RECT wr = { 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE); // This function takes in client size (where we want to draw) and returns rect that is window size (includes bars)

	hwnd = CreateWindowEx(NULL, "WindowClass1", "PPV Ubihard", WS_OVERLAPPEDWINDOW, 300, 25, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	bool result;

	if (!hwnd)
	{
		result = false;
	}
}

void Window::ShowWind(int nCmdShow)
{
	ShowWindow(hwnd, nCmdShow);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	//default:
	//	DefWindowProc(hwnd, message, wParam, lParam);
	//	break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

//getters
const HWND Window::GetHWND()
{
	return hwnd;
}