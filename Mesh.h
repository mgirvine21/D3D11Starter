#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include <memory>


class Mesh
{
private:
	//ID3D11 buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	//indices and verticies in buffers
	unsigned int indexCount;
	unsigned int vertexCount;
	const char* shapeName;

public:
	//constructor
	Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, const char* shapeName);

	//methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return indexBuffer; }
	unsigned int GetIndexCount() { return indexCount; }
	unsigned int GetVertexCount() { return vertexCount; }
	const char* GetShapeName() { return shapeName; }
	void Draw();

	//destructor
	~Mesh() = default;

};
