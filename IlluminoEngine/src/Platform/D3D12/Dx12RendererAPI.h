#pragma once

#include "d3dx12.h"

#include "Illumino/Renderer/RendererAPI.h"

namespace IlluminoEngine
{
	class Dx12RendererAPI : public RendererAPI
	{
	public:
		Dx12RendererAPI();

		virtual void Init() override;
		virtual void SetViewportSize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void ClearColor(const glm::vec4& color) override;
		virtual void DrawIndexed(const Ref<MeshBuffer>& meshBuffer) override;

	private:
		friend class Dx12GraphicsContext;

		ID3D12Device* m_Device;
		ID3D12CommandQueue* m_CommandQueue;
		ID3D12GraphicsCommandList* m_CommandList;
		D3D12_CPU_DESCRIPTOR_HANDLE m_RenderTarget;
	};
}
