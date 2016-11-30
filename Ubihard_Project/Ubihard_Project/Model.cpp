#include "Model.h"

void Model::CreateDevResources(ID3D11Device* device, ID3D11DeviceContext* devContext)
{
	//create vertex bufer
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	if (vertexType == Shadertypes::BASIC) //vertices are a vector of Vertex though, not VS_BasicInput, but because it's a pointer to memory, we're ok as long as the stride is good
	{
		vertexBufferData.pSysMem = basicVertices.data();

		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VS_BasicInput) * (unsigned int)basicVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
	}
	else if (vertexType == Shadertypes::BIND)
	{
		vertexBufferData.pSysMem = vertices.data();

		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex) * (unsigned int)vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
	}

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
	HRESULT hrtemp = device->CreateBuffer(&mvpBufferDesc, NULL, mvpConstantBuffer.GetAddressOf());

	//bone offsets
	if (vertexType == Shadertypes::BIND)
	{
		CD3D11_BUFFER_DESC boneOffsetDesc(sizeof(BoneOffsetConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		device->CreateBuffer(&boneOffsetDesc, NULL, boneOffsetConstantBuffer.GetAddressOf());
	}

	//create shader resource view with texture path
	wstring wideTexturePath = wstring(texturePath.begin(), texturePath.end());
	HRESULT baseTextResult = CreateDDSTextureFromFile(device, wideTexturePath.c_str(), nullptr, textureSRV.GetAddressOf());
}

void Model::Render(ID3D11Device* device, ID3D11DeviceContext* devContext)
{
	//set shaders
	devContext->VSSetShader(vertexShader.Get(), NULL, NULL);
	devContext->PSSetShader(pixelShader.Get(), NULL, NULL);

	//set input layout
	devContext->IASetInputLayout(inputLayout.Get());

	//update constant buffers
	devContext->UpdateSubresource(mvpConstantBuffer.Get(), NULL, NULL, &mvpData, NULL, NULL);

	if (vertexType == Shadertypes::BIND)
	{
		devContext->UpdateSubresource(boneOffsetConstantBuffer.Get(), NULL, NULL, &boneOffsetData, NULL, NULL);
	}

	//set constant buffers
	devContext->VSSetConstantBuffers(0, 1, mvpConstantBuffer.GetAddressOf());

	if (vertexType == Shadertypes::BIND)
	{
		devContext->VSSetConstantBuffers(1, 1, boneOffsetConstantBuffer.GetAddressOf());
	}

	//set vertex buffer
	if (vertexType == Shadertypes::BIND)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		devContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	}
	else
	{
		UINT stride = sizeof(VS_BasicInput);
		UINT offset = 0;

		devContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	}

	//set shader resource view
	devContext->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());

	//set index buffer
	if (indices.data())
	{
		devContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//and finally... draw model
		devContext->DrawIndexed((unsigned int)indices.size(), 0, 0);
	}
	else
	{
		if (vertexType == Shadertypes::BIND)
		{
			devContext->Draw((unsigned int)vertices.size(), 0);
		}
		else
		{
			devContext->Draw((unsigned int)basicVertices.size(), 0);
		}
	}
}

void Model::SetModel(XMMATRIX& model)
{
	XMFLOAT4X4 tempModel;

	XMStoreFloat4x4(&tempModel, model);
	mvpData.model = tempModel;
}

void Model::SetBoneOffsetData(XMFLOAT4X4* data)
{ 
	for (int i = 0; i < MAXBONES; ++i)
	{
		boneOffsetData.boneOffsets[i] = data[i];
	}
}