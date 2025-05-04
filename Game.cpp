#include "Graphics.h"
#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include <math.h>
#include "BufferStructs.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	ImGui::StyleColorsDark();

	//create camera
	float aspectRatio = Window::AspectRatio();
	XMFLOAT3 persStartPos = { 6.0f, 1.0f, -12.0f };
	XMFLOAT3 orthoStartPos = { 0.0f, 0.0f, -2.0f };
	cameras.push_back(std::make_shared<Camera>(aspectRatio, persStartPos, XM_PIDIV4, true)); // perspective cam 1
	cameras.push_back(std::make_shared<Camera>(aspectRatio, orthoStartPos, XM_PIDIV4 + 10, true)); // perspective cam 2
	activeCameraIndex = 0;

	//shadow map setup
	shadowOptions.ShadowMapResolution = 1024;
	shadowOptions.ShadowProjectionSize = 10.0f;
	CreateShadowMapResources();

	//post processing 
	//box blur
	blurRad = 0;

	//fog
	fogType = 1;
	fogColor = XMFLOAT3(0.75f, 0.65f, 0.7f);
	fogStartDist  = 25.0f;
	fogEndDist  = 50.0f;
	fogDensity = 0.05f;
	heightBasedFog = 0;
	fogHeight = 5.0f;
	fogVerticalDensity = 0.05f;
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

