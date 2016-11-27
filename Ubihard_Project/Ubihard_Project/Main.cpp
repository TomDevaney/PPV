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

	//wparam is used for storing keys and mouse events
	WPARAM wparam;

	while (true)
	{
		//clear wparam
		wparam = 0;

		//clear views
		devResources.Clear();

		//render scene (every object)
		scene.Render();

		//present backbuffer
		devResources.Present();

		int windowResult = projectWindow.Update(wparam); //check window for messages

		if (windowResult == WM_QUIT) 
		{
			break;
		}
		else //handle game code
		{
			
		}

		scene.Update(wparam); //handle 

	}

	return 0;
}