#pragma once
#include <DirectXMath.h>

class Transform
{
public:
    Transform();

    // Setters
    void SetPosition(float x, float y, float z);
    void SetPosition(DirectX::XMFLOAT3 pos);
    void SetRotation(float pitch, float yaw, float roll);
    void SetRotation(DirectX::XMFLOAT3 rot);
    void SetScale(float x, float y, float z);
    void SetScale(DirectX::XMFLOAT3 scale);

    // Getters
    DirectX::XMFLOAT3 GetPosition() const;
    DirectX::XMFLOAT3 GetPitchYawRoll() const;
    DirectX::XMFLOAT3 GetScale() const;
    DirectX::XMFLOAT4X4 GetWorldMatrix();
    DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

    // Transformers
    void MoveAbsolute(float x, float y, float z);
    void MoveAbsolute(DirectX::XMFLOAT3 offset);
    void Rotate(float pitch, float yaw, float roll);
    void Rotate(DirectX::XMFLOAT3 rot);
    void Scale(float x, float y, float z);
    void Scale(DirectX::XMFLOAT3 scale);

    //other transform methods
    void MoveRelative(float x, float y, float z);
    void MoveRelative(DirectX::XMFLOAT3 offset);
    DirectX::XMFLOAT3 GetRight();
    DirectX::XMFLOAT3 GetUp();
    DirectX::XMFLOAT3 GetForward();

private:
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation; // Pitch, Yaw, Roll
    DirectX::XMFLOAT3 scale;
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;
    bool isDirty;

    void UpdateMatrices();
};

