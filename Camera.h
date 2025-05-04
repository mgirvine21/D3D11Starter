#pragma once
#include <DirectXMath.h>
#include <memory>
#include "Transform.h"
#include "Input.h"
using namespace DirectX;
class Camera
{
public:
    Camera(float aspectRatio, XMFLOAT3 pos, float fov, bool isPersp,
        float moveSpeed = 5.0f, float lookSpeed = 0.002f, 
        float nearCP = 0.1f, float farCP = 100.0f);

    void Update(float dt);
    void UpdateViewMatrix();
    void UpdateProjectionMatrix(float aspectRatio);

    //getters
    XMFLOAT4X4 GetView() const { return viewMatrix; }
    XMFLOAT4X4 GetProjection() const { return projMatrix; }
    float Getfov() { return fieldOfView; }
    float GetFarCP() { return farCP; }

    std::shared_ptr<Transform> GetTransform() { return transform; }

private:
    XMFLOAT4X4 viewMatrix;
    XMFLOAT4X4 projMatrix;

    std::shared_ptr<Transform> transform;

    float fieldOfView;
    float movementSpeed;
    float mouseLookSpeed;
    float nearCP;
    float farCP;
    bool isPerspective;
};

