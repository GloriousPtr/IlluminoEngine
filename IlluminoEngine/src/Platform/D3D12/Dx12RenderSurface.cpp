#include "ipch.h"
#include "Dx12RenderSurface.h"

#include "Illumino/Core/Application.h"
#include "Window.h"
#include "Dx12GraphicsContext.h"

namespace IlluminoEngine
{
	Dx12RenderSurface::Dx12RenderSurface(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
	}

	Dx12RenderSurface::~Dx12RenderSurface()
	{
		Release();
	}

	void Dx12RenderSurface::Create(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format)
	{
		ILLUMINO_ASSERT(factory && commandQueue);

		if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
			format = DXGI_FORMAT_R8G8B8A8_UNORM;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = g_QueueSlotCount;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = 0;
		desc.Format = format;
		desc.Width = m_Width;
		desc.Height = m_Height;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Stereo = false;

		IDXGISwapChain1* swapChain;
		HWND hwnd = Application::GetApplication()->GetWindow()->GetHwnd();
		HRESULT hr = factory->CreateSwapChainForHwnd(commandQueue, hwnd, &desc, nullptr, nullptr, &swapChain);
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create the swapchain");
		hr = factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
		ILLUMINO_ASSERT(SUCCEEDED(hr));
		hr = swapChain->QueryInterface(IID_PPV_ARGS(&m_SwapChain));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to querry the swapchain");
		swapChain->Release();

		m_CurrentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			DescriptorHeap& heap = Dx12GraphicsContext::s_Context->GetRTVDescriptorHeap();
			m_RenderTargetData[i].RTVHandle = heap.Allocate();
		}

		Finalize();
	}

	void Dx12RenderSurface::Present()
	{
		ILLUMINO_ASSERT(m_SwapChain);
		HRESULT hr = m_SwapChain->Present(0, 0);
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to present the swapchain");
		m_CurrentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void Dx12RenderSurface::Resize(uint32_t width, uint32_t height)
	{
	}

	void Dx12RenderSurface::Finalize()
	{
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);
		ID3D12Device* device = Dx12GraphicsContext::s_Context->GetDevice();

		for (size_t i = 0 ; i < g_QueueSlotCount; ++i)
		{
			RenderTargetData& data = m_RenderTargetData[i];
			ILLUMINO_ASSERT(!data.Resource);
			HRESULT hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&data.Resource));
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			device->CreateRenderTargetView(data.Resource, &desc, data.RTVHandle.CPU);
		}

		int width = m_Width;
		int height = m_Height;
		m_Viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
		m_ScissorRect = { 0, 0, width, height };
	}

	void Dx12RenderSurface::Release()
	{
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);
		Dx12GraphicsContext* context = Dx12GraphicsContext::s_Context;

		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			RenderTargetData& data = m_RenderTargetData[i];
			data.Resource->Release();
			data.Resource = nullptr;

			DescriptorHeap& heap = context->GetRTVDescriptorHeap();
			heap.Free(data.RTVHandle);
		}

		m_SwapChain->Release();
		m_SwapChain = nullptr;
	}
}
