#pragma once

#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

// Defines a single light that can be sent to the GPU
// Note: This must match light struct in shaders
//       and must also be a multiple of 16 bytes!
struct Light
{
	int					Type;
	DirectX::XMFLOAT3	Direction;	// 16 bytes

	float				Range;
	DirectX::XMFLOAT3	Position;	// 32 bytes

	float				Intensity;
	DirectX::XMFLOAT3	Color;		// 48 bytes

	float				SpotInnerAngle;
	float				SpotOuterAngle;
	DirectX::XMFLOAT2	Padding;	// 64 bytes
};

// shadow options struct
struct ShadowOptions
{
	int ShadowMapResolution;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> ShadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowSRV;

	float ShadowProjectionSize;
	DirectX::XMFLOAT4X4 ShadowViewMatrix;
	DirectX::XMFLOAT4X4 ShadowProjectionMatrix;
};