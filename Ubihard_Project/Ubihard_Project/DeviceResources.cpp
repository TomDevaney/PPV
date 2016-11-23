#include "DeviceResources.h"

void DeviceResources::Init(HWND hwnd)
{
	//create swapchainDesc
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.Windowed = true;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;

	//create swapchain and device
	UINT flags = NULL;

	if (_DEBUG)
	{
		flags = D3D11_CREATE_DEVICE_DEBUG;
	}

	HRESULT swapResult = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), device.GetAddressOf(), NULL, deviceContext.GetAddressOf());

	//Set up back buffer
	HRESULT scBufferResult = swapChain.Get()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)swapChainBuffer.GetAddressOf()); //this returns address of back buffer in swapChain

	//create render target view by linking with back buffer
	HRESULT rtvResult = device->CreateRenderTargetView(swapChainBuffer.Get(), nullptr, renderTargetView.GetAddressOf());

	//create depth stencil buffer 
	CD3D11_TEXTURE2D_DESC dsDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, lround(CLIENT_WIDTH), lround(CLIENT_HEIGHT), 1, 1, D3D11_BIND_DEPTH_STENCIL);
	HRESULT dsvBufferResult = device->CreateTexture2D(&dsDesc, nullptr, depthStencilBuffer.GetAddressOf());

	//create depth stencil view now that depth stencil buffer is done created
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	HRESULT dsvResult = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());

	//set render target view and link depth stencil view with rtv
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	//set up viewport
	ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));

	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = CLIENT_HEIGHT;
	viewPort.Height = CLIENT_HEIGHT;

	deviceContext->RSSetViewports(1, &viewPort);

}

void DeviceResources::Present()
{
	//swap back buffer with buffer
	swapChain->Present(1, 0);

	//clear views
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Red);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}