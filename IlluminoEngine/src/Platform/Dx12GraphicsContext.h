#pragma once

#include "Core/GraphicsContext.h"

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_2.h>

namespace IlluminoEngine
{
	class Window;

	class Dx12GraphicsContext : public GraphicsContext
	{
	public:
		Dx12GraphicsContext(const Window& window);

		virtual void Init() override;
		virtual void SwapBuffers() override;
		
	private:
		void CreateDeviceAndSwapChain(uint32_t width, uint32_t height, HWND hwnd);

	private:
		const static uint32_t s_QueueSlotCount = 3;

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
	};
}
