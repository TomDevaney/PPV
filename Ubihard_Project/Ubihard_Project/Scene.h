#pragma once
#include "../ShaderStructures/ShaderStructures.h"
#include "Model.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "DeviceResources.h"
#include "ResourceManager.h"
#include "AnimatedRenderNode.h"
#include "AnimatedGameObject.h"

#define NUMOFPOINTLIGHTS 1
#define NUMOFSPOTLIGHTS 0
// in regards to shaders and what not, if we want multiple ones, we can have an enum and a member to choose which num from enum we want


class Scene
{
private:
	//buffer data
	DirectX::XMFLOAT4X4 camera;
	DirectX::XMFLOAT4X4 projection;

	float camYaw;
	float camPitch;

	//input data
	bool buttons[256];
	bool leftClick;
	bool rightClick;
	int prevMouseX, prevMouseY;
	int mouseX, mouseY;

	time_t previousTime; //deltaTime

	vector<Model> models;
	DirectionalLight dirLight;
	vector<PointLight> pointLights;
	vector<SpotLight> spotLights;

	ResourceManager* resourceManager;
	vector<AnimatedRenderNode> renderNodes;
	vector<AnimatedGameObject> gameObjects;

	float radiusChange[2] = { 1.0f / 60.0f, 1.0f / 60.0f };

	//Device Resources
	const DeviceResources* deviceResources;
	ID3D11Device* device;
	ID3D11DeviceContext* devContext;
	vector<Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixelShaders;
	vector<Microsoft::WRL::ComPtr<ID3D11VertexShader>> vertexShaders;
	vector<Microsoft::WRL::ComPtr<ID3D11ComputeShader>> computerShaders;
	vector<Microsoft::WRL::ComPtr<ID3D11InputLayout>> inputLayouts; // is it safe to assume that all objects will have same input layout (same vertex)?
	Microsoft::WRL::ComPtr<ID3D11SamplerState> wrapSamplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> dirLightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pointLightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> spotLightConstantBuffer;

	//private helper functions
	void CreateDevResources(DeviceResources const * devResources);
	void CreateLights();
	void UpdateCamera(float dt, const float moveSpeed, const float rotateSpeed, WPARAM wparam);
	void CreateModels();
	void LoadModelsFromBinary();

public:
	void Init(DeviceResources const * devResources);
	void Update(WPARAM wparam);
	void Render();

	//setters
	void SetButtons(bool butts[256]) { memcpy(buttons, butts, sizeof(buttons)); }
	void SetLeftClick(bool clicked) { leftClick = clicked; }
	void SetRightClick(bool clicked) { rightClick = clicked; }
	void SetMousePosition(int x, int y) { mouseX = x; mouseY = y; }
};