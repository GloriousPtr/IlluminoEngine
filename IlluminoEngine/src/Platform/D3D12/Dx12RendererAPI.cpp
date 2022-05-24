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
		ID3D12GraphicsCommandList* commandList = Dx12GraphicsContext::s_Context->GetCommandList();

		OPTICK_GPU_CONTEXT(commandList);
		OPTICK_GPU_EVENT("ClearColor");

		commandList->ClearRenderTargetView(Dx12GraphicsContext::s_Context->GetRenderTargetHandle(), glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::DrawIndexed(const Ref<MeshBuffer>& meshBuffer, uint64_t cbvGPUHandle)
	{
		meshBuffer->Bind();

		ID3D12GraphicsCommandList* commandList = Dx12GraphicsContext::s_Context->GetCommandList();

		OPTICK_GPU_CONTEXT(commandList);
		OPTICK_GPU_EVENT("DrawIndexed");

		commandList->SetGraphicsRootConstantBufferView(1, cbvGPUHandle);
		commandList->DrawIndexedInstanced(meshBuffer->GetIndexCount(), 1, 0, 0, 0);
	}
}
