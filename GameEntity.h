#pragma once
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh,
		std::shared_ptr<Material> mat);
	
	//getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMat();

	//setters
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMat(std::shared_ptr<Material> mat);

	//other methods
	void Draw(std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> mat;
};

