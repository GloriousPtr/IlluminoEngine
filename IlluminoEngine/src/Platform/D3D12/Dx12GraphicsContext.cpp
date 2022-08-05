#include "ipch.h"
#include "Dx12GraphicsContext.h"

#include <comdef.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

#ifdef ILLUMINO_DEBUG
#include <dxgidebug.h>
#endif

#include "Window.h"
#include "Dx12RendererAPI.h"
#include "Illumino/Renderer/RenderCommand.h"

namespace IlluminoEngine
{

#ifdef ILLUMINO_DEBUG
	static DWORD s_DebugCallbackCookie;
#endif
	
	Dx12GraphicsContext* Dx12GraphicsContext::s_Context;

	Dx12GraphicsContext::Dx12GraphicsContext(const Window& window)
		: m_Window(window), m_Vsync(true)
	{
		OPTICK_EVENT();
		
		ILLUMINO_ASSERT(!s_Context, "Dx12 Graphics Context already exists!");
		s_Context = this;
	}

	Dx12GraphicsContext::~Dx12GraphicsContext()
	{
		s_Context = nullptr;
	}

	void Dx12GraphicsContext::Init()
	{
		OPTICK_EVENT();

		UINT dxgiFactoryFlags = 0;
#ifdef ILLUMINO_DEBUG
		ID3D12Debug* debugController;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();
		debugController->Release();

		IDXGIInfoQueue* dxgiInfoQueue;
        DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue));
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		dxgiInfoQueue->Release();

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // ILLUMINO_DEBUG

		IDXGIFactory7* factory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create DXGI Factory");
		
		CreateDevice(factory);

		bool result = true;
		result &= m_RTVDescriptorHeap.Init(512, false);
		result &= m_DSVDescriptorHeap.Init(512, false);
		result &= m_SRVDescriptorHeap.Init(4096, true);
		result &= m_UAVDescriptorHeap.Init(512, true);
		ILLUMINO_ASSERT(result, "Failed to create some descriptor heap allocations");

		m_RTVDescriptorHeap.GetHeap()->SetName(L"RTV Heap");
		m_DSVDescriptorHeap.GetHeap()->SetName(L"DSV Heap");
		m_SRVDescriptorHeap.GetHeap()->SetName(L"SRV Heap");
		m_UAVDescriptorHeap.GetHeap()->SetName(L"UAV Heap");

		CreateRenderSurface(factory);
		factory->Release();

		CreateAllocatorsAndCommandLists();

		WaitForFence(m_Fences[m_CurrentBackBuffer], m_FenceValues[m_CurrentBackBuffer], m_FenceEvents[m_CurrentBackBuffer]);
		PrepareRender();
	}

	void Dx12GraphicsContext::SwapBuffers()
	{
		OPTICK_EVENT();

		// Finalize Render
		{
			// Transition the swap chain back to present
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = m_RenderSurface->GetBackBufferResource();
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			auto commandList = m_CommandLists[m_CurrentBackBuffer];
			commandList->ResourceBarrier(1, &barrier);

			commandList->Close();

			// Execute our commands
			ID3D12CommandList* commandLists [] = { commandList };
			m_CommandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
		}

		// Present
		{
			m_RenderSurface->Present(m_Vsync);

			const uint64_t fenceValue = m_CurrentFenceValue;
			m_CommandQueue->Signal(m_Fences[m_CurrentBackBuffer], fenceValue);
			m_FenceValues[m_CurrentBackBuffer] = fenceValue;
			++m_CurrentFenceValue;
			m_CurrentBackBuffer = m_RenderSurface->GetBackBufferIndex();// (m_CurrentBackBuffer + 1) % g_QueueSlotCount;
		}

		WaitForFence(m_Fences[m_CurrentBackBuffer], m_FenceValues[m_CurrentBackBuffer], m_FenceEvents[m_CurrentBackBuffer]);
		
		if (m_Window.GetWidth() != m_RenderSurface->GetWidth() || m_Window.GetHeight() != m_RenderSurface->GetHeight())
			m_RenderSurface->Resize(m_Window.GetWidth(), m_Window.GetHeight());

		PrepareRender();
	}

	void Dx12GraphicsContext::Shutdown()
	{
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
			WaitForFence(m_Fences[i], m_FenceValues[i], m_FenceEvents[i]);

		for (auto e : m_FenceEvents)
			CloseHandle(e);

#ifdef ILLUMINO_DEBUG
#ifdef ENABLE_DX12_DEBUG_MESSAGES
		ID3D12InfoQueue1* infoQueue1;
		HRESULT hr = m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue1));

		if (SUCCEEDED(hr))
		{
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue1->UnregisterMessageCallback(s_DebugCallbackCookie);
			infoQueue1->Release();
		}
		else
		{
			ID3D12InfoQueue* infoQueue;
			HRESULT hr = m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			if (SUCCEEDED(hr))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				infoQueue->Release();
			}
		}
