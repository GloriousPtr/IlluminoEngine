#pragma once

#include <glm/glm.hpp>

#include "Buffer.h"

namespace IlluminoEngine
{
	class Renderer;

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, DX12
		};

		virtual ~RendererAPI() = default;
		virtual void Init() = 0;
		virtual void SetViewportSize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void ClearColor(const glm::vec4& color) = 0;
		virtual void DrawIndexed(const Ref<MeshBuffer>& meshBuffer, uint64_t cbvGPUHandle) = 0;

		static Scope<RendererAPI> Create();

		inline static API GetAPI() { return s_API; }
		
	private:
		static API s_API;
	};
}