//helper methods for creating geometry, materials, textures, shaders...
void LoadMaterialTextures(const std::wstring& baseName,
	ID3D11ShaderResourceView** albedo,
	ID3D11ShaderResourceView** normal,
	ID3D11ShaderResourceView** roughness,
	ID3D11ShaderResourceView** metal) {
	auto device = Graphics::Device.Get();
	auto context = Graphics::Context.Get();

	CreateWICTextureFromFile(device, context, FixPath(L"../../Assets/Textures/" + baseName + L"_albedo.png").c_str(), 0, albedo);
	CreateWICTextureFromFile(device, context, FixPath(L"../../Assets/Textures/" + baseName + L"_normals.png").c_str(), 0, normal);
	CreateWICTextureFromFile(device, context, FixPath(L"../../Assets/Textures/" + baseName + L"_roughness.png").c_str(), 0, roughness);
	CreateWICTextureFromFile(device, context, FixPath(L"../../Assets/Textures/" + baseName + L"_metal.png").c_str(), 0, metal);
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//loading textures
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler; 
	//create a sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&sampDesc, sampler.GetAddressOf());

	//actually load a texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleA, cobbleN, cobbleR, cobbleM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorA, floorN, floorR, floorM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintA, paintN, paintR, paintM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedA, scratchedN, scratchedR, scratchedM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeA, bronzeN, bronzeR, bronzeM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughA, roughN, roughR, roughM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodA, woodN, woodR, woodM;

	LoadMaterialTextures(L"cobblestone", cobbleA.GetAddressOf(), cobbleN.GetAddressOf(), cobbleR.GetAddressOf(), cobbleM.GetAddressOf());
	LoadMaterialTextures(L"floor", floorA.GetAddressOf(), floorN.GetAddressOf(), floorR.GetAddressOf(), floorM.GetAddressOf());
	LoadMaterialTextures(L"paint", paintA.GetAddressOf(), paintN.GetAddressOf(), paintR.GetAddressOf(), paintM.GetAddressOf());
	LoadMaterialTextures(L"scratched", scratchedA.GetAddressOf(), scratchedN.GetAddressOf(), scratchedR.GetAddressOf(), scratchedM.GetAddressOf());
	LoadMaterialTextures(L"bronze", bronzeA.GetAddressOf(), bronzeN.GetAddressOf(), bronzeR.GetAddressOf(), bronzeM.GetAddressOf());
	LoadMaterialTextures(L"rough", roughA.GetAddressOf(), roughN.GetAddressOf(), roughR.GetAddressOf(), roughM.GetAddressOf());
	LoadMaterialTextures(L"wood", woodA.GetAddressOf(), woodN.GetAddressOf(), woodR.GetAddressOf(), woodM.GetAddressOf());

	//creating shaders
	shadowVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"ShadowMapVS.cso").c_str());
	std::shared_ptr<SimpleVertexShader> vertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> uvShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> customShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> multiplyShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"MultiplyPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelPBRShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelLightingShader.cso").c_str());
	std::shared_ptr<SimpleVertexShader> skyVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"SkyVS.cso").c_str());
	std::shared_ptr<SimplePixelShader> skyPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"SkyPS.cso").c_str());


	//loading models
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>("sphere0", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("sphere0", FixPath(L"../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("sphere0", FixPath(L"../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("sphere0", FixPath(L"../../Assets/Models/cylinder.obj").c_str());
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("cube", FixPath(L"../../Assets/Models/cube.obj").c_str());

	//updating mesh vector
	meshes.insert(meshes.end(), { sphereMesh, cubeMesh, helixMesh, torusMesh, cylinderMesh });

	sky = std::make_shared<Sky>(
		FixPath(L"../../Assets/Textures/Skies/Clouds Pink/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Clouds Pink/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Clouds Pink/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Clouds Pink/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Clouds Pink/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Clouds Pink/back.png").c_str(),
		cubeMesh,
		skyVS,
		skyPS,
		sampler);

	//creating materials
	std::shared_ptr<Material> matUV = std::make_shared<Material>(uvShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "UV Preview", XMFLOAT2(1, 1));
	std::shared_ptr<Material> matNorm = std::make_shared<Material>(normalShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Normal Preview", XMFLOAT2(1, 1));
	std::shared_ptr<Material> matCustom = std::make_shared<Material>(customShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Custom Colorshift", XMFLOAT2(1, 1));
	
	//mat that uses lighting
	std::shared_ptr<Material> cobbleMat4x = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Cobblestone (4x Scale)", XMFLOAT2(4, 4));
	cobbleMat4x->AddSampler("BasicSampler", sampler);
	cobbleMat4x->AddTextureSRV("Albedo", cobbleA);
	cobbleMat4x->AddTextureSRV("NormalMap", cobbleN);
	cobbleMat4x->AddTextureSRV("RoughnessMap", cobbleR);
	cobbleMat4x->AddTextureSRV("MetalnessMap", cobbleM);

	std::shared_ptr<Material> floorMat = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Metal Floor", XMFLOAT2(2, 2));
	floorMat->AddSampler("BasicSampler", sampler);
	floorMat->AddTextureSRV("Albedo", floorA);
	floorMat->AddTextureSRV("NormalMap", floorN);
	floorMat->AddTextureSRV("RoughnessMap", floorR);
	floorMat->AddTextureSRV("MetalnessMap", floorM);

	std::shared_ptr<Material> paintMat = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Blue Paint", XMFLOAT2(2, 2));
	paintMat->AddSampler("BasicSampler", sampler);
	paintMat->AddTextureSRV("Albedo", paintA);
	paintMat->AddTextureSRV("NormalMap", paintN);
	paintMat->AddTextureSRV("RoughnessMap", paintR);
	paintMat->AddTextureSRV("MetalnessMap", paintM);

	std::shared_ptr<Material> scratchedMat = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Scratched Paint", XMFLOAT2(2, 2));
	scratchedMat->AddSampler("BasicSampler", sampler);
	scratchedMat->AddTextureSRV("Albedo", scratchedA);
	scratchedMat->AddTextureSRV("NormalMap", scratchedN);
	scratchedMat->AddTextureSRV("RoughnessMap", scratchedR);
	scratchedMat->AddTextureSRV("MetalnessMap", scratchedM);

	std::shared_ptr<Material> bronzeMat = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Bronze", XMFLOAT2(2, 2));
	bronzeMat->AddSampler("BasicSampler", sampler);
	bronzeMat->AddTextureSRV("Albedo", bronzeA);
	bronzeMat->AddTextureSRV("NormalMap", bronzeN);
	bronzeMat->AddTextureSRV("RoughnessMap", bronzeR);
	bronzeMat->AddTextureSRV("MetalnessMap", bronzeM);

	std::shared_ptr<Material> roughMat = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Rough Metal", XMFLOAT2(2, 2));
	roughMat->AddSampler("BasicSampler", sampler);
	roughMat->AddTextureSRV("Albedo", roughA);
	roughMat->AddTextureSRV("NormalMap", roughN);
	roughMat->AddTextureSRV("RoughnessMap", roughR);
	roughMat->AddTextureSRV("MetalnessMap", roughM);

	std::shared_ptr<Material> woodMat = std::make_shared<Material>(pixelPBRShader, vertexShader, XMFLOAT3(1, 1, 1), 0.0f, "Wood", XMFLOAT2(2, 2));
	woodMat->AddSampler("BasicSampler", sampler);
	woodMat->AddTextureSRV("Albedo", woodA);
	woodMat->AddTextureSRV("NormalMap", woodN);
	woodMat->AddTextureSRV("RoughnessMap", woodR);
	woodMat->AddTextureSRV("MetalnessMap", woodM);

	//updating mats vector
	mats.insert(mats.end(), { matUV, matNorm, matCustom, cobbleMat4x, floorMat, paintMat, scratchedMat, bronzeMat, roughMat, woodMat });

	//updating entities vector
	entities.push_back(std::make_shared<GameEntity>(sphereMesh, cobbleMat4x));
	entities.push_back(std::make_shared<GameEntity>(helixMesh, paintMat));
	entities.push_back(std::make_shared<GameEntity>(helixMesh, scratchedMat));
	entities.push_back(std::make_shared<GameEntity>(torusMesh, roughMat));
	entities.push_back(std::make_shared<GameEntity>(cylinderMesh, bronzeMat));

	std::shared_ptr<GameEntity> floor = std::make_shared<GameEntity>(cubeMesh, woodMat);
	floor->GetTransform()->SetScale(50, 1, 50);
	floor->GetTransform()->SetPosition(0, -5, 0);
	entities.push_back(floor);


	//place entities in scene
	entities[0]->GetTransform()->MoveAbsolute(-9, 0, 5);
	entities[1]->GetTransform()->MoveAbsolute(-6, 0, 5);
	entities[2]->GetTransform()->MoveAbsolute(-3, 0, 5);
	entities[3]->GetTransform()->MoveAbsolute(0, 0, 5);
	entities[4]->GetTransform()->MoveAbsolute(3, 0, 5);



	//LIGHTING
	//changed to match the skybox
	ambientColor = XMFLOAT3(0, 0, 0); //black

	//lights
	dirLight1 = {};
	dirLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight1.Direction = XMFLOAT3(0, -1, 1);
	dirLight1.Color = XMFLOAT3(1, .5, .5);
	dirLight1.Intensity = 1.0;

	Light dirLight2 = {};
	dirLight2.Color = XMFLOAT3(.5, .5, 1);
	dirLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight2.Intensity = 0.5f;
	dirLight2.Direction = XMFLOAT3(0, 1, 0);

	Light dirLight3 = {};
	dirLight3.Color = XMFLOAT3(.5, 1, .5);
	dirLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight3.Intensity = 0.5f;
	dirLight3.Direction = XMFLOAT3(0, 0, 1);

	Light pointLight1 = {};
	pointLight1.Color = XMFLOAT3(.5, 1, 1);
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Intensity = 1.0f;
	pointLight1.Position = XMFLOAT3(-1.5f, 0, 0);
	pointLight1.Range = 10.0f;

	Light spotLight1 = {};
	spotLight1.Color = XMFLOAT3(1, .5, 1);
	spotLight1.Type = LIGHT_TYPE_SPOT;
	spotLight1.Intensity = 2.0f;
	spotLight1.Position = XMFLOAT3(6.0f, 1.5f, 0);
	spotLight1.Direction = XMFLOAT3(0, -1, 0);
	spotLight1.Range = 10.0f;
	spotLight1.SpotOuterAngle = XMConvertToRadians(30.0f);
	spotLight1.SpotInnerAngle = XMConvertToRadians(20.0f);


	lights.push_back(dirLight1);
	lights.push_back(dirLight2);
	lights.push_back(dirLight3);
	lights.push_back(pointLight1);
	lights.push_back(spotLight1);

	for (int i = 0; i < lights.size(); i++)
		if (lights[i].Type != LIGHT_TYPE_POINT)
			XMStoreFloat3(
				&lights[i].Direction,
				XMVector3Normalize(XMLoadFloat3(&lights[i].Direction))
			);


	//POST PROCESSING EFFECTS
	//loading necessary shaders
	blurPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"BlurPS.cso").c_str());
	fullscreenVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"FullscreenVS.cso").c_str());


	//create pp resources 
	CreatePostProcessingResources();

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());
}

