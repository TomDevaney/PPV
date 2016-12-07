#pragma once
#include "Window.h"
#include "Scene.h"
#include "DeviceResources.h"

class Application
{
private:
	//object members
	Window window;
	DeviceResources devResources;
	Scene scene;
	//Game game; //will handle any of the game logic

	//input members
	WPARAM wparam;
	LPARAM lparam;
	bool buttons[256];
	int mouseX, mouseY;
	bool rightButtonPressed, leftButtonPressed;

	//private helper functions
	void HandleInput(int message);
public:
	void Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	bool Update();
	void Shutdown();
};