#include "Application.h"

void Application::Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//initialize window
	window.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	//initialize device resources
	devResources.Init(window.GetHWND());

	//initalize scene
	scene.Init(&devResources);

	//initialize buttons
	memset(buttons, 0, sizeof(buttons)); //zero them out
}

bool Application::Update()
{
	bool result = true;
	int windowResult;

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
	windowResult = window.Update(wparam, lparam);

	if (windowResult == WM_QUIT)
	{
		result = false;
	}
	else //handle game code
	{

	}

	//handle input
	HandleInput(windowResult);

	//send scene all the input
	scene.SetMousePosition(mouseX, mouseY);
	scene.SetButtons(buttons);
	scene.SetLeftClick(leftButtonPressed);
	scene.SetRightClick(rightButtonPressed);

	//update scene
	scene.Update(wparam); //handle

	return result;
}

//private helper functions
void Application::HandleInput(int message)
{
	//set button based off of wparam
	if (message == WM_KEYDOWN)
	{
		buttons[wparam] = true;
	}
	else if (message == WM_KEYUP)
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
}

void Application::Shutdown()
{
	//clean up any memory application dynamically allocated
	scene.Shutdown();
}