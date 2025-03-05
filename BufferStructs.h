#pragma once
#include <DirectXMath.h>

struct ConstantBufferData
{
    DirectX::XMFLOAT4 colorTint;
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
    //DirectX::XMFLOAT3 offset;
};