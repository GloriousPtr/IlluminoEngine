#include "ipch.h"
#include "Dx12RendererAPI.h"

#include <glm/gtc/type_ptr.hpp>

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

		m_CommandList->ClearRenderTargetView(m_RenderTarget, glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::DrawIndexed(const Ref<MeshBuffer>& meshBuffer)
	{
		OPTICK_EVENT();

		m_CommandList->DrawIndexedInstanced(meshBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void Dx12RendererAPI::SetConstantBufferView(void* cb, size_t offsetAligned)
	{
		OPTICK_EVENT();

		m_CommandList->SetGraphicsRootConstantBufferView(0, reinterpret_cast<ID3D12Resource*>(cb)->GetGPUVirtualAddress() + offsetAligned);
	}
}
