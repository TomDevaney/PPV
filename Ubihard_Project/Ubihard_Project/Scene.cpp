#include "Scene.h"
#include "../Bin/FBXLoader/FBXLoader.h"

void Scene::Init(DeviceResources const * devResources)
{
	//set previousTime to current time
	previousTime = time(nullptr);

	//set buttons to zero
	memset(buttons, 0, sizeof(buttons));

	//set camera initial position
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));

	camPitch = camYaw = 0.0f;

	//set projection matrix
	float aspectRatio = (float)CLIENT_WIDTH / (float)CLIENT_HEIGHT;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	XMMATRIX perspective = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);
	XMStoreFloat4x4(&projection, XMMatrixTranspose(perspective));

	//create all the device resources
	CreateDevResources(devResources);

	//create all lights
	CreateLights();

	//create models in scene
	CreateModels();

	//load in models
	LoadModelsFromBinary();
}

void Scene::CreateDevResources(DeviceResources const * devResources)
{
	deviceResources = devResources;
	device = devResources->GetDevice();
	devContext = devResources->GetDeviceContext();

	//compile shaders
	Microsoft::WRL::ComPtr<ID3D10Blob> basicVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> basicPSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> depthPrePassVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> bindVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> basicCSBuffer;


	UINT flags = D3DCOMPILE_DEBUG;

	HRESULT vsCompResult = D3DCompileFromFile(L"VS_Basic.hlsl", NULL, NULL, "main", "vs_4_0", flags, NULL, basicVSBuffer.GetAddressOf(), NULL);
	HRESULT psCompResult = D3DCompileFromFile(L"PS_Basic.hlsl", NULL, NULL, "main", "ps_4_0", flags, NULL, basicPSBuffer.GetAddressOf(), NULL);
	HRESULT vsBindCompResult = D3DCompileFromFile(L"VS_Bind.hlsl", NULL, NULL, "main", "vs_4_0", flags, NULL, bindVSBuffer.GetAddressOf(), NULL);
	HRESULT vsDepthPrePassCompResult = D3DCompileFromFile(L"VS_Basic.hlsl", NULL, NULL, "PreDepthPass", "vs_4_0", flags, NULL, depthPrePassVSBuffer.GetAddressOf(), NULL);
	HRESULT csCompResult = D3DCompileFromFile(L"CS_Basic.hlsl", NULL, NULL, "main", "cs_4_0", flags, NULL, basicCSBuffer.GetAddressOf(), NULL);

	//create shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> basicVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> basicPS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> bindVS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> depthPrePassVS;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> basicCS;

	HRESULT vsCrtResult = device->CreateVertexShader(basicVSBuffer->GetBufferPointer(), basicVSBuffer->GetBufferSize(), NULL, basicVS.GetAddressOf());
	HRESULT psCrtResult = device->CreatePixelShader(basicPSBuffer->GetBufferPointer(), basicPSBuffer->GetBufferSize(), NULL, basicPS.GetAddressOf());
	HRESULT vsBindCrtResult = device->CreateVertexShader(bindVSBuffer->GetBufferPointer(), bindVSBuffer->GetBufferSize(), NULL, bindVS.GetAddressOf());
	HRESULT vsDepthPrePassCrtResult = device->CreateVertexShader(depthPrePassVSBuffer->GetBufferPointer(), depthPrePassVSBuffer->GetBufferSize(), NULL, depthPrePassVS.GetAddressOf());
	HRESULT csCrtResult = device->CreateComputeShader(basicCSBuffer->GetBufferPointer(), basicCSBuffer->GetBufferSize(), NULL, basicCS.GetAddressOf());

	vertexShaders.push_back(basicVS);
	vertexShaders.push_back(bindVS);
	vertexShaders.push_back(depthPrePassVS);
	pixelShaders.push_back(basicPS);

	//set up input layouts
	Microsoft::WRL::ComPtr<ID3D11InputLayout> basicInput;

	D3D11_INPUT_ELEMENT_DESC basicInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT inputResult = device->CreateInputLayout(basicInputElementDescs, ARRAYSIZE(basicInputElementDescs), basicVSBuffer->GetBufferPointer(), basicVSBuffer->GetBufferSize(), basicInput.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11InputLayout> bindInput;

	D3D11_INPUT_ELEMENT_DESC bindInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT bindInputResult = device->CreateInputLayout(bindInputElementDescs, ARRAYSIZE(bindInputElementDescs), bindVSBuffer->GetBufferPointer(), bindVSBuffer->GetBufferSize(), bindInput.GetAddressOf());

	inputLayouts.push_back(basicInput);
	inputLayouts.push_back(bindInput);

	//set topology
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//create sampler state
	CD3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // do D3D11_FILTER_ANISOTROPIC for better quality
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 1; //16 for anisotropic
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;

	HRESULT wrapSampleResult = device->CreateSamplerState(&samplerDesc, &wrapSamplerState);

	devContext->PSSetSamplers(0, 1, wrapSamplerState.GetAddressOf());

	//create lighting buffers and set them
}

void Scene::CreateLights()
{
	//create only directional light
	dirLight.Create({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.3f, 0.3f, 0.3f, 0.3f });

	//create point lights
	PointLight pointLight0;
	pointLight0.Create({ 0, 1.0f, 2.0f, 0 }, { 1, 0, 0, 0 }, 5.0f);

	PointLight pointLight1;
	pointLight1.Create({ 0, 1.0f, 2.0f, 0 }, { 0, 1.0f, 0, 0 }, 7.0f);

	pointLights.push_back(pointLight0);
	pointLights.push_back(pointLight1);

	//create spot lights
	//Light spotLight;

	//spotLight.CreateSpotlight({ 2, 3.0f, 2, 0 }, { 1, 0, 0, 0 }, 5.0f);

	//spotLights.push_back(spotLight);

	//create directional light constant buffer
	CD3D11_BUFFER_DESC dirLightConstantBufferDesc(sizeof(DirectionalLightConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&dirLightConstantBufferDesc, nullptr, &dirLightConstantBuffer);

	devContext->UpdateSubresource(dirLightConstantBuffer.Get(), NULL, NULL, &dirLight, NULL, NULL);

	devContext->PSSetConstantBuffers(0, 1, dirLightConstantBuffer.GetAddressOf());

	//create point light constant buffer
	CD3D11_BUFFER_DESC pointLightConstantBufferDesc(sizeof(PointLightConstantBuffer) * (UINT)pointLights.size(), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&pointLightConstantBufferDesc, nullptr, &pointLightConstantBuffer);

	devContext->UpdateSubresource(pointLightConstantBuffer.Get(), NULL, NULL, pointLights.data(), NULL, NULL);

	devContext->PSSetConstantBuffers(1, 1, pointLightConstantBuffer.GetAddressOf());

	//Create spot light constant buffer
	if (NUMOFSPOTLIGHTS)
	{
		CD3D11_BUFFER_DESC spotLightConstantBufferDesc(sizeof(SpotLightConstantBuffer) * (UINT)spotLights.size(), D3D11_BIND_CONSTANT_BUFFER);
		device->CreateBuffer(&spotLightConstantBufferDesc, nullptr, &spotLightConstantBuffer);

		devContext->UpdateSubresource(spotLightConstantBuffer.Get(), NULL, NULL, spotLights.data(), NULL, NULL);

		devContext->PSSetConstantBuffers(2, 1, spotLightConstantBuffer.GetAddressOf());
	}
}

void Scene::CreateModels()
{
	//ground plane
	Model groundPlane;

	vector<VS_BasicInput> basicVertices =
	{
		{ XMFLOAT3(-5.5f, 0, -5.5f), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(0.0f, 1.0f) }, //left bottom
		{ XMFLOAT3(5.5f, 0, -5.5f), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(1.0f, 1.0f) }, //right bottom
		{ XMFLOAT3(-5.5f,  0, 5.5f), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(0.0f, 0.0f) }, //left top
		{ XMFLOAT3(5.5f,  0,  5.5f), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(1.0f, 0.0f) } //right top
	};

	//clockwise
	vector<unsigned int> indices =
	{
		2, 1, 0,
		2, 3, 1
	};

	groundPlane.Init(Shadertypes::BASIC, vertexShaders[Shadertypes::BASIC].Get(), vertexShaders[Shadertypes::DEPTHPREPASS].Get(), pixelShaders[Shadertypes::BASIC].Get(), inputLayouts[Shadertypes::BASIC].Get(), basicVertices, indices, "../Resources/FloorTexture.dds", XMMatrixIdentity(), camera, projection);
	groundPlane.CreateDevResources(deviceResources);
	models.push_back(groundPlane);

	//test model for fbx loading 
	Model testModel;
	vector<Vertex> bindVertices;
	vector<XMFLOAT4X4> boneMatrices;
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	XMFLOAT4X4 identities[4] = { identity, identity, identity, identity };

	FBXLoader::Functions::FBXLoadFile(&bindVertices, &indices, &boneMatrices, "..\\Assets\\Box_Idle.fbx");
	bindVertices.clear();
	indices.clear();
	FBXLoader::Functions::FBXExportToBinary(&bindVertices, &indices, "..\\Assets\\Box_Idle.fbx", "..\\Assets\\Box_Idle.fbx");
	testModel.Init(Shadertypes::BIND, vertexShaders[Shadertypes::BIND].Get(), pixelShaders[Shadertypes::BASIC].Get(), inputLayouts[Shadertypes::BIND].Get(), bindVertices, indices, "../Resources/TestCube.dds", XMMatrixIdentity(), camera, projection, identities);
	testModel.CreateDevResources(deviceResources);
	//models.push_back(testModel);

	basicVertices.clear();
	bindVertices.clear();
	
	//add four spheres. set postions at position in boneMats
	FBXLoader::Functions::FBXLoadFile(&bindVertices, &indices, nullptr, "..\\Assets\\Sphere.fbx");

	for (int i = 0; i < 4; ++i)
	{
		Model sphereModel;

		sphereModel.Init(Shadertypes::BIND, vertexShaders[Shadertypes::BIND].Get(), pixelShaders[Shadertypes::BASIC].Get(), inputLayouts[Shadertypes::BIND].Get(), bindVertices, indices, "", XMMatrixTranspose(XMMatrixTranslation(boneMatrices[i]._41, boneMatrices[i]._42, boneMatrices[i]._43)), camera, projection, identities);
		sphereModel.CreateDevResources(deviceResources);
		//models.push_back(sphereModel);
	}

	//add magician
	//Model mage;

	//bindVertices.clear();

	//FBXLoader::Functions::FBXLoadFile(&bindVertices, &indices, &boneMatrices, "..\\Assets\\Battle Mage with Rig and textures.fbx");
	//
	//bindVertices.clear();
	//indices.clear();
	//FBXLoader::Functions::FBXExportToBinary(&bindVertices, &indices, "..\\Assets\\Battle Mage with Rig and textures.fbx", "..\\Assets\\Battle Mage with Rig and textures.fbx");

	//mage.Init(Shadertypes::BIND, vertexShaders[Shadertypes::BIND].Get(), pixelShaders[Shadertypes::BASIC].Get(), inputLayouts[Shadertypes::BIND].Get(), bindVertices, indices, "", XMMatrixTranspose(XMMatrixTranslation(-3, 0, 3)), camera, projection, identities);
	//mage.CreateDevResources(deviceResources);
}

void Scene::LoadModelsFromBinary()
{
	resourceManager.LoadInSkeleton();
	resourceManager.LoadInAnimation();
}

void Scene::Update(WPARAM wparam)
{
	//delta time
	float dt; 
	dt = 1.0f / 60.0f;

	//update lights
	pointLights[0].DoRadiusEffect(5.0f, radiusChange[0]);
	pointLights[1].DoRadiusEffect(7.0f, radiusChange[1]);

	devContext->UpdateSubresource(pointLightConstantBuffer.Get(), NULL, NULL, pointLights.data(), NULL, NULL);

	devContext->PSSetConstantBuffers(1, 1, pointLightConstantBuffer.GetAddressOf());

	//update camera (private function)
	UpdateCamera(dt, 5.0f, 0.75f, wparam);

	//update view on every object
	XMFLOAT4X4 tempCamera;

	XMStoreFloat4x4(&tempCamera, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));

	for (int i = 0; i < models.size(); ++i)
	{
		models[i].SetView(tempCamera);
	}

	//TODO: we need to update bone offsets somehow by calculating: vertexOut = inverseBindMatrix  * currentWorldMatrix * bindVertexPosition
}

void Scene::UpdateCamera(float dt, const float moveSpeed, const float rotateSpeed, WPARAM wparam)
{
	if (buttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpeed * dt);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
	}

	if (buttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpeed * dt);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
	}

	if (buttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpeed * dt, 0.0f, 0.0f);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
	}

	if (buttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpeed * dt, 0.0f, 0.0f);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
	}

	if (buttons['Q']) //up
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpeed * dt, 0.0f);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
	}

	if (buttons['E']) //down
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpeed * dt, 0.0f);
		XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
		XMMATRIX newCamera = XMMatrixMultiply(translation, tempCamera);
		XMStoreFloat4x4(&camera, newCamera);
	}

	if (mouseX && mouseY)
	{
		if (rightClick && prevMouseX && prevMouseY)
		{
			float dx = (float)mouseX - (float)prevMouseX;
			float dy = (float)mouseY - (float)prevMouseY;

			//store old cam position
			XMFLOAT3 camPosition = XMFLOAT3(camera._41, camera._42, camera._43);

			camera._41 = 0;
			camera._42 = 0;
			camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotateSpeed * dt);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotateSpeed * dt);

			//apply rotations to camera
			XMMATRIX tempCamera = XMLoadFloat4x4(&camera);
			tempCamera = XMMatrixMultiply(rotX, tempCamera);
			tempCamera = XMMatrixMultiply(tempCamera, rotY);

			//store new camera
			XMStoreFloat4x4(&camera, tempCamera);

			//change position to where it was earlier
			camera._41 = camPosition.x;
			camera._42 = camPosition.y;
			camera._43 = camPosition.z;
		}

		prevMouseX = mouseX;
		prevMouseY = mouseY;
	}


}

void Scene::Render()
{
	//render all models
	for (size_t i = 0; i < models.size(); ++i)
	{
		models[i].Render();
	}
}