#include "Model.h"

void Model::CreateDevResources(ID3D11Device* device, ID3D11DeviceContext* devContext)
{
	//create vertex bufer
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = vertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex) * (unsigned int)vertices.size(), D3D11_BIND_VERTEX_BUFFER);

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());

	//create index buffer
	if (indices.data())
	{
		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * (unsigned int)indices.size(), D3D11_BIND_INDEX_BUFFER);

		device->CreateBuffer(&indexBufferDesc, &indexBufferData, indexBuffer.GetAddressOf());
	}

	//create constant buffers
	CD3D11_BUFFER_DESC mvpBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&mvpBufferDesc, NULL, mvpConstantBuffer.GetAddressOf());

	//create shader resource view with texture path
	wstring wideTexturePath = wstring(texturePath.begin(), texturePath.end());
	HRESULT baseTextResult = CreateDDSTextureFromFile(device, wideTexturePath.c_str(), nullptr, textureSRV.GetAddressOf());

	//create sampler state
	CD3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;

	HRESULT wrapSampleResult = device->CreateSamplerState(&samplerDesc, &wrapSamplerState);
}

void Model::Render(ID3D11Device* device, ID3D11DeviceContext* devContext)
{
	//set shaders
	devContext->VSSetShader(vertexShader.Get(), NULL, NULL);
	devContext->PSSetShader(pixelShader.Get(), NULL, NULL);

	//update constant buffers
	devContext->UpdateSubresource(mvpConstantBuffer.Get(), NULL, NULL, &mvpData, NULL, NULL);

	//set constant buffers
	devContext->VSSetConstantBuffers(0, 1, mvpConstantBuffer.GetAddressOf());

	//set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	devContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	//set shader resource view
	devContext->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());

	//set sampler state
	devContext->PSSetSamplers(0, 1, wrapSamplerState.GetAddressOf());

	//set index buffer
	if (indices.data())
	{
		devContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//and finally... draw model
		devContext->DrawIndexed((unsigned int)indices.size(), 0, 0);
	}
	else
	{
		devContext->Draw((unsigned int)vertices.size(), 0);
	}
}

void Model::SetModel(XMMATRIX& model)
{
	XMFLOAT4X4 tempModel;

	XMStoreFloat4x4(&tempModel, model);
	mvpData.model = tempModel;
}