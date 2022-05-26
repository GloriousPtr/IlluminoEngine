#pragma once

#include <glm/glm.hpp>

namespace IlluminoEngine
{
	struct RenderTextureSpec
	{
		uint32_t Width = 1920;
		uint32_t Height = 1080;

		uint32_t Samples = 1;
	};

	class RenderTexture
	{
	public:
		virtual ~RenderTexture() = default;

		virtual void Resize(size_t width, size_t height) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void SetClearColor(glm::vec4 color) = 0;

		virtual const RenderTextureSpec& GetSpecification() const = 0;
		virtual uint64_t GetRendererID() = 0;

		static Ref<RenderTexture> Create(RenderTextureSpec spec);
	};
}
