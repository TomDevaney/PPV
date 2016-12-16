#include "DeviceResources.h"

void DeviceResources::Init(HWND hwnd)
{
	//create swapchainDesc
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	//swapChainDesc.BufferDesc = braynsBufferDesc;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//create swapchain and device
	UINT flags = NULL;

#if (_DEBUG)
	{
		flags = D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	HRESULT swapResult = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), device.GetAddressOf(), NULL, deviceContext.GetAddressOf());

	//Set up back buffer
	HRESULT scBufferResult = swapChain.Get()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)swapChainBuffer.GetAddressOf()); //this returns address of back buffer in swapChain

	//create shadow map buffer
	D3D11_TEXTURE2D_DESC smTextDesc;
	ZeroMemory(&smTextDesc, sizeof(D3D11_TEXTURE2D_DESC));

	smTextDesc.Width = CLIENT_WIDTH;
	smTextDesc.Height = CLIENT_HEIGHT;
	smTextDesc.MipLevels = 1;
	smTextDesc.ArraySize = 1;
	smTextDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	smTextDesc.SampleDesc.Count = 1;
	smTextDesc.Usage = D3D11_USAGE_DEFAULT;
	smTextDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	smTextDesc.CPUAccessFlags = 0;
	smTextDesc.MiscFlags = 0;

	HRESULT smBufferResult = device->CreateTexture2D(&smTextDesc, NULL, shadowMapBuffer.GetAddressOf());

	//create render target view by linking with back buffer
	HRESULT rtvResult = device->CreateRenderTargetView(swapChainBuffer.Get(), nullptr, renderTargetView.GetAddressOf());

	//create shadowMapRTV by linking with shadowMapBuffer
	HRESULT smRtvResult = device->CreateRenderTargetView(shadowMapBuffer.Get(), nullptr, shadowMapRTV.GetAddressOf());

	//create depth stencil buffer 
	CD3D11_TEXTURE2D_DESC dsDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, lround((float)CLIENT_WIDTH), lround((float)CLIENT_HEIGHT), 1, 1, D3D11_BIND_DEPTH_STENCIL);
	HRESULT dsvBufferResult = device->CreateTexture2D(&dsDesc, nullptr, depthStencilBuffer.GetAddressOf());

	//create shadow map stv buffer
	CD3D11_TEXTURE2D_DESC smDsDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, lround((float)CLIENT_WIDTH), lround((float)CLIENT_HEIGHT), 1, 1, D3D11_BIND_DEPTH_STENCIL);
	HRESULT smDsvBufferResult = device->CreateTexture2D(&smDsDesc, nullptr, shadowMapDepthStencilBuffer.GetAddressOf());

	//create depth stencil view now that depth stencil buffer is done created
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	HRESULT dsvResult = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());

	//create shadow map DSV
	CD3D11_DEPTH_STENCIL_VIEW_DESC smDepthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	HRESULT smDvResult = device->CreateDepthStencilView(shadowMapDepthStencilBuffer.Get(), &smDepthStencilViewDesc, shadowMapDSV.GetAddressOf());

	//create shadow SRV
	CD3D11_SHADER_RESOURCE_VIEW_DESC smSRVDesc(D3D11_SRV_DIMENSION_TEXTURE2D);
	HRESULT smSRVResult = device->CreateShaderResourceView(shadowMapBuffer.Get(), &smSRVDesc, shadowMapSRV.GetAddressOf());

	//set render target view and link depth stencil view with rtv
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	//set up viewport
	ZeroMemory(&viewPort, sizeof(D3D11_VIEWPORT));

	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = CLIENT_WIDTH;
	viewPort.Height = CLIENT_HEIGHT;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	deviceContext->RSSetViewports(1, &viewPort);

}

void DeviceResources::Clear()
{
	//clear views
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::SkyBlue);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DeviceResources::Present()
{
	//swap back buffer with buffer
	HRESULT swapResult = swapChain->Present(1, 0);
}