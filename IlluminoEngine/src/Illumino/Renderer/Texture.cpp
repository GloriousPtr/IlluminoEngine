#include "ipch.h"
#include "Texture.h"

#include "RendererAPI.h"
#include "Platform/D3D12/Dx12Texture2D.h"

namespace IlluminoEngine
{
	Ref<Texture2D> Texture2D::Create(const char* filepath)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:	ILLUMINO_ASSERT(false, "RendererAPI::None is currently not supported");
											return nullptr;
			case RendererAPI::API::DX12:	return CreateRef<Dx12Texture2D>(filepath);
		}

		ILLUMINO_ASSERT(false, "Unknown Shader");
		return nullptr;
	}
}
