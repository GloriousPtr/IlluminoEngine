#include "ipch.h"
#include "RendererAPI.h"

#include "Platform/D3D12/Dx12RendererAPI.h"

namespace IlluminoEngine
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::DX12;

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	ILLUMINO_ASSERT(false, "RendererAPI::None is currently not supported");
										return nullptr;
		case RendererAPI::API::DX12:	return CreateScope<Dx12RendererAPI>();
		}

		ILLUMINO_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
