#pragma once
#include "ShaderStructures.h"
#include "Model.h"
#include "DeviceResources.h"

// in regards to shaders and what not, if we want multiple ones, we can have an enum and a member to choose which num from enum we want


class Scene
{
private:
	XMFLOAT4X4 camera;
	XMFLOAT4X4 projection;

	vector<Model> models;
	//vector of lights

	ID3D11Device* device;
	ID3D11DeviceContext* devContext;
	vector<Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixelShaders;
	vector<Microsoft::WRL::ComPtr<ID3D11VertexShader>> vertexShaders;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout; // is it safe to assume that all objects will have same input layout (same vertex)?

	//private helper functions
	void CreateDevResources(DeviceResources const * devResources);
public:
	void Init(DeviceResources const * devResources);
	void CreateModels();
	void Update();
	void Render();
};