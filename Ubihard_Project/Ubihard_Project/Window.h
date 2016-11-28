#pragma once
#include "Includes.h"
#include <windows.h>
#include <windowsx.h>

class Window
{
private:
	WNDCLASSEX wc;
	HWND hwnd;


	//Window helper functions
	void RegisterWindowClass(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	void CreateWind(HINSTANCE hInstance);
	void ShowWind(int nCmdShow);

public:
	void Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow); //initializes window
	int Update(WPARAM& wparam, LPARAM& lparam); //runs window msg loop and also handles running directx11 stuff
	const HWND GetHWND();
};