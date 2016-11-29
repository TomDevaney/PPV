#pragma once
#include "ShaderStructures.h"
#include "DeviceResources.h"

class Model
{
private:
	//string filePath;
	string texturePath;
	vector<Vertex> vertices;
	vector<VS_BasicInput> basicVertices;
	vector<unsigned int> indices;
	Shadertypes vertexType;

	//devices
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader > pixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	//constant buffer data
	ModelViewProjectionConstantBuffer mvpData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mvpConstantBuffer;
	BoneOffsetConstantBuffer boneOffsetData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> boneOffsetConstantBuffer;

public:
	void CreateDevResources(ID3D11Device* device, ID3D11DeviceContext* devContext);
	void Render(ID3D11Device* device, ID3D11DeviceContext* devContext);

	//getters

	//setters 
	//void SetFilePath(string path) { filePath = path; }
	void SetTexturePath(string path) { texturePath = path; }
	void SetVertexShader(ID3D11VertexShader* vs, Shadertypes type) { vertexShader = vs; vertexType = type; }
	void SetPixelShader(ID3D11PixelShader* ps) { pixelShader = ps; }
	void SetInputLayout(ID3D11InputLayout* ip) { inputLayout = ip; }
	void SetVertices(vector<Vertex> verts) { vertices = verts; }
	void SetVertices(vector<VS_BasicInput> verts) { basicVertices = verts; }
	void SetIndices(vector<unsigned int> ind) { indices = ind; }
	void SetModel(XMMATRIX& model);
	void SetView(XMFLOAT4X4 view) { mvpData.view = view; }
	void SetProjection(XMFLOAT4X4 projection) { mvpData.projection = projection; }
};