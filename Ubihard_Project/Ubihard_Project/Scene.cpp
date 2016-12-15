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

	//get resources manager singleton
	resourceManager = ResourceManager::GetSingleton();

	//create all the device resources
	CreateDevResources(devResources);

	//create all lights
	CreateLights();

	//export all fbx data to binary
	DoFBXExporting();

	//create models in scene
	CreateModels();

	//load in models
	LoadModelsFromBinary();

	//temporary
	curFrame = 0;
}

void Scene::CreateDevResources(DeviceResources const * devResources)
{
	deviceResources = devResources;
	device = devResources->GetDevice();
	devContext = devResources->GetDeviceContext();

	//compile shaders
	Microsoft::WRL::ComPtr<ID3D10Blob> basicVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> basicPSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> normalPSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> depthPrePassVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> bindVSBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> basicCSBuffer;


	UINT flags = D3DCOMPILE_DEBUG;
	HRESULT result;
	result = D3DCompileFromFile(L"VS_Basic.hlsl", NULL, NULL, "main", "vs_4_0", flags, NULL, basicVSBuffer.GetAddressOf(), NULL);
	result = D3DCompileFromFile(L"PS_Basic.hlsl", NULL, NULL, "main", "ps_4_0", flags, NULL, basicPSBuffer.GetAddressOf(), NULL);
	result = D3DCompileFromFile(L"PS_NormalMapped.hlsl", NULL, NULL, "main", "ps_4_0", flags, NULL, normalPSBuffer.GetAddressOf(), NULL);
	result = D3DCompileFromFile(L"VS_Bind.hlsl", NULL, NULL, "main", "vs_4_0", flags, NULL, bindVSBuffer.GetAddressOf(), NULL);
	result = D3DCompileFromFile(L"VS_Basic.hlsl", NULL, NULL, "PreDepthPass", "vs_4_0", flags, NULL, depthPrePassVSBuffer.GetAddressOf(), NULL);
	result = D3DCompileFromFile(L"CS_Basic.hlsl", NULL, NULL, "main", "cs_4_0", flags, NULL, basicCSBuffer.GetAddressOf(), NULL);

	//create shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> basicVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> basicPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> normalPS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> bindVS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> depthPrePassVS;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> basicCS;

	result = device->CreateVertexShader(basicVSBuffer->GetBufferPointer(), basicVSBuffer->GetBufferSize(), NULL, basicVS.GetAddressOf());
	result = device->CreatePixelShader(basicPSBuffer->GetBufferPointer(), basicPSBuffer->GetBufferSize(), NULL, basicPS.GetAddressOf());
	result = device->CreatePixelShader(normalPSBuffer->GetBufferPointer(), normalPSBuffer->GetBufferSize(), NULL, normalPS.GetAddressOf());
	result = device->CreateVertexShader(bindVSBuffer->GetBufferPointer(), bindVSBuffer->GetBufferSize(), NULL, bindVS.GetAddressOf());
	result = device->CreateVertexShader(depthPrePassVSBuffer->GetBufferPointer(), depthPrePassVSBuffer->GetBufferSize(), NULL, depthPrePassVS.GetAddressOf());
	result = device->CreateComputeShader(basicCSBuffer->GetBufferPointer(), basicCSBuffer->GetBufferSize(), NULL, basicCS.GetAddressOf());

	vertexShaders.push_back(basicVS);
	vertexShaders.push_back(bindVS);
	vertexShaders.push_back(depthPrePassVS);
	pixelShaders.push_back(basicPS);
	pixelShaders.push_back(normalPS);

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
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
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

	//create camera constat buffer
	CD3D11_BUFFER_DESC cameraConstantBufferDesc(sizeof(CameraPositionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&cameraConstantBufferDesc, nullptr, &cameraConstantBuffer);
}

void Scene::CreateLights()
{
	//create only directional light
	dirLight.Create({ 0.577f, 0.577f, -0.577f, 0 }, { 0.75f, 0.75f, 0.94f, 1.0f }, { 0.6f, 0.6f, 0.6f, 0.6f });

	//create point lights
	PointLight pointLight0;
	pointLight0.Create({ 0, 5.0f, -2.0f, 0 }, { 1, 0, 0, 0 }, 8.0f);

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
	if (pointLights.size())
	{

		CD3D11_BUFFER_DESC pointLightConstantBufferDesc(sizeof(PointLightConstantBuffer) * (UINT)pointLights.size(), D3D11_BIND_CONSTANT_BUFFER);
		device->CreateBuffer(&pointLightConstantBufferDesc, nullptr, &pointLightConstantBuffer);

		devContext->UpdateSubresource(pointLightConstantBuffer.Get(), NULL, NULL, pointLights.data(), NULL, NULL);

		devContext->PSSetConstantBuffers(1, 1, pointLightConstantBuffer.GetAddressOf());
	}
	//Create spot light constant buffer
	if (NUMOFSPOTLIGHTS)
	{
		CD3D11_BUFFER_DESC spotLightConstantBufferDesc(sizeof(SpotLightConstantBuffer) * (UINT)spotLights.size(), D3D11_BIND_CONSTANT_BUFFER);
		device->CreateBuffer(&spotLightConstantBufferDesc, nullptr, &spotLightConstantBuffer);

		devContext->UpdateSubresource(spotLightConstantBuffer.Get(), NULL, NULL, spotLights.data(), NULL, NULL);

		devContext->PSSetConstantBuffers(2, 1, spotLightConstantBuffer.GetAddressOf());
	}
}

void Scene::DoFBXExporting()
{
#if 0
	// load in box animations and rig
	FBXLoader::Functions::FBXLoadExportFileBind("..\\Assets\\Box\\Box_Idle.fbx", "Box", "Box_Idle");
	FBXLoader::Functions::FBXLoadExportAnimation("..\\Assets\\Box\\Box_Attack.fbx", "Box", "Box_Attack");

	////load in teddy animation and rig
	FBXLoader::Functions::FBXLoadExportFileBind("..\\Assets\\Teddy\\Teddy_Idle.fbx", "Teddy", "Teddy_Idle");
	FBXLoader::Functions::FBXLoadExportAnimation("..\\Assets\\Teddy\\Teddy_Attack1.fbx", "Teddy", "Teddy_Attack1");
	FBXLoader::Functions::FBXLoadExportAnimation("..\\Assets\\Teddy\\Teddy_Attack2.fbx", "Teddy", "Teddy_Attack2");
	FBXLoader::Functions::FBXLoadExportAnimation("..\\Assets\\Teddy\\Teddy_Run.fbx", "Teddy", "Teddy_Run");

	////load in sphere
	FBXLoader::Functions::FBXLoadExportFileBasic("..\\Assets\\Sphere.fbx", "Sphere");

	////load in mage with rig and animation
#endif
	FBXLoader::Functions::FBXLoadExportFileBind("..\\Assets\\Mage\\Idle.fbx", "Mage", "Mage_Idle");
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
	groundPlane.SetBasicVerts(basicVertices);
	groundPlane.SetIndices(indices);

	groundPlane.Init(VertexShaderTypes::vsBASIC, vertexShaders[VertexShaderTypes::vsBASIC].Get(), vertexShaders[VertexShaderTypes::vsDEPTHPREPASS].Get(), pixelShaders[PixelShaderTypes::psBASIC].Get(), inputLayouts[VertexShaderTypes::vsBASIC].Get(), "../Assets/Textures/DDS/FloorTexture.dds", XMMatrixIdentity(), camera, projection, L"");
	groundPlane.CreateDevResources(deviceResources);
	models.push_back(groundPlane);

	//test model for fbx loading 
	Model testModel;
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	XMFLOAT4X4 identities[4] = { identity, identity, identity, identity };

	testModel.Init(VertexShaderTypes::vsBIND, vertexShaders[VertexShaderTypes::vsBIND].Get(), pixelShaders[PixelShaderTypes::psNORMALMAPPED].Get(), inputLayouts[VertexShaderTypes::vsBIND].Get(), "../Assets/Textures/DDS/TestCube.dds", "", XMMatrixIdentity(), camera, projection, identities, L"Box");
	testModel.CreateDevResources(deviceResources);
	models.push_back(testModel);

	//add bear
	Model monokuma;
	monokuma.Init(VertexShaderTypes::vsBIND, vertexShaders[VertexShaderTypes::vsBIND].Get(), pixelShaders[PixelShaderTypes::psNORMALMAPPED].Get(), inputLayouts[VertexShaderTypes::vsBIND].Get(), "../Assets/Textures/DDS/Teddy.dds", "", XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), XMMatrixTranslation(-3, 0, 3))), camera, projection, identities, L"Teddy");
	monokuma.CreateDevResources(deviceResources);

	models.push_back(monokuma);

	//add test box but with attack animation
	Model testModelAttack;
	testModelAttack.Init(VertexShaderTypes::vsBIND, vertexShaders[VertexShaderTypes::vsBIND].Get(), pixelShaders[PixelShaderTypes::psNORMALMAPPED].Get(), inputLayouts[VertexShaderTypes::vsBIND].Get(), "../Assets/Textures/DDS/TestCube.dds", "", XMMatrixTranspose(XMMatrixTranslation(3, 0, 0)), camera, projection, identities, L"Box");
	testModelAttack.CreateDevResources(deviceResources);
	models.push_back(testModelAttack);


	//add magician
	Model mage;

	mage.Init(VertexShaderTypes::vsBIND, vertexShaders[VertexShaderTypes::vsBIND].Get(), pixelShaders[PixelShaderTypes::psNORMALMAPPED].Get(), inputLayouts[VertexShaderTypes::vsBIND].Get(), "../Assets/Textures/DDS/Mage.dds", "../Assets/Textures/DDS/Mage_NM.dds", XMMatrixTranspose(XMMatrixTranslation(-3, 0, -3)), camera, projection, identities, L"Mage");
	mage.SetSpecMap("../Assets/Textures/DDS/Mage_Spec.dds");
	mage.CreateDevResources(deviceResources);

	models.push_back(mage);

	//add four spheres. set postions at position in boneMats
	for (int i = 0; i < 4; ++i)
	{
		Model sphereModel;

		sphereModel.Init(VertexShaderTypes::vsBASIC, vertexShaders[VertexShaderTypes::vsBASIC].Get(), vertexShaders[VertexShaderTypes::vsDEPTHPREPASS].Get(), pixelShaders[PixelShaderTypes::psBASIC].Get(), inputLayouts[VertexShaderTypes::vsBASIC].Get(), "", XMMatrixIdentity(), camera, projection, L"Sphere");
		sphereModel.CreateDevResources(deviceResources);
		models.push_back(sphereModel);
	}
}

