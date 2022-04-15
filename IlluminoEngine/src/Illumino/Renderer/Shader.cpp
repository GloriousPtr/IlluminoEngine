#include "ipch.h"
#include "Shader.h"

#include "RendererAPI.h"
#include "Platform/D3D12/Dx12Shader.h"

namespace IlluminoEngine
{
	Ref<Shader> Shader::Create(const char* filepath, const BufferLayout& layout)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:	ILLUMINO_ASSERT(false, "RendererAPI::None is currently not supported");
											return nullptr;
			case RendererAPI::API::DX12:	return CreateRef<Dx12Shader>(filepath, layout);
		}

		ILLUMINO_ASSERT(false, "Unknown Shader");
		return nullptr;
	}
}