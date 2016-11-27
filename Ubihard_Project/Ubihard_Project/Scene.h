#pragma once
#include "ShaderStructures.h"
#include "Model.h"
#include "DeviceResources.h"

// in regards to shaders and what not, if we want multiple ones, we can have an enum and a member to choose which num from enum we want


class Scene
{
private:
	//buffer data
	XMFLOAT4X4 camera;
	XMFLOAT4X4 projection;

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
	//vector of lights

	ID3D11Device* device;
	ID3D11DeviceContext* devContext;
	vector<Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixelShaders;
	vector<Microsoft::WRL::ComPtr<ID3D11VertexShader>> vertexShaders;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout; // is it safe to assume that all objects will have same input layout (same vertex)?

	//private helper functions
	void CreateDevResources(DeviceResources const * devResources);
	void UpdateCamera(float dt, const float moveSpeed, const float rotateSpeed, WPARAM wparam);
	//void CheckForInput(float dt);
public:
	void Init(DeviceResources const * devResources);
	void CreateModels();
	void Update(WPARAM wparam);
	void Render();
	
	//setters
	void SetButtons(bool butts[256]) { memcpy(buttons, butts, sizeof(buttons)); }
	void SetLeftClick(bool clicked) { leftClick = clicked; }
	void SetRightClick(bool clicked) { rightClick = clicked; }
	void SetMousePosition(int x, int y) { mouseX = x; mouseY = y; }
};