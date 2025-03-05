#include "Transform.h"
#include <DirectXMath.h>
using namespace DirectX;

Transform::Transform() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), isDirty(true) {
    XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}

void Transform::UpdateMatrices() {
    if (!isDirty) return;
    XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMMATRIX scl = XMMatrixScaling(scale.x, scale.y, scale.z);
    XMMATRIX world = scl * rot * trans;
    XMStoreFloat4x4(&worldMatrix, world);
    XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));
    isDirty = false;
}

void Transform::SetPosition(float x, float y, float z) {
    position = { x, y, z };
    isDirty = true;
}

void Transform::SetPosition(XMFLOAT3 pos) {
    position = pos;
    isDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll) {
    rotation = { pitch, yaw, roll };
    isDirty = true;
}

void Transform::SetRotation(XMFLOAT3 rot) {
    rotation = rot;
    isDirty = true;
}

void Transform::SetScale(float x, float y, float z) {
    scale = { x, y, z };
    isDirty = true;
}

void Transform::SetScale(XMFLOAT3 scl) {
    scale = scl;
    isDirty = true;
}

XMFLOAT3 Transform::GetPosition() const { return position; }
XMFLOAT3 Transform::GetPitchYawRoll() const { return rotation; }
XMFLOAT3 Transform::GetScale() const { return scale; }

XMFLOAT4X4 Transform::GetWorldMatrix() {
    UpdateMatrices();
    return worldMatrix;
}

XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix() {
    UpdateMatrices();
    return worldInverseTransposeMatrix;
}

void Transform::MoveAbsolute(float x, float y, float z) {
    position.x += x;
    position.y += y;
    position.z += z;
    isDirty = true;
}

void Transform::MoveAbsolute(XMFLOAT3 offset) {
    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;
    isDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll) {
    rotation.x += pitch;
    rotation.y += yaw;
    rotation.z += roll;
    isDirty = true;
}

void Transform::Rotate(XMFLOAT3 rot) {
    rotation.x += rot.x;
    rotation.y += rot.y;
    rotation.z += rot.z;
    isDirty = true;
}

void Transform::Scale(float x, float y, float z) {
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
    isDirty = true;
}

void Transform::Scale(XMFLOAT3 scl) {
    scale.x *= scl.x;
    scale.y *= scl.y;
    scale.z *= scl.z;
    isDirty = true;
}

//move along our "local" axis
void Transform::MoveRelative(float x, float y, float z)
{
    XMVECTOR movement = XMVectorSet(x, y, z, 0);
    //a quaternion for ration based p/y/r
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    //perform rotation of desired movment
    XMVECTOR dir = XMVector3Rotate(movement, rotQuat);
    //store rotated direction and add to our position
    XMStoreFloat3(&position, XMLoadFloat3(&position) + dir); 
}

void Transform::MoveRelative(XMFLOAT3 offset)
{
    MoveRelative(offset.x, offset.y, offset.z);
}

XMFLOAT3 Transform::GetRight() 
{
    XMVECTOR right = XMVectorSet(1,0,0,0);
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMVECTOR dir = XMVector3Rotate(right, rotQuat);
    XMFLOAT3 result;
    XMStoreFloat3(&result, dir);
    return result;
}

XMFLOAT3 Transform::GetUp() 
{
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMVECTOR dir = XMVector3Rotate(up, rotQuat);
    XMFLOAT3 result;
    XMStoreFloat3(&result, dir);
    return result;
}

XMFLOAT3 Transform::GetForward() 
{
    XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMVECTOR dir = XMVector3Rotate(forward, rotQuat);
    XMFLOAT3 result;
    XMStoreFloat3(&result, dir);
    return result;
}