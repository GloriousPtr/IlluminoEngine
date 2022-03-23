#pragma once

#include "Core/RendererAPI.h"

namespace IlluminoEngine
{
	class Dx12RendererAPI : public RendererAPI
	{
	public:
		Dx12RendererAPI();

		virtual void Init() override;
		virtual void SetViewportSize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
	};
}
