#pragma once

#include "Core/Buffer.h"

#include "d3dx12.h"

namespace IlluminoEngine
{
	class Dx12MeshBuffer : public MeshBuffer
	{
	public:
		Dx12MeshBuffer(float* vertexData, uint32_t* indexData, size_t verticesSize, size_t indicesSize, size_t strideSize);
		virtual ~Dx12MeshBuffer() override;

		virtual void* GetVertexBufferView() override { return &m_VertexBufferView; }
		virtual void* GetIndexBufferView() override { return &m_IndexBufferView; }
		virtual void Bind() override;

	private:
		ID3D12Resource* m_UploadBuffer;
		ID3D12Resource* m_VertexBuffer;
		ID3D12Resource* m_IndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};
}
