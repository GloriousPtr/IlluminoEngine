#include "ipch.h"
#include "Buffer.h"

#include "RendererAPI.h"
#include "Platform/Dx12Buffer.h"

namespace IlluminoEngine
{
	Ref<MeshBuffer> MeshBuffer::Create(float* vertexData, uint32_t* indexData, size_t verticesSize, size_t indicesSize, size_t strideSize)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:	ILLUMINO_ASSERT(false, "RendererAPI::None is currently not supported");
											return nullptr;
			case RendererAPI::API::DX12:	return CreateRef<Dx12MeshBuffer>(vertexData, indexData, verticesSize, indicesSize, strideSize);
		}

		ILLUMINO_ASSERT(false, "Unknown Shader");
		return nullptr;
	}
}