void Scene::LoadModelsFromBinary()
{
	resourceManager->LoadInAnimationSet();

	//make a function for this later
	//but make the animated render node and animated game objects
	//probably should only be making animated render node, but I don't understand how to do such a thing
	AnimatedRenderNode* boxRenderNode = new AnimatedRenderNode();
	AnimatedGameObject* boxGameObject = new AnimatedGameObject();

	boxGameObject->Init("Box", 1, -1, false);
	boxGameObject->SetRenderNode(boxRenderNode);

	renderNodes.push_back(boxRenderNode);
	gameObjects.push_back(boxGameObject);

	AnimatedRenderNode* teddyRenderNode = new AnimatedRenderNode();
	AnimatedGameObject* teddyGameObject = new AnimatedGameObject();

	teddyGameObject->Init("Teddy", 3, -1, true);
	teddyGameObject->SetRenderNode(teddyRenderNode);

	renderNodes.push_back(teddyRenderNode);
	gameObjects.push_back(teddyGameObject);

	AnimatedRenderNode* boxAttackRenderNode = new AnimatedRenderNode();
	AnimatedGameObject* boxAttackGameObject = new AnimatedGameObject();

	boxAttackGameObject->Init("Box", 0, -1, true);
	boxAttackGameObject->SetRenderNode(boxAttackRenderNode);

	renderNodes.push_back(boxAttackRenderNode);
	gameObjects.push_back(boxAttackGameObject);
	
	AnimatedRenderNode* mageRenderNode = new AnimatedRenderNode();
	AnimatedGameObject* mageGameObject = new AnimatedGameObject();

	mageGameObject->Init("Mage", 0, -1, false);
	mageGameObject->SetRenderNode(mageRenderNode);

	renderNodes.push_back(mageRenderNode);
	gameObjects.push_back(mageGameObject);
}

