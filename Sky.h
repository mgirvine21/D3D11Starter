#pragma once
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"
#include <memory>
#include <wrl/client.h>

class Sky
{
public:
	//constructor
	Sky(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<Mesh> mesh,
		std::shared_ptr<SimpleVertexShader> skyVS,
		std::shared_ptr<SimplePixelShader> skyPS,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions
	);

	~Sky();

	void Draw(std::shared_ptr<Camera> camera);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSkyTexture();

private:

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;
	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> skyVS;
	

	

	
	
	

	
	
};

