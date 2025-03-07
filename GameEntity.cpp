#include "GameEntity.h"
#include "BufferStructs.h"
#include "Graphics.h"

using namespace DirectX;

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh,
	std::shared_ptr<Material> mat) : 
	mesh(mesh), 
	mat(mat), 
	transform(std::make_shared<Transform>())
{
	transform = std::make_shared<Transform>();
}

//getters
std::shared_ptr<Mesh> GameEntity::GetMesh() { return mesh; }
std::shared_ptr<Transform> GameEntity::GetTransform() { return transform; }
std::shared_ptr<Material> GameEntity::GetMat() { return mat; }

//setters
void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
void GameEntity::SetMat(std::shared_ptr<Material> mat) { this->mat = mat; }

//other methods
void GameEntity::Draw(std::shared_ptr<Camera> camera)
{
	mat->PrepareMaterial(transform, camera);

	mesh->Draw();
}