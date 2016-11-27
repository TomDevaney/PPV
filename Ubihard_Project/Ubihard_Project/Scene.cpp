#include "Scene.h"
#include "..\FBXLoader\FBXLoader.h"

void Scene::Init(DeviceResources const * devResources)
{
	//set previousTime to current time
	previousTime = time(nullptr);

	//set camera initial position
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));

	//set projection matrix
	float aspectRatio = CLIENT_WIDTH / CLIENT_HEIGHT;
	float fovAngleY = 70.0f * XM_PI / 180.0f;
	
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	XMMATRIX perspective = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);
	XMStoreFloat4x4(&projection, XMMatrixTranspose(perspective));

	//create all the device resources
	CreateDevResources(devResources);
}

void Scene::CreateDevResources(DeviceResources const * devResources)
{
	device = devResources->GetDevice();
	devContext = devResources->GetDeviceContext();

	//compile shaders
	Microsoft::WRL::ComPtr<ID3D10Blob> basicVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> basicPSBuffer;

	UINT flags = D3DCOMPILE_DEBUG;

	HRESULT vsCompResult = D3DCompileFromFile(L"VS_Basic.hlsl", NULL, NULL, "main", "vs_4_0", flags, NULL, basicVSBuffer.GetAddressOf(), NULL);
	HRESULT psCompResult = D3DCompileFromFile(L"PS_Basic.hlsl", NULL, NULL, "main", "ps_4_0", flags, NULL, basicPSBuffer.GetAddressOf(), NULL);

	//create shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> basicVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> basicPS;

	HRESULT vsCrtResult = device->CreateVertexShader(basicVSBuffer->GetBufferPointer(), basicVSBuffer->GetBufferSize(), NULL, basicVS.GetAddressOf());
	HRESULT psCrtResult = device->CreatePixelShader(basicPSBuffer->GetBufferPointer(), basicPSBuffer->GetBufferSize(), NULL, basicPS.GetAddressOf());

	vertexShaders.push_back(basicVS);
	pixelShaders.push_back(basicPS);

	//set up input layouts
	D3D11_INPUT_ELEMENT_DESC basicInputElementDescs[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT inputResult = device->CreateInputLayout(basicInputElementDescs, ARRAYSIZE(basicInputElementDescs), basicVSBuffer->GetBufferPointer(), basicVSBuffer->GetBufferSize(), inputLayout.GetAddressOf());

	//might need to make input layout more dynamic if shaders use a different vertex
	devContext->IASetInputLayout(inputLayout.Get());

	//set topology
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::CreateModels()
{
	//for each model, set their file path, texture path, shader, etc.
	//then make them create their device resources

	Model groundPlane;
	groundPlane.SetVertexShader(vertexShaders[Shadertypes::BASIC].Get());
	groundPlane.SetPixelShader(pixelShaders[Shadertypes::BASIC].Get());

	vector<Vertex> vertices =
	{
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f)}, //left bottom
		{ XMFLOAT3(0.5f, -0.5f,  0.5f)}, //right bottom
		{ XMFLOAT3(-0.5f,  0.5f, 0.5f)}, //left top
		{ XMFLOAT3(0.5f,  0.5f,  0.5f)} //right top
	};

	//clockwise
	vector<unsigned int> indices = 
	{
		2, 1, 0,
		2, 3, 1
	};

	groundPlane.SetVertices(vertices);
	groundPlane.SetIndices(indices);
	groundPlane.SetModel(XMMatrixIdentity());
	groundPlane.SetView(camera);
	groundPlane.SetProjection(projection);
	groundPlane.CreateDevResources(device, devContext);

	//models.push_back(groundPlane);


	//test model for fbx loading 
	Model testModel;
	testModel.SetVertexShader(vertexShaders[Shadertypes::BASIC].Get());
	testModel.SetPixelShader(pixelShaders[Shadertypes::BASIC].Get());
	testModel.SetTexturePath("../Assets/Box_Idle.fbm/TestCube.dds");
	vertices.clear();
	FBXLoader::Functions::FBXLoadFile(&vertices, "..\\Assets\\Box_Idle.fbx");
	testModel.SetVertices(vertices);
	testModel.SetModel(XMMatrixIdentity());
	testModel.SetView(camera);
	testModel.SetProjection(projection);
	testModel.CreateDevResources(device, devContext);
	models.push_back(testModel);


}

void Scene::Update(WPARAM wparam)
{
	//calculate delta time
	time_t currentTime = time(nullptr);
	float dt; //delta time
	//double dt2;

	//dt = (float)difftime(currentTime, previousTime);
	//dt2 = difftime(currentTime, previousTime);
	//previousTime = currentTime;

	dt = 1.0f / 60.0f;

	//check for input
	//CheckForInput(dt);

	//update anything that every model needs to know about (e.g., lights)
	

	//update camera (private function)
	UpdateCamera(dt, 5.0f, 0.75f, wparam);

	//update view on every object
	XMFLOAT4X4 tempCamera;

	XMStoreFloat4x4(&tempCamera, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));

	for (int i = 0; i < models.size(); ++i)
	{
		models[i].SetView(tempCamera);
	}

}

//void Scene::CheckForInput(float dt)
//{
//	IDirectInput
//}

void Scene::UpdateCamera(float dt, const float moveSpeed, const float rotSpeed, WPARAM wparam)
{
	switch (wparam) //switch on keyboard keys
	{
	case ('W'):
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpeed * dt);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
		break;
	}
	case ('S'):
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpeed * dt);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
		break;
	}
	case ('A'):
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpeed * dt, 0.0f, 0.0f);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
		break;
	}
	case ('D'):
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpeed * dt, 0.0f, 0.0f);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
		break;
	}
	}

	//check if right button is pressed
	if (wparam & MK_RBUTTON)
	{
		//I might need lparam to see if the curPosition != prevPosition
	}
}

void Scene::Render()
{
	//render all models
	for (size_t i = 0; i < models.size(); ++i)
	{
		models[i].Render(device, devContext);
	}
}