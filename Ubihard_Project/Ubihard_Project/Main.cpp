#include "Window.h"
#include "Scene.h"
#include "DeviceResources.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//create window
	Window projectWindow;
	projectWindow.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	//create device resources
	DeviceResources devResources;
	devResources.Init(projectWindow.GetHWND());

	//create scene
	Scene scene;
	scene.Init(&devResources);
	scene.CreateModels();

	while (true)
	{
		//render scene
		scene.Render();

		//present backbuffer and clear views
		devResources.Present();

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