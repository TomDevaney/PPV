#include <windows.h>
#include <windowsx.h>
//#include <vld.h>
#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application app;
	bool run = true;

	app.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	while (run)
	{
		run = app.Update();
	}

	app.Shutdown();

	return 0;
}