void Game::CreateShadowMapResources()
{
	shadowOptions.ShadowDSV.Reset();
	shadowOptions.ShadowSRV.Reset();
	shadowSampler.Reset();
	shadowRasterizer.Reset();

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowOptions.ShadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowOptions.ShadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowOptions.ShadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowOptions.ShadowSRV.GetAddressOf());

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // COMPARISON filter!
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);



	// light matrices
	// View
	//when light direction changes, need to update and find new position
	XMMATRIX lightView = XMMatrixLookAtLH(
		XMVectorSet(0, 20, -20, 0), // Position
		XMVectorSet(0, 0, 0, 0), // Direction: light's direction [using the first directional light]
		XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)
	XMStoreFloat4x4(&shadowOptions.ShadowViewMatrix, lightView);

	//projection
	shadowOptions.ShadowProjectionSize = 25.0f; // Tweak for your scene!
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		shadowOptions.ShadowProjectionSize,
		shadowOptions.ShadowProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&shadowOptions.ShadowProjectionMatrix, lightProjection);
}

void Game::CreatePostProcessingResources()
{
	//resets views if window size changes
	ppSRV.Reset();
	ppRTV.Reset();

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (unsigned int)(Window::Width());
	textureDesc.Height = (unsigned int)(Window::Height());
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());
	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	//update proj matrix based on aspect ratio	
	float aspectRatio = Window::AspectRatio();
	for (auto& camera : cameras)
	{
		camera->UpdateProjectionMatrix(aspectRatio);
	}

	if (Graphics::Device) CreatePostProcessingResources();
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//new frame init
	ImGuiFrame(deltaTime);
	//UI creation
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	//update entity transforms each frame
	//float scale = (float)sin(totalTime * 5) * 0.5f + 1.0f;
	//entities[0]->GetTransform()->SetScale(scale, scale, scale);

	if (cameras.size() > 0)
	{
		cameras[activeCameraIndex]->Update(deltaTime);
	}

	//entity movement
	entities[0]->GetTransform()->SetPosition(-9, -sin(totalTime), 0);
	entities[1]->GetTransform()->SetPosition(-6, sin(totalTime), 0);
	entities[2]->GetTransform()->SetPosition(-3 - sin(totalTime), 0, 0);
	entities[3]->GetTransform()->SetPosition(0, 0, sin(totalTime));
	entities[4]->GetTransform()->SetPosition(3 + sin(totalTime), 0, 0);

	//updating lightView matrix if light direction changes
	XMFLOAT3 lightDirFloat3 = lights[0].Direction;
	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&lightDirFloat3));
	XMMATRIX lightView = XMMatrixLookToLH(
		lightDir * -20.0f,  //move light backwards
		lightDir,           //move to new direction
		XMVectorSet(0, 1, 0, 0));

	// Store the new matrix
	XMStoreFloat4x4(&shadowOptions.ShadowViewMatrix, lightView);

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	colorPkr);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//post processing pre draw phase
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Graphics::Context->ClearRenderTargetView(ppRTV.Get(), clearColor);

	

	//render the shadow map before anything else
	RenderShadowMap();

	//swapping active render target
	Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

	//entity render loop  draw phase
	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	//get matrices from the camera
	//copy data to constant buffer
	for (auto& entity : entities)
	{
		std::shared_ptr<SimpleVertexShader> vs = entity->GetMat()->GetVertexShader();
		vs->SetMatrix4x4("lightView", shadowOptions.ShadowViewMatrix);
		vs->SetMatrix4x4("lightProjection", shadowOptions.ShadowProjectionMatrix);

		std::shared_ptr<SimplePixelShader> ps = entity->GetMat()->GetPixelShader();
		ps->SetFloat("time", totalTime);
		ps->SetFloat3("ambientColor", ambientColor);
		//entity->GetMat()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		ps->SetInt("lightCount", (int)lights.size());
		ps->SetData(
			"lights", &lights[0], // The address of the data to set
			sizeof(Light) * (int)lights.size());// The address of the data to set

		//sending cam pos to pixle shader for specualr lighting
		ps->SetFloat3("cameraPos", cameras[activeCameraIndex]->GetTransform()->GetPosition());

		ps->SetShaderResourceView("ShadowMap", shadowOptions.ShadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);

		//setting fog values
		ps->SetFloat("farClipDist", cameras[activeCameraIndex]->GetFarCP());
		ps->SetInt("fogType", fogType);
		ps->SetFloat3("fogColor", fogColor);
		ps->SetFloat("fogStartDist", fogStartDist);
		ps->SetFloat("fogEndDist", fogEndDist);
		ps->SetFloat("fogDensity", fogDensity);
		ps->SetInt("heightBasedFog", heightBasedFog);
		ps->SetFloat("fogHeight", fogHeight);
		ps->SetFloat("fogVerticalDensity", fogVerticalDensity);

		entity->Draw(cameras[activeCameraIndex]);

	}

	sky->Draw(cameras[activeCameraIndex]);

	//post processing post draw phase
	//restoring back buffer
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

	//setting post process VS and PS, data SRV and samplers
	// Activate shaders and bind resources
	// Also set any required cbuffer data (not shown)
	fullscreenVS->SetShader();
	blurPS->SetShader();
	blurPS->SetShaderResourceView("Pixels", ppSRV.Get());
	blurPS->SetSamplerState("ClampSampler", ppSampler.Get());

	blurPS->SetFloat("pixelWidth", 1.0f / Window::Width());
	blurPS->SetFloat("pixelHeight", 1.0f / Window::Height());
	blurPS->SetInt("blurRadius", blurRad);
	blurPS->CopyAllBufferData();

	Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

	//unbinds srvs at end of frame
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

