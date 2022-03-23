#include "ipch.h"
#include "Dx12GraphicsContext.h"

#include <dxgi1_4.h>;
#include <comdef.h>

namespace IlluminoEngine
{
	Dx12GraphicsContext::Dx12GraphicsContext(void* window)
	{
		Window* win = static_cast<Window*>(window);
		CreateDeviceAndSwapChain(win);
	}

	void Dx12GraphicsContext::Init()
	{
	}

	void Dx12GraphicsContext::SwapBuffers()
	{
	}

	const static uint32_t s_QueueSlotCount = 3;

	struct RenderEnvironment
	{
		Microsoft::WRL::ComPtr<ID3D12Device> Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> Queue;
		Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	};

	static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL minFeatureLevel)
	{
		*ppAdapter = nullptr;
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			IDXGIAdapter1* pAdapter = nullptr;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
			{
				// No more adapters to enumerate.
				break;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(pAdapter, minFeatureLevel, _uuidof(ID3D12Device), nullptr)))
			{
				*ppAdapter = pAdapter;
				return;
			}
			pAdapter->Release();
		}
	}

	static RenderEnvironment CreateDeviceAndSwapChainHelper(D3D_FEATURE_LEVEL minFeatureLevel, DXGI_SWAP_CHAIN_DESC* swapChainDesc)
	{
		RenderEnvironment result;

		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
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
		
		hr = D3D12CreateDevice(adapter.Get(), minFeatureLevel, IID_PPV_ARGS(&result.Device));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to find a compatible device");

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		hr = result.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&result.Queue));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create command queue");

		DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
		hr = dxgiFactory->CreateSwapChain(result.Device.Get(), &swapChainDescCopy, &result.SwapChain);
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create SwapChain");

		return result;
	}

	void Dx12GraphicsContext::CreateDeviceAndSwapChain(Window* window)
	{
#ifdef ILLUMINO_DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		D3D12GetDebugInterface(IID_PPV_ARGS (&debugController));
		debugController->EnableDebugLayer();
#endif // ILLUMINO_DEBUG

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		swapChainDesc.BufferCount = s_QueueSlotCount;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Width = window->GetWidth();
		swapChainDesc.BufferDesc.Height = window->GetHeight();
		swapChainDesc.OutputWindow = window->GetHwnd();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Windowed = true;

		auto renderEnv = CreateDeviceAndSwapChainHelper(D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

		m_Device = renderEnv.Device;
		m_CommandQueue = renderEnv.Queue;
		m_SwapChain = renderEnv.SwapChain;

		m_RenderTargetViewDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Setting up SwapChain
		
	}
}