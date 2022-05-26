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
		void TransitionTo(D3D12_RESOURCE_STATES afterState);

		struct RenderTargetData
		{
			D3D12_RESOURCE_STATES State;
			ID3D12Resource* Resource = nullptr;
			DescriptorHandle SRVHandle;
			DescriptorHandle RTVHandle;
		};

		RenderTargetData m_RenderTargets[g_QueueSlotCount];

		glm::vec4 m_ClearColor;

		DXGI_FORMAT m_Format;
		RenderTextureSpec m_Specification;
	};
}
