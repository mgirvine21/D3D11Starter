#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include <memory>
#include <string>


class Mesh
{
public:
	//constructor
	Mesh(const char* name, Vertex* vertArray, size_t numVerts, unsigned int* indexArray, size_t numIndices);
	Mesh(const char* name, const std::wstring& objFile);

	//methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return indexBuffer; }
	unsigned int GetIndexCount() { return numIndices; }
	unsigned int GetVertexCount() { return numVertices; }
	const char* GetShapeName() { return name; }

	void Draw();
	
	//destructor
	~Mesh() = default;

private:
	//ID3D11 buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	//indices and verticies in buffers
	unsigned int numIndices;
	unsigned int numVertices;
	const char* name;

	void CreateBuffers(Vertex* vertexArray, size_t vertexCount, unsigned int* indexArray, size_t indexCount);
	void CalculateTangents(Vertex* verts, size_t numVerts, unsigned int* indices, size_t numIndices);

};