void Scene::Update(WPARAM wparam)
{
	//delta time
	float dt;
	dt = 1.0f / 60.0f;

	//handle input
	HandleInput();

	//update lights
	if (pointLights.size())
	{

		//TODO: pointLights[0].DoRadiusEffect(5.0f, radiusChange[0]);
		pointLights[1].DoRadiusEffect(7.0f, radiusChange[1]);

		devContext->UpdateSubresource(pointLightConstantBuffer.Get(), NULL, NULL, pointLights.data(), NULL, NULL);

		devContext->PSSetConstantBuffers(1, 1, pointLightConstantBuffer.GetAddressOf());
	}

	//update camera (private function)
	UpdateCamera(dt, 5.0f, 0.75f, wparam);

	//update camera constant buffer
	cameraBufferData.cameraposW = XMFLOAT4(camera._41, camera._42, camera._43, 1);
	devContext->UpdateSubresource(cameraConstantBuffer.Get(), NULL, NULL, &cameraBufferData, NULL, NULL);
	devContext->PSSetConstantBuffers(2, 1, cameraConstantBuffer.GetAddressOf());


	//update view on every object
	XMFLOAT4X4 tempCamera;

	XMStoreFloat4x4(&tempCamera, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));

	for (int i = 0; i < models.size(); ++i)
	{
		models[i].SetView(tempCamera);
	}


	//this is for models where I want to click to move them
	gameObjects[0]->SetCurFrame(curFrame);
	gameObjects[3]->SetCurFrame(curFrame);

	//update inverse bind poses in game objects
	gameObjects[0]->Update(0); // box will move from key pres
	gameObjects[1]->Update(dt / 2); // bear will move based on time
	gameObjects[2]->Update(dt / 2); //box attack will move time based
	gameObjects[3]->Update(0);

	//update model to take in bone offset data from render node
	for (int i = 1; i < renderNodes.size() + 1; ++i) // plus one because plane is first
	{
		vector<XMFLOAT4X4> boneOffsets = renderNodes[i - 1]->GetBoneOffsets();

		models[i].SetBoneOffsetData(renderNodes[i - 1]->GetBoneOffsets());
	}

	//update spheres to new bone positions
	vector<XMFLOAT4X4> bonesWorlds = renderNodes[0]->GetBonesWorlds();

	for (int i = 0; i < bonesWorlds.size(); ++i)
	{
		models[i + models.size() - 4].SetModel(XMMatrixTranspose(XMMatrixTranslation(bonesWorlds[i]._41, bonesWorlds[i]._42, bonesWorlds[i]._43)));
	}
}

void Scene::HandleInput()
{
	if (buttons['O'] & 0x01)
	{
		++curFrame;
	}

	if (buttons['P'])
	{
		--curFrame;
	}

	if (buttons['K']) // go to idle
	{
		BlendInfo info;
		info.totalBlendTime = 2.0f;

		gameObjects[1]->SetBlendInfo(info);
		gameObjects[1]->SetNextAnimation(2);
		//gameObjects[1]->CreateNextAnimation(true);
	}

	if (buttons['J']) //go to run
	{
		BlendInfo info;
		info.totalBlendTime = 2.0f;

		gameObjects[1]->SetNextAnimation(3);
		gameObjects[1]->SetBlendInfo(info);
		//gameObjects[1]->CreateNextAnimation(true);
	}

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
		//if (i == 0) //to not render the idle test box
		{
			models[i].Render();
		}
	}
}

void Scene::Shutdown()
{
	//clean up render nodes and game objects b/c I dynamically allocated them
	for (int i = 0; i < renderNodes.size(); ++i)
	{
		delete renderNodes[i];
	}

	for (int i = 0; i < gameObjects.size(); ++i)
	{
		delete gameObjects[i];
	}

	//clean up singleton resource manager
	resourceManager->CleanUp();
}