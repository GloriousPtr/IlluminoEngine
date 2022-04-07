#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "Core/GraphicsContext.h"
#include "Core/Shader.h"

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
		
		void CreateRootSignature(ID3DBlob* rootBlob, ID3D12RootSignature** rootSignature);
		void CreatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** pipelineState);

		void BindShader(ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature);

	private:
		void CreateDeviceAndSwapChain();
		void CreateAllocatorsAndCommandLists();
		void CreateViewportScissor();

		void WaitForFence(Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t completionValue, HANDLE waitEvent);

	private:
		const static uint32_t s_QueueSlotCount = 3;

		const Window& m_Window;

		bool m_Vsync;
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_RectScissor;

		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;

		uint64_t m_CurrentFenceValue;
		uint64_t m_FenceValues[s_QueueSlotCount];
		HANDLE m_FenceEvents[s_QueueSlotCount];
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fences[s_QueueSlotCount];

		uint32_t m_RenderTargetViewDescriptorSize;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RenderTargetDescriptorHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTargets[s_QueueSlotCount];

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocators[s_QueueSlotCount];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandLists[s_QueueSlotCount];

		int32_t m_CurrentBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

		Ref<Shader> m_Shader;
	};
}
