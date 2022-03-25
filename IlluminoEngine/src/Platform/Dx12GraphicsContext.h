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
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;

		int32_t m_RenderTargetViewDescriptorSize;
	};
}
