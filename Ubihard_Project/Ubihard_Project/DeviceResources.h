#pragma once
#include "Includes.h"

class DeviceResources
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> swapChainBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	D3D11_VIEWPORT viewPort;


public:
	void Init(HWND hwnd);
	void Present();

	//Getters
	ID3D11Device* GetDevice() const { return device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return deviceContext.Get(); }
	IDXGISwapChain* GetSwapChain() const { return swapChain.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }
	D3D11_VIEWPORT GetViewport() const { return viewPort; }

};