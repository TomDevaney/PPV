#pragma once
#include "Includes.h"

class DeviceResources
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> swapChainBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> shadowMapRTV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapDepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowMapDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowMapSRV;
	D3D11_VIEWPORT viewPort;


public:
	void Init(HWND hwnd);
	void Present();
	void Clear();

	//Getters
	ID3D11Device* GetDevice() const { return device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return deviceContext.Get(); }
	IDXGISwapChain* GetSwapChain() const { return swapChain.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }
	ID3D11DepthStencilView* GetShadowMapDSV() const { return shadowMapDSV.Get(); }
	ID3D11RenderTargetView* GetRenderTargetView() const { return shadowMapRTV.Get(); }
	ID3D11RenderTargetView* GetShadowMapRTV() const { return renderTargetView.Get(); }
	ID3D11RenderTargetView* const * GetShadowMapRTVAddress() const { return shadowMapRTV.GetAddressOf(); }
	ID3D11RenderTargetView* const * GetRenderTargetViewAddress() const { return renderTargetView.GetAddressOf(); }
	ID3D11ShaderResourceView* GetShadowMapSRV() const { return shadowMapSRV.Get(); }
	ID3D11ShaderResourceView* const * GetShadowMapSRVAddress() const { return shadowMapSRV.GetAddressOf(); }
	D3D11_VIEWPORT GetViewport() const { return viewPort; }

};