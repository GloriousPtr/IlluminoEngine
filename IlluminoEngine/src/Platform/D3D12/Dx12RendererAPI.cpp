#include "ipch.h"
#include "Dx12RendererAPI.h"

#include <glm/gtc/type_ptr.hpp>

#include "Dx12GraphicsContext.h"

namespace IlluminoEngine
{
	Dx12RendererAPI::Dx12RendererAPI()
	{
	}

	void Dx12RendererAPI::Init()
	{
		OPTICK_EVENT();

	}

	void Dx12RendererAPI::SetViewportSize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		OPTICK_EVENT();

	}

	void Dx12RendererAPI::ClearColor(const glm::vec4& color)
	{
		OPTICK_EVENT();

		auto target = Dx12GraphicsContext::s_Context->GetRenderTargetHandle();
		Dx12GraphicsContext::s_Context->GetCommandList()->ClearRenderTargetView(target, glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::DrawIndexed(const Ref<MeshBuffer>& meshBuffer)
	{
		OPTICK_EVENT();

		Dx12GraphicsContext::s_Context->GetCommandList()->DrawIndexedInstanced(meshBuffer->GetIndexCount(), 1, 0, 0, 0);
	}
}
