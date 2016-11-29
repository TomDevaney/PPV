#pragma once
//libraries
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

//includes
#include <iostream>
#include <wrl\client.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <vector>
#include <string>
#include <d3dcompiler.h> //needed to compile shaders
#include <time.h>
#include <vld.h>
#include "DDSTextureLoader.h"	

//defines
#define CLIENT_WIDTH 1000
#define CLIENT_HEIGHT 800

//enums
enum Shadertypes
{
	BASIC = 0,
	BIND,
};

//enum InputTypes
//{
//	BASIC = 0,
//	BIND,
//};

using namespace std;