#else
		ID3D12InfoQueue* infoQueue;
		HRESULT hr = m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (SUCCEEDED(hr))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue->Release();
		}
#endif // ENABLE_DX12_DEBUG_MESSAGES
#endif // ILLUMINO_DEBUG

		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			m_CommandLists[i]->Release();
			m_CommandAllocators[i]->Release();

			m_Fences[i]->Release();
		}

		m_RTVDescriptorHeap.Release();
		m_DSVDescriptorHeap.Release();
		m_SRVDescriptorHeap.Release();
		m_UAVDescriptorHeap.Release();

		for (uint32_t i = 0; i < g_QueueSlotCount; ++i)
			ProcessDeferredReleases(i);

		delete m_RenderSurface;

		m_CommandQueue->Release();
		m_Device->Release();

		ProcessDeferredReleases(0);
	}

	static void GetHardwareAdapter(IDXGIFactory7* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL minFeatureLevel)
	{
		*ppAdapter = nullptr;
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			IDXGIAdapter1* pAdapter = nullptr;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)))
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

	static void DebugMessageCallback(
		D3D12_MESSAGE_CATEGORY category,
		D3D12_MESSAGE_SEVERITY severity,
		D3D12_MESSAGE_ID id,
		LPCSTR pDescription,
		void* pContext)
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

	void Dx12GraphicsContext::CreateDevice(IDXGIFactory7* factory)
	{
		D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		IDXGIAdapter1* adapter;
		GetHardwareAdapter(factory, &adapter, minFeatureLevel);
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
		
		HRESULT hr = D3D12CreateDevice(adapter, minFeatureLevel, IID_PPV_ARGS(&m_Device));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to find a compatible device");
		m_Device->SetName(L"MainD3D12Device");

#ifdef ILLUMINO_DEBUG
#ifdef ENABLE_DX12_DEBUG_MESSAGES
		ID3D12InfoQueue1* infoQueue1;
		hr = m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue1));

		if (SUCCEEDED(hr))
		{
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue1->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &s_DebugCallbackCookie);
			infoQueue1->Release();
		}
		else
		{
			ILLUMINO_WARN("Could not enable enable DX12 debug messages on console window!");

			ID3D12InfoQueue* infoQueue;
			hr = m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			if (SUCCEEDED(hr))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				infoQueue->Release();
			}
			else
			{
				ILLUMINO_ERROR("Could not enable debugging support for the device!");
			}
		}
#else
		ILLUMINO_WARN("Support for DX12 debug messages on console window is disabled, define ENABLE_DX12_DEBUG_MESSAGES to enable the support, it requires Windows 11 SDK!");
		ID3D12InfoQueue* infoQueue;
		hr = m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (SUCCEEDED(hr))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->Release();
		}
		else
		{
			ILLUMINO_ERROR("Could not enable debugging support for the DX12 device!");
		}
