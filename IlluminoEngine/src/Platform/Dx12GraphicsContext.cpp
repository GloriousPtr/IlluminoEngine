#include "ipch.h"
#include "Dx12GraphicsContext.h"

#include <dxgi1_4.h>;
#include <comdef.h>

#ifdef ILLUMINO_DEBUG
#include <dxgidebug.h>
#endif

#include "Window.h"

namespace IlluminoEngine
{
	const static uint32_t s_QueueSlotCount = 3;

	Dx12GraphicsContext::Dx12GraphicsContext(const Window& window)
	{
		OPTICK_EVENT();
		CreateDeviceAndSwapChain(window.GetWidth(), window.GetHeight(), window.GetHwnd());
	}

	void Dx12GraphicsContext::Init()
	{
		OPTICK_EVENT();

	}

	void Dx12GraphicsContext::SwapBuffers()
	{
		OPTICK_EVENT();

	}

	static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL minFeatureLevel)
	{
		*ppAdapter = nullptr;
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			IDXGIAdapter1* pAdapter = nullptr;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
			{
				break;
			}

			if (SUCCEEDED(D3D12CreateDevice(pAdapter, minFeatureLevel, _uuidof(ID3D12Device), nullptr)))
			{
				*ppAdapter = pAdapter;
				return;
			}
			pAdapter->Release();
		}
	}

	static void DebugMessageCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR pDescription, void* pContext)
	{
		OPTICK_EVENT();

		switch (severity)
		{
		case D3D12_MESSAGE_SEVERITY_CORRUPTION:		ILLUMINO_CRITICAL("DirectX 12: {0}", pDescription);
													break;
		case D3D12_MESSAGE_SEVERITY_ERROR:			ILLUMINO_ERROR("DirectX 12: {0}", pDescription);
													break;
		case D3D12_MESSAGE_SEVERITY_WARNING:		ILLUMINO_WARN("DirectX 12: {0}", pDescription);
													break;
		}
	}

	void Dx12GraphicsContext::CreateDeviceAndSwapChain(uint32_t width, uint32_t height, HWND hwnd)
	{
		OPTICK_EVENT();

		UINT dxgiFactoryFlags = 0;
#ifdef ILLUMINO_DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();

		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue));
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

#endif // ILLUMINO_DEBUG

		D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = s_QueueSlotCount;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;


		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create DXGI Factory");
		
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		GetHardwareAdapter(dxgiFactory.Get(), &adapter, minFeatureLevel);
		if (adapter != nullptr)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			adapter->GetDesc(&adapterDesc);
			ILLUMINO_INFO("DirectX Info:");

			_bstr_t wc(adapterDesc.Description);
			const char* desc = wc;
			ILLUMINO_INFO("  Device: {0}", desc);
			ILLUMINO_INFO("  DedicatedVideoMemory: {0}", adapterDesc.DedicatedVideoMemory / (1024.0f * 1024.0f * 1024.0f));
		}
		
		hr = D3D12CreateDevice(adapter.Get(), minFeatureLevel, IID_PPV_ARGS(&m_Device));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to find a compatible device");

#ifdef ILLUMINO_DEBUG
		Microsoft::WRL::ComPtr<ID3D12InfoQueue1> infoQueue;
		m_Device.As(&infoQueue);

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		DWORD cookie;
		infoQueue->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &cookie);
#endif // ILLUMINO_DEBUG

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create command queue");

		DXGI_SWAP_CHAIN_DESC1 swapChainDescCopy = swapChainDesc;
		hr = dxgiFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), hwnd, &swapChainDescCopy, nullptr, nullptr, &m_SwapChain);
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create SwapChain");

		m_RenderTargetViewDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Setting up SwapChain
		
	}
}