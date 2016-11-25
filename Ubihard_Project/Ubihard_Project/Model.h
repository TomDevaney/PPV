#pragma once
#include "ShaderStructures.h"
#include "DeviceResources.h"

class Model
{
private:
	string filePath;
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	
	//devices
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader > pixelShader;

	//constant buffer data
	ModelViewProjectionConstantBuffer mvpData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mvpConstantBuffer;

public:
	void CreateDevResources(ID3D11Device* device, ID3D11DeviceContext* devContext);
	void Render(ID3D11Device* device, ID3D11DeviceContext* devContext);

	//getters

	//setters 
	void SetFilePath(string path) { filePath = path; }
	void SetVertexShader(ID3D11VertexShader* vs) { vertexShader = vs; }
	void SetPixelShader(ID3D11PixelShader* ps) { pixelShader = ps; }
	void SetVertices(vector<Vertex> verts) { vertices = verts; }
	void SetIndices(vector<unsigned int> ind) { indices = ind; }
	void SetModel(XMMATRIX& model);
	void SetView(XMFLOAT4X4 view) { mvpData.view = view; }
	void SetProjection(XMFLOAT4X4 projection) { mvpData.projection = projection; }
};