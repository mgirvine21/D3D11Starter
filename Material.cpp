#include "Material.h"

Material::Material(std::shared_ptr<SimplePixelShader> pixelShader, 
	std::shared_ptr<SimpleVertexShader> vertexShader, 
	DirectX::XMFLOAT3 tint,
	float roughness,
	const char* name,
	DirectX::XMFLOAT2 uvScale,
	DirectX::XMFLOAT2 uvOffset) :
	name(name),
	roughness(roughness),
	pixelShader(pixelShader),
	vertexShader(vertexShader),
	colorTint(tint),
	uvScale(uvScale),
	uvOffset(uvOffset)
{

}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertexShader; }
DirectX::XMFLOAT3 Material::GetColorTint() { return colorTint; }
const char* Material::GetName() { return name; }
DirectX::XMFLOAT2 Material::GetUVScale() { return uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return uvOffset; }
float Material::GetRoughness() { return roughness; }

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetTextureSRV(std::string name)
{
	// Search for the key
	auto it = textureSRVs.find(name);

	// Not found, return null
	if (it == textureSRVs.end())
		return 0;

	// Return the texture ComPtr
	return it->second;
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> Material::GetSampler(std::string name)
{
	auto it = samplers.find(name);
	if (it == samplers.end())
		return 0;

	return it->second;
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& Material::GetTextureSRVMap()
{
	return textureSRVs;
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>& Material::GetSamplerMap()
{
	return samplers;
}


void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader) { this->pixelShader = pixelShader; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader) { this->vertexShader = vertexShader; }
void Material::SetColorTint(DirectX::XMFLOAT3 tint) { this->colorTint = tint; }
void Material::SetUVScale(DirectX::XMFLOAT2 scale) { uvScale = scale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 offset) { uvOffset = offset; }
void Material::SetRoughness(float rough) { roughness = rough; }

void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	//activating shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	//preparing data for the GPU
	vertexShader->SetMatrix4x4("worldMatrix", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("worldInvTrans", transform->GetWorldInverseTransposeMatrix());
	vertexShader->SetMatrix4x4("viewMatrix", camera->GetView());
	vertexShader->SetMatrix4x4("projectionMatrix", camera->GetProjection());

	//copy data to GPU
	vertexShader->CopyAllBufferData();

	// Send data to the pixel shader
	pixelShader->SetFloat3("colorTint", colorTint);
	pixelShader->SetFloat2("uvScale", uvScale);
	pixelShader->SetFloat2("uvOffset", uvOffset);
	pixelShader->SetFloat("roughness", roughness);
	pixelShader->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());

	//copy data to GPU
	pixelShader->CopyAllBufferData();

	//setting up texture and sampler resources
	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second.Get()); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second.Get()); }
}



void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}