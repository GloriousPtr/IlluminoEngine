#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Illumino/Renderer/GraphicsContext.h"
#include "Illumino/Renderer/Shader.h"

namespace IlluminoEngine
{
	class Window;

	class Dx12GraphicsContext : public GraphicsContext
	{
	public:
		Dx12GraphicsContext(const Window& window);
		virtual ~Dx12GraphicsContext() override = default;

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual void Shutdown() override;
		virtual void SetVsync(bool state) override { m_Vsync = state; }
		virtual void* GetDevice() override { return m_Device; }
		virtual void* GetCommandQueue() override { return m_CommandQueue; }
		virtual void* GetCommandList() override { return m_CommandLists[m_CurrentBackBuffer]; }
		virtual void WaitForFence(void* fence, uint64_t completionValue, HANDLE waitEvent) override;
		virtual void BindMeshBuffer(MeshBuffer& mesh) override;
		
		void CreateRootSignature(ID3DBlob* rootBlob, ID3D12RootSignature** rootSignature);
		void CreatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** pipelineState);

		void BindShader(ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature);
		ID3D12Resource* CreateConstantBuffer(size_t sizeAligned);
	private:
		void CreateDeviceAndSwapChain();
		void CreateAllocatorsAndCommandLists();
		void CreateViewportScissor();
		void PrepareRender();
	private:
		const static uint32_t s_QueueSlotCount = 3;

		const Window& m_Window;

		bool m_Vsync;
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_RectScissor;

		ID3D12Device* m_Device;
		ID3D12CommandQueue* m_CommandQueue;
		IDXGISwapChain1* m_SwapChain;

		uint64_t m_CurrentFenceValue;
		uint64_t m_FenceValues[s_QueueSlotCount];
		HANDLE m_FenceEvents[s_QueueSlotCount];
		ID3D12Fence* m_Fences[s_QueueSlotCount];

		uint32_t m_RenderTargetViewDescriptorSize;
		ID3D12DescriptorHeap* m_RenderTargetDescriptorHeap;
		ID3D12Resource* m_RenderTargets[s_QueueSlotCount];

		ID3D12CommandAllocator* m_CommandAllocators[s_QueueSlotCount];
		ID3D12GraphicsCommandList* m_CommandLists[s_QueueSlotCount];

		int32_t m_CurrentBackBuffer = 0;

		Ref<Shader> m_Shader;
		Ref<MeshBuffer> m_Mesh;
	};
}
