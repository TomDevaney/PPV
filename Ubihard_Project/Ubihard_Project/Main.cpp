#include "Window.h"
#include "Scene.h"
#include "DeviceResources.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//create window
	Window projectWindow;
	projectWindow.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	//create scene
	DeviceResources devResources;
	devResources.Init(projectWindow.GetHWND());

	while (true)
	{
		if (projectWindow.Update() != WM_QUIT) //run game
		{

		}
		else
		{
			break;
		}

	}

	return 0;
}