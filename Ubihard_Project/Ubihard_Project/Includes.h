#pragma once
//libraries
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

//includes
#include <d3d11.h>
#include <iostream>
#include <wrl\client.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <vector>
#include <string>
#include <d3dcompiler.h> //needed to compile shaders
#include <time.h>
//#include <vld.h> 
#include "DDSTextureLoader.h"	 
#include "../ShaderStructures/ShaderStructures.h"

//defines
#define CLIENT_WIDTH 1000
#define CLIENT_HEIGHT 800

//enums
enum VertexShaderTypes
{
	vsBASIC = 0,
	vsBIND,
	vsDEPTHPREPASS,
	vsDEPTHSHADOW,
	vsSHADOW
};
enum PixelShaderTypes
{
	psBASIC = 0,
	psNORMALMAPPED,
	psDEPTHSHADOW,
	psSHADOW,
};

using namespace std;