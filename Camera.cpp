#pragma once
#include "Camera.h"

Camera::Camera(float aspectRatio, XMFLOAT3 pos, float fov, bool isPersp, float moveSpeed,
	float lookSpeed, float nearCP, float farCP)
	:fieldOfView(fov),
	movementSpeed(moveSpeed),
	mouseLookSpeed(lookSpeed),
	nearCP(nearCP),
	farCP(farCP),
	isPerspective(isPersp)
{
	//init functions
	transform = std::make_shared<Transform>();
	transform->SetPosition(pos);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::Update(float dt)
{
	//called every frame
	UpdateViewMatrix();

	XMFLOAT3 move = { 0.0f, 0.0f, 0.0f };

	//keyboard movement input
	if (Input::KeyDown('W')) move.z += 1.0f;
	if (Input::KeyDown('S')) move.z -= 1.0f;
	if (Input::KeyDown('A')) move.x -= 1.0f;
	if (Input::KeyDown('D')) move.x += 1.0f;
	if (Input::KeyDown(VK_SPACE)) move.y += 1.0f;
    if (Input::KeyDown('X')) move.y -= 1.0f;

	//scale movement by time
	XMVECTOR moveVector = XMLoadFloat3(&move) * (movementSpeed * dt);
	XMStoreFloat3(&move, moveVector);

	//actually move transform
	transform->MoveRelative(move.x, move.y, move.z);

	//update view matrix to match camera transform
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	//gets pos and direction of camera transform
	XMFLOAT3 pos = transform->GetPosition();
	XMFLOAT3 direction = transform->GetForward();
	XMFLOAT3 up = transform->GetUp();
	XMFLOAT3 right = transform->GetRight();

	//create and store view matrix
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&direction), XMLoadFloat3(&up));
	XMStoreFloat4x4(&viewMatrix, view);

	//mouse movement input
	if (Input::MouseLeftDown())
	{
		//mouse x and y pos
		int deltaX = Input::GetMouseXDelta();
		int deltaY = Input::GetMouseYDelta();

		//how far mouse moved since last frame
		float yaw = deltaX * mouseLookSpeed;
		float pitch = deltaY * mouseLookSpeed;

		//rotate transform based on mouse movement
		XMFLOAT3 currentRot = transform->GetPitchYawRoll();
		currentRot.y += yaw;                             
		currentRot.x += pitch;                           

		//clamp pitch with offset
		currentRot.x = max(-XM_PIDIV2 + 0.01f, min(XM_PIDIV2 - 0.01f, currentRot.x));

		transform->SetRotation(currentRot);
	}
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	//call dxMath function to make a perspective projection
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearCP, farCP);
	XMStoreFloat4x4(&projMatrix, proj);
}
//return viewMtrix / projMatrix / transform