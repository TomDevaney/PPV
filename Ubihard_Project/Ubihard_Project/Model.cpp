#include "Model.h"

void Model::CreateDevResources(ID3D11Device* device, ID3D11DeviceContext* devContext)
{
	//create vertex bufer
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = vertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex) * vertices.size(), D3D11_BIND_VERTEX_BUFFER);

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());

	//create index buffer
	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = indices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * indices.size(), D3D11_BIND_INDEX_BUFFER);

	device->CreateBuffer(&indexBufferDesc, &indexBufferData, indexBuffer.GetAddressOf());

	//create constant buffers
	CD3D11_BUFFER_DESC mvpBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&mvpBufferDesc, NULL, mvpConstantBuffer.GetAddressOf());

	//create shader resource view with texture path
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

	//set index buffer
	devContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//and finally... draw model
	devContext->DrawIndexed(indices.size(), 0, 0);
}

void Model::SetModel(XMMATRIX& model)
{
	 XMFLOAT4X4 mod;

	 XMStoreFloat4x4(&mod, model);
	 mvpData.model = mod; 
}