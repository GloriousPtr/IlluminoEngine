#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Illumino/Renderer/GraphicsContext.h"
#include "Illumino/Renderer/Shader.h"
#include "Dx12Resources.h"
#include "Dx12RenderSurface.h"

namespace IlluminoEngine
{
	class Window;

	class Dx12GraphicsContext : public GraphicsContext
	{
	public:
		Dx12GraphicsContext(const Window& window);
		virtual ~Dx12GraphicsContext() override;

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual void Shutdown() override;
		virtual void SetVsync(bool state) override { m_Vsync = state; }
		
		ID3D12Device* GetDevice() { return m_Device; }
		ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }
		ID3D12GraphicsCommandList* GetCommandList() { return m_CommandLists[m_CurrentBackBuffer]; }
		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetHandle() { return m_RenderSurface->GetRTV(); }

		DescriptorHeap& GetRTVDescriptorHeap() { return m_RTVDescriptorHeap; }
		DescriptorHeap& GetDSVDescriptorHeap() { return m_DSVDescriptorHeap; }
		DescriptorHeap& GetSRVDescriptorHeap() { return m_SRVDescriptorHeap; }
		DescriptorHeap& GetUAVDescriptorHeap() { return m_UAVDescriptorHeap; }
		
		uint32_t GetCurrentBackBufferIndex() { return m_CurrentBackBuffer; }

	private:
		void CreateDevice(IDXGIFactory7* factory);
		void CreateRenderSurface(IDXGIFactory7* factory);
		void CreateAllocatorsAndCommandLists();
		void CreateViewportScissor();
		void PrepareRender();
		void WaitForFence(ID3D12Fence* fence, uint64_t completionValue, HANDLE waitEvent);

		void CreateRootSignature(ID3DBlob* rootBlob, ID3D12RootSignature** rootSignature);
		void CreatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** pipelineState);
		ID3D12Resource* CreateConstantBuffer(size_t sizeAligned);

		void BindShader(ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature);
		void BindMeshBuffer(MeshBuffer& mesh);

		void SetDeferredReleasesFlag() { m_DeferredReleasesFlag[m_CurrentBackBuffer] = 1; }
		void DeferredRelease(IUnknown* resource);
		void ProcessDeferredReleases(const uint32_t frameIndex);

	private:

		const Window& m_Window;

		bool m_Vsync;
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_RectScissor;

		ID3D12Device* m_Device;
		ID3D12CommandQueue* m_CommandQueue;
		Dx12RenderSurface* m_RenderSurface;

		uint64_t m_CurrentFenceValue;
		uint64_t m_FenceValues[g_QueueSlotCount];
		HANDLE m_FenceEvents[g_QueueSlotCount];
		ID3D12Fence* m_Fences[g_QueueSlotCount];

		ID3D12CommandAllocator* m_CommandAllocators[g_QueueSlotCount];
		ID3D12GraphicsCommandList* m_CommandLists[g_QueueSlotCount];

		int32_t m_CurrentBackBuffer = 0;
		std::vector<IUnknown*> m_DeferredReleases[g_QueueSlotCount];
		uint32_t m_DeferredReleasesFlag[g_QueueSlotCount];

		DescriptorHeap m_RTVDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
		DescriptorHeap m_DSVDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
		DescriptorHeap m_SRVDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		DescriptorHeap m_UAVDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

		std::mutex m_Mutex;

		friend class Dx12MeshBuffer;
		friend class Dx12RendererAPI;
		friend class Dx12RenderSurface;
		friend class Dx12Shader;
		friend class DescriptorHeap;
		static Dx12GraphicsContext* s_Context;
	};
}