#endif //ENABLE_DX12_DEBUG_MESSAGES
#endif // ILLUMINO_DEBUG

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create command queue");
		m_CommandQueue->SetName(L"MainD3D12CommandQueue");

		OPTICK_GPU_INIT_D3D12(m_Device, &m_CommandQueue, 1);

		adapter->Release();
	}

	void Dx12GraphicsContext::CreateRenderSurface(IDXGIFactory7* factory)
	{
		m_RenderSurface = new Dx12RenderSurface(m_Window.GetWidth(), m_Window.GetHeight());
		m_RenderSurface->Create(factory, m_CommandQueue, DXGI_FORMAT_R8G8B8A8_UNORM);

		// Setting up Fences and SwapChain
		m_CurrentFenceValue = 1;
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			m_FenceEvents[i] = CreateEvent(nullptr, false, false, nullptr);
			m_FenceValues[i] = 0;
			m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[i]));
		}
	}

	void Dx12GraphicsContext::CreateAllocatorsAndCommandLists()
	{
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i]));
			m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[i], nullptr, IID_PPV_ARGS(&m_CommandLists[i]));
			_bstr_t wc(i);
			m_CommandLists[i]->SetName(wc);
			m_CommandLists[i]->Close();
		}
	}

	void Dx12GraphicsContext::CreateRootSignature(ID3DBlob* rootBlob, ID3D12RootSignature** rootSignature)
	{
		OPTICK_EVENT();

		HRESULT hr = m_Device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create root signature");
	}

	void Dx12GraphicsContext::CreatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** pipelineState)
	{
		OPTICK_EVENT();

		HRESULT hr = m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create pipeline state");
	}

	void Dx12GraphicsContext::WaitForFence(ID3D12Fence* fence, uint64_t completionValue, HANDLE waitEvent)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(fence != nullptr, "Fence is null");

		if (fence->GetCompletedValue() < completionValue)
		{
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void Dx12GraphicsContext::WaitForAllFrames()
	{
		OPTICK_EVENT();

		for (size_t i = m_CurrentBackBuffer; i < g_QueueSlotCount; ++i)
			WaitForFence(m_Fences[i], m_FenceValues[i], m_FenceEvents[i]);
	}

	void Dx12GraphicsContext::PrepareRender()
	{
		OPTICK_EVENT();

		if (m_DeferredReleasesFlag[m_CurrentBackBuffer])
		{
			ProcessDeferredReleases(m_CurrentBackBuffer);
		}

		m_CommandAllocators[m_CurrentBackBuffer]->Reset();

		auto commandList = m_CommandLists[m_CurrentBackBuffer];
		commandList->Reset(m_CommandAllocators[m_CurrentBackBuffer], nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RenderSurface->GetRTV();

		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		ID3D12DescriptorHeap* descHeap = const_cast<ID3D12DescriptorHeap*>(m_SRVDescriptorHeap.GetHeap());
        commandList->SetDescriptorHeaps(1, &descHeap);
		
		auto& viewport = m_RenderSurface->GetViewport();
		auto& scissorRect = m_RenderSurface->GetScissorRect();
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		// Transition back buffer
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = m_RenderSurface->GetBackBufferResource();
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		commandList->ResourceBarrier(1, &barrier);
	}

	void Dx12GraphicsContext::DeferredRelease(IUnknown** resource)
	{
		m_DeferredReleases[m_CurrentBackBuffer].push_back(resource);
		SetDeferredReleasesFlag();
	}

	void Dx12GraphicsContext::ProcessDeferredReleases(const uint32_t frameIndex)
	{
		m_DeferredReleasesFlag[frameIndex] = 0;

		m_RTVDescriptorHeap.ProcessDeferredFree(frameIndex);
		m_DSVDescriptorHeap.ProcessDeferredFree(frameIndex);
		m_SRVDescriptorHeap.ProcessDeferredFree(frameIndex);
		m_UAVDescriptorHeap.ProcessDeferredFree(frameIndex);

		auto& resources = m_DeferredReleases[frameIndex];
		if (!resources.empty())
		{
			for (auto& r : resources)
			{
				(*r)->Release();
				r = nullptr;
			}

			resources.clear();
		}
	}

	void Dx12GraphicsContext::BindShader(ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature)
	{
		OPTICK_EVENT();

		auto commandList = m_CommandLists[m_CurrentBackBuffer];
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetPipelineState(pso);
		commandList->SetGraphicsRootSignature(rootSignature);
	}

	void Dx12GraphicsContext::BindMeshBuffer(MeshBuffer& mesh)
	{
		OPTICK_EVENT();

		auto commandList = m_CommandLists[m_CurrentBackBuffer];
		commandList->IASetVertexBuffers(0, 1, reinterpret_cast<D3D12_VERTEX_BUFFER_VIEW*>(mesh.GetVertexBufferView()));
		commandList->IASetIndexBuffer(reinterpret_cast<D3D12_INDEX_BUFFER_VIEW*>(mesh.GetIndexBufferView()));
	}

	ID3D12Resource* Dx12GraphicsContext::CreateConstantBuffer(size_t sizeAligned)
	{
		OPTICK_EVENT();

		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CreationNodeMask = 1;
		heapDesc.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Width = sizeAligned;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* ret = nullptr;
		HRESULT hr = m_Device->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&ret));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Could not create constant buffer");
		return ret;
	}
}
