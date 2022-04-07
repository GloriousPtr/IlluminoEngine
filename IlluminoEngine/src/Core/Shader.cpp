#include "ipch.h"
#include "Shader.h"

#include "RendererAPI.h"
#include "Platform/Dx12Shader.h"

namespace IlluminoEngine
{
	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	ILLUMINO_ASSERT(false, "RendererAPI::None is currently not supported");
										return nullptr;
		case RendererAPI::API::DX12:	return CreateRef<Dx12Shader>(filepath);
		}

		ILLUMINO_ASSERT(false, "Unknown Shader");
		return nullptr;
	}
}