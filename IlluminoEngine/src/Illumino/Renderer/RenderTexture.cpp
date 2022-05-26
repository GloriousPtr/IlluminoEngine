#include "ipch.h"
#include "RenderTexture.h"

#include "RendererAPI.h"
#include "Platform/D3D12/Dx12RenderTexture.h"

namespace IlluminoEngine
{
	Ref<RenderTexture> RenderTexture::Create(RenderTextureSpec spec)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:	ILLUMINO_ASSERT(false, "RendererAPI::None is currently not supported");
											return nullptr;
			case RendererAPI::API::DX12:	return CreateRef<Dx12RenderTexture>(spec);
		}

		ILLUMINO_ASSERT(false, "Unknown Render Texture");
		return nullptr;
	}
}
