#pragma once

#include "Illumino/Core/Core.h"
#include "Dx12Resources.h"

#include <d3d12.h>
#include <dxgi1_6.h>

namespace IlluminoEngine
{
	class Dx12RenderSurface
	{
	public:
		Dx12RenderSurface(uint32_t width, uint32_t height);
		virtual ~Dx12RenderSurface();

		void Create(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format);
		void Present();
		void Resize(uint32_t width, uint32_t height);

		constexpr uint32_t GetWidth() const { return m_Width; }
		constexpr uint32_t GetHeight() const { return m_Height; }
		constexpr uint32_t GetBackBufferIndex() const { return m_CurrentBackBuffer; }
		constexpr ID3D12Resource* const GetBackBufferResource() const { return m_RenderTargetData[m_CurrentBackBuffer].Resource; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RenderTargetData[m_CurrentBackBuffer].RTVHandle.CPU; }

	private:
		void Finalize();
		void Release();

		struct RenderTargetData
		{
			ID3D12Resource* Resource = nullptr;
			DescriptorHandle RTVHandle;
		};

		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_CurrentBackBuffer = 0;
		IDXGISwapChain4* m_SwapChain = nullptr;
		RenderTargetData m_RenderTargetData[g_QueueSlotCount];
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;
	};
}
