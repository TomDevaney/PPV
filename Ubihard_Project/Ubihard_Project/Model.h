#pragma once
#include "../ShaderStructures/ShaderStructures.h"
#include "DeviceResources.h"

class Model
{
private:
	std::wstring resourcesPath = L"../Resources/";

	const DeviceResources* devResources;
	ID3D11Device* device;
	ID3D11DeviceContext* devContext;

	//string filePath;
	string texturePath;
	vector<Vertex> mVertices;
	vector<VS_BasicInput> mBasicVertices;
	vector<unsigned int> mIndices;
	Shadertypes vertexType;

	//devices
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> preDepthPassVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader > pixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	//constant buffer data
	ModelViewProjectionConstantBuffer mvpData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mvpConstantBuffer;
	BoneOffsetConstantBuffer boneOffsetData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> boneOffsetConstantBuffer;

	//Private helper functions
	void DepthPrePass();
	void LightCulling();
public:
	//for model with bones
	void Init(Shadertypes shaderType, ID3D11VertexShader* vShader, ID3D11PixelShader* pShader, ID3D11InputLayout* iLayout, string tPath, XMMATRIX& model, XMFLOAT4X4 view, XMFLOAT4X4 projection, XMFLOAT4X4* boneOffData, std::wstring name);
	
	//for a basic model
	void Init(Shadertypes shaderType, ID3D11VertexShader* vShader, ID3D11VertexShader* preDepthPassVShader, ID3D11PixelShader* pShader, ID3D11InputLayout* iLayout, string tPath, XMMATRIX& model, XMFLOAT4X4 view, XMFLOAT4X4 projection, std::wstring name);
	void SetBasicVerts(vector<VS_BasicInput> verts) { mBasicVertices = verts; }
	void SetIndices(vector<unsigned int> ind) { mIndices = ind; }
	void LoadMesh(std::wstring name);
	void LoadBasicMesh(std::wstring name);
	void CreateDevResources(DeviceResources const * deviceResources);
	void Render();

	//getters

	//setters 
	//void SetFilePath(string path) { filePath = path; }
	//void SetTexturePath(string path) { texturePath = path; }
	//void SetVertexShader(ID3D11VertexShader* vs, Shadertypes type) { vertexShader = vs; vertexType = type; }
	//void SetPixelShader(ID3D11PixelShader* ps) { pixelShader = ps; }
	//void SetInputLayout(ID3D11InputLayout* ip) { inputLayout = ip; }
	//void SetVertices(vector<Vertex> verts) { vertices = verts; }
	//void SetVertices(vector<VS_BasicInput> verts) { basicVertices = verts; }
	//void SetIndices(vector<unsigned int> ind) { indices = ind; }
	void SetModel(XMMATRIX& model);
	void SetView(XMFLOAT4X4 view) { mvpData.view = view; }
	//void SetProjection(XMFLOAT4X4 projection) { mvpData.projection = projection; }
	void SetBoneOffsetData(vector<XMFLOAT4X4> data);
};