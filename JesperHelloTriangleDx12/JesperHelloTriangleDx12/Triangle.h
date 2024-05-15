#pragma once
#include "Renderer.h"


class Triangle
{
private:

	ID3D12Resource1* m_dx12VertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_dx12VertexBufferView = {};


	void CreateTriangleData(Renderer& renderer);							//12. Create vertexdata
	void CreateShadersAndPiplelineState(Renderer& renderer);				//10. Set up the pipeline state


public:
	Triangle(Renderer& renderer);
	~Triangle();

	void draw(Renderer& renderer);
};

