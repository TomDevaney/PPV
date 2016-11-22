#include "Window.h"

//function prototype
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void App::Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RegisterWindowClass(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	CreateWind(hInstance);
	ShowWind(nCmdShow);
}

int App::Update()
{
	MSG msg;

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			//send message to windowproc
			DispatchMessage(&msg);

			//check to see if we should quit
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else //run game code
		{

		}
	}

	return msg.wParam;
}

void App::RegisterWindowClass(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
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

void App::CreateWind(HINSTANCE hInstance)
{
	RECT wr = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(NULL, "WindowClass1", "PPV Ubihard", WS_OVERLAPPEDWINDOW, 300, 25, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	bool result;

	if (!hwnd)
	{
		result = false;
	}
}

void App::ShowWind(int nCmdShow)
{
	bool result = ShowWindow(hwnd, nCmdShow);

	cout << result;
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