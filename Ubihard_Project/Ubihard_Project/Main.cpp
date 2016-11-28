#include "Window.h"
#include "Scene.h"
#include "DeviceResources.h"

//maybe put everything in a class called App, and then winmain just has an app and that's it

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

	//create wparam and lparam
	WPARAM wparam;
	LPARAM lparam; 

	//create keyboard buffer
	bool buttons[256];
	memset(buttons, 0, sizeof(buttons)); //zero them out

	//create mouse variables
	int mouseX, mouseY;
	bool rightButtonPressed, leftButtonPressed;

	while (true)
	{
		//clear wparam and lparam
		wparam = 0;
		lparam = 0;

		//clear views
		devResources.Clear();

		//render scene (every object)
		scene.Render();

		//present backbuffer
		devResources.Present();

		//check window for messages and get input
		int windowResult = projectWindow.Update(wparam, lparam); 

		if (windowResult == WM_QUIT) 
		{
			break;
		}
		else //handle game code
		{
			
		}

		//set button based off of wparam
		if (windowResult == WM_KEYDOWN)
		{
			buttons[wparam] = true;
		}
		else if (windowResult == WM_KEYUP)
		{
			buttons[wparam] = false;
		}

		//check if mouse button are beind held
		if (wparam & MK_RBUTTON)
		{
			rightButtonPressed = true;
		}
		else
		{
			rightButtonPressed = false;
		}

		if (wparam & MK_LBUTTON)
		{
			leftButtonPressed = true;
		}
		else
		{
			leftButtonPressed = false;
		}

		//set mouse positions based off of lparam
		mouseX = GET_X_LPARAM(lparam);
		mouseY = GET_Y_LPARAM(lparam);

		//update input members in scene
		scene.SetMousePosition(mouseX, mouseY);
		scene.SetButtons(buttons);
		scene.SetLeftClick(leftButtonPressed);
		scene.SetRightClick(rightButtonPressed);

		//update scene
		scene.Update(wparam); //handle 

	}

	return 0;
}