#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include "Camera.h"
#include "GameEntity.h"
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "Material.h"
#include "SimpleShader.h"
#include "Lights.h"
#include "Sky.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry();
	void ImGuiFrame(float deltaTime);
	void BuildUI();
	void CreateShadowMapResources();
	void CreatePostProcessingResources();
	void RenderShadowMap();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	/*Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;*/

	//// Shaders and shader-related constructs
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	//Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	////constant buffer
	//Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	//variables for game class and UI
	int sliderNumber;
	int number;

	//color picker
	float colorPkr[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	//window toggle button
	bool demoWindowShown = false;

	//meshes / entities / cameras stored in vectors
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector < std::shared_ptr<Material>> mats;
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCameraIndex;

	DirectX::XMFLOAT4 meshColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  //white
	DirectX::XMFLOAT3 meshOffset = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);       // no offset

	//lights
	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light> lights;
	Light dirLight1;

	//sky box
	std::shared_ptr<Sky> sky;

	//shadow mapping data and resources
	ShadowOptions shadowOptions;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	//post processing data and resources 
	std::shared_ptr<SimplePixelShader> blurPS;
	std::shared_ptr<SimpleVertexShader> fullscreenVS;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;
	// Resources that are tied to a particular post process
	std::shared_ptr<SimplePixelShader> ppPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // For sampling

	//variables for pp imgui
	//box blur
	int blurRad;

	//fog
	int fogType;
	DirectX::XMFLOAT3 fogColor;
	float fogStartDist;
	float fogEndDist;
	float fogDensity;
	int heightBasedFog;
	float fogHeight;
	float fogVerticalDensity;
};