//render shadow map with light pov
void Game::RenderShadowMap()
{
	//clear the shadow map
	Graphics::Context->OMSetRenderTargets(0, 0, shadowOptions.ShadowDSV.Get());
	Graphics::Context->ClearDepthStencilView(shadowOptions.ShadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	//change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)shadowOptions.ShadowMapResolution;
	viewport.Height = (float)shadowOptions.ShadowMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	//entity render loop
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowOptions.ShadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowOptions.ShadowProjectionMatrix);
	//deactivate pixel shader
	Graphics::Context->PSSetShader(0, 0, 0);

	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e->GetMesh()->Draw();
	}

	//set up output merger stage


	//resetting the pipeline
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get());
	Graphics::Context->RSSetState(0);

}

void Game::ImGuiFrame(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();
}

void Game::BuildUI()
{
	ImGui::Begin("Custom UI Window + Mesh Info"); // Everything after is part of the window
	{
		ImGui::PushItemWidth(200);
		ImGui::Text("This text is in the window");
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

		ImGui::ColorEdit4("RGBA Background Color", colorPkr);

		//DEMO WINDOW BUTTON
		if (ImGui::Button("Toggle ImGui Demo Window"))
		{
			demoWindowShown = !demoWindowShown;
		}
		if (demoWindowShown)
		{
			ImGui::ShowDemoWindow();
		}

		//mesh ui info
		if (ImGui::CollapsingHeader("Mesh Debug Information"))
		{
			for (size_t i = 0; i < meshes.size(); i++)
			{
				auto& mesh = meshes[i];
				ImGui::Text("Mesh %zu: %s", i, meshes[i]->GetShapeName());
				ImGui::Text("Triangle Count: %d", (meshes[i]->GetIndexCount()/3));
				ImGui::Text("Vertex Count: %d", meshes[i]->GetVertexCount());
				ImGui::Text("Index Count: %d", meshes[i]->GetIndexCount());
				ImGui::Separator();
			}
		}

		//entity ui info
		if (ImGui::CollapsingHeader("Entity Debug Information"))
		{
			for (int i = 0; i < entities.size(); i++)
			{
				std::shared_ptr<Transform> trans = entities[i]->GetTransform();
				XMFLOAT3 pos = trans->GetPosition();
				XMFLOAT3 rot = trans->GetPitchYawRoll();
				XMFLOAT3 sca = trans->GetScale();
				XMFLOAT3 colorTint = entities[i]->GetMat()->GetColorTint();
				XMFLOAT2 uvScale = entities[i]->GetMat()->GetUVScale();
				XMFLOAT2 uvOffset = entities[i]->GetMat()->GetUVOffset();
				ImGui::PushID(i);
				if (ImGui::TreeNode("Entity Node", "Entity %d", i))
				{	
					ImGui::Text("Mesh Shape: %s", entities[i]->GetMesh()->GetShapeName());
					ImGui::Text("Material Name: %s", entities[i]->GetMat()->GetName());
					ImGui::Text("Index Count: %d", entities[i]->GetMesh()->GetIndexCount());
					if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) trans->SetPosition(pos);
					if (ImGui::DragFloat3("Rotation (Radians)", &rot.x, 0.01f)) trans->SetRotation(rot);
					if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) trans->SetScale(sca);

					//material color tint adjsut
					if (ImGui::ColorEdit4("Color Tint", &colorTint.x))
						entities[i]->GetMat()->SetColorTint(colorTint);

					//UV Scale and Offset Adjust
					if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.01f))
						entities[i]->GetMat()->SetUVScale(uvScale);
					if (ImGui::DragFloat2("UV Offset", &uvOffset.x, 0.01f))
						entities[i]->GetMat()->SetUVOffset(uvOffset);

					//for each of the entities textures [there is amax 2 right now]
					for (auto& it : entities[i]->GetMat()->GetTextureSRVMap())
					{
						//display texture name
						ImGui::Text(it.first.c_str());

						//if there is texutres
						if (it.second)
						{
							ImGui::Image((ImTextureID)it.second.Get(), ImVec2(256, 256));
						}
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		//mesh ui info
		if (ImGui::CollapsingHeader("Light Debug Information"))
		{
			//for each light
			for (int i = 0; i < lights.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::Text("Light: %d", i);
				std::string lightName = "Light %d";
				switch (lights[i].Type)
				{
				case LIGHT_TYPE_DIRECTIONAL: lightName += " (Directional)"; break;
				case LIGHT_TYPE_POINT: lightName += " (Point)"; break;
				case LIGHT_TYPE_SPOT: lightName += " (Spot)"; break;
				}

				//Color
				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				ImGui::SliderFloat("Intensity", &lights[i].Intensity, 0.0f, 10.0f);

				//Direction
				if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL || lights[i].Type == LIGHT_TYPE_SPOT)
				{
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.1f);
					XMVECTOR dirNorm = XMVector3Normalize(XMLoadFloat3(&lights[i].Direction));
					XMStoreFloat3(&lights[i].Direction, dirNorm);
				}

				//Position/Range
				if (lights[i].Type == LIGHT_TYPE_POINT || lights[i].Type == LIGHT_TYPE_SPOT)
				{
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.1f);
					ImGui::SliderFloat("Range", &lights[i].Range, 0.1f, 100.0f);
				}
				ImGui::PopID();
			}
		}

		//SHADOW MAP
		if (ImGui::CollapsingHeader("Shadow Map Info"))
		{
			ImGui::Image((ImTextureID)shadowOptions.ShadowSRV.Get(), ImVec2(512, 512));
		}

		//BOX BLUR
		if (ImGui::CollapsingHeader("Blur Post Processing Info"))
		{
			ImGui::SliderInt("Blur Radius", &blurRad, 0, 25);
		}

		//FOG
		if (ImGui::CollapsingHeader("Fog Post Processing Info"))
		{
			ImGui::ColorEdit4("Fog Color", &fogColor.x);
			ImGui::SliderInt("Fog Type [Linear - Smooth - Exponential]", &fogType, 0, 2);
			if (fogType == 1)
			{
				ImGui::SliderFloat("Fog Start Distance", &fogStartDist, 0, 50);
				ImGui::SliderFloat("Fog End Distance", &fogEndDist, 0, 50);
			}
			else if (fogType == 2)
			{
				ImGui::SliderFloat("Fog Density", &fogDensity, 0, 1);
			}
			ImGui::SliderInt("Height Based Fog [Off - On]", &heightBasedFog, 0, 1);			
			if (heightBasedFog)
			{
				ImGui::SliderFloat("Fog Height", &fogHeight, 0, 15);
				ImGui::SliderFloat("Fog Vertical Density", &fogVerticalDensity, 0, 1);
			}

		}
	}
	ImGui::End();	

	ImGui::Begin("Camera Control");
	ImGui::PushItemWidth(200);
	for (int i = 0; i < cameras.size(); ++i)
	{
		//radio button label
		//swap between cameras at runtime
		std::string label = "Camera " + std::to_string(i + 1);
		if (ImGui::RadioButton(label.c_str(), activeCameraIndex == i))
		{
			activeCameraIndex = i; 
		}
	}

	//if there are cameras
	//display active camera details
	if (!cameras.empty())
	{
		auto& camera = cameras[activeCameraIndex];
		ImGui::Text("Active Camera: %d", activeCameraIndex + 1);
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera->GetTransform()->GetPosition().x,
			camera->GetTransform()->GetPosition().y, camera->GetTransform()->GetPosition().z);
		ImGui::Text("Field of View: %.2f", camera->Getfov());
	}
    ImGui::End();
}




