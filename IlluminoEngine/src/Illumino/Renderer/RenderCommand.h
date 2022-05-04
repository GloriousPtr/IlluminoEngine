#pragma once

#include "RendererAPI.h"
#include "Buffer.h"

namespace IlluminoEngine
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewportSize(x, y, width, height);
		}

		inline static void ClearColor(const glm::vec4& color)
		{
			s_RendererAPI->ClearColor(color);
		}

		inline static void DrawIndexed(Ref<MeshBuffer>& meshBuffer, uint64_t cbvGPUHandle)
		{
			s_RendererAPI->DrawIndexed(meshBuffer, cbvGPUHandle);
		}

	private:
		friend class Dx12GraphicsContext;

		static Scope<RendererAPI> s_RendererAPI;
	};
}
