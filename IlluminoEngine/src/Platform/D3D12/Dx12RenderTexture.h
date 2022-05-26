#pragma once

#include <glm/glm.hpp>

#include "Illumino/Renderer/RenderTexture.h"
#include "Dx12Resources.h"

namespace IlluminoEngine
{
	class Dx12RenderTexture : public RenderTexture
	{
	public:
		Dx12RenderTexture(RenderTextureSpec spec);
		virtual ~Dx12RenderTexture() override;

		virtual void Resize(size_t width, size_t height) override;
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void SetClearColor(glm::vec4 color) override { m_ClearColor = color; }

		virtual const RenderTextureSpec& GetSpecification() const override { return m_Specification; }
		virtual uint64_t GetRendererID() override;

	private:

		struct RenderTargetData
		{
			D3D12_RESOURCE_STATES ColorState;
			ID3D12Resource* ColorResource = nullptr;
			DescriptorHandle SRVHandle;
			DescriptorHandle RTVHandle;

			D3D12_RESOURCE_STATES DepthState;
			ID3D12Resource* DepthResource = nullptr;
			DescriptorHandle DSVHandle;
		};

		RenderTargetData m_RenderTargets[g_QueueSlotCount];

		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		glm::vec2 m_ClearDepth = glm::vec2(1.0f, 0.0f);

		DXGI_FORMAT m_Format;
		RenderTextureSpec m_Specification;
	};
}
