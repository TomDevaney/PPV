#pragma once
#include "Includes.h"

class DeviceResources
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	//Microsoft::WRL::ComPtr<ID3D11Texture2D> swapChainBuffer;

public:
	void Init(HWND hwnd);

};