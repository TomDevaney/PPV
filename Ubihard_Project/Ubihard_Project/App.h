#pragma once
#include "Includes.h"
#include <windows.h>
#include <windowsx.h>

class Window
{
//Window Members
private:
	WNDCLASSEX wc;
	HWND hwnd;

	//Window helper functions
	void RegisterWindowClass(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	void CreateWind(HINSTANCE hInstance);
	void ShowWind(int nCmdShow);
private:

public:
	void Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow); //initializes window
	int Update(); //runs window msg loop and also handles running directx11 stuff
};