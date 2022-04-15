#pragma once

#include "Illumino/Renderer/Buffer.h"

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

		virtual uint32_t GetVertexCount() override { return m_VertexCount; }
		virtual uint32_t GetIndexCount() override { return m_IndexCount; }

	private:
		ID3D12Resource* m_UploadBuffer;
		ID3D12Resource* m_VertexBuffer;
		ID3D12Resource* m_IndexBuffer;

		uint32_t m_VertexCount;
		uint32_t m_IndexCount;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};
}
