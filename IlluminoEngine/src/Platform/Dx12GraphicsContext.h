#pragma once

#include "Core/GraphicsContext.h"
#include "Window.h"

#include <wrl.h>
#include <d3d12.h>
#include <dxgi.h>

namespace IlluminoEngine
{
	class Dx12GraphicsContext : public GraphicsContext
	{
	public:
		Dx12GraphicsContext(void* window);

		virtual void Init() override;
		virtual void SwapBuffers() override;
		
	private:
		void CreateDeviceAndSwapChain(Window* window);

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		int32_t m_RenderTargetViewDescriptorSize;
	};
}
