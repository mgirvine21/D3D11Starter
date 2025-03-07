#include "Material.h"

Material::Material(std::shared_ptr<SimplePixelShader> pixelShader, 
	std::shared_ptr<SimpleVertexShader> vertexShader, 
	DirectX::XMFLOAT3 tint,
	const char* name) :
	name(name),
	pixelShader(pixelShader),
	vertexShader(vertexShader),
	colorTint(tint)
{

}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertexShader; }
DirectX::XMFLOAT3 Material::GetColorTint() { return colorTint; }
const char* Material::GetName() { return name; }

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader) { this->pixelShader = pixelShader; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader) { this->vertexShader = vertexShader; }
void Material::SetColorTint(DirectX::XMFLOAT3 tint) { this->colorTint = tint; }

void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	//activating shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	//preparing data for the GPU
	vertexShader->SetMatrix4x4("worldMatrix", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("viewMatrix", camera->GetView());
	vertexShader->SetMatrix4x4("projectionMatrix", camera->GetProjection());

	//copy data to GPU
	vertexShader->CopyAllBufferData();

	// Send data to the pixel shader
	pixelShader->SetFloat3("colorTint", colorTint);

	//copy data to GPU
	pixelShader->CopyAllBufferData();

}