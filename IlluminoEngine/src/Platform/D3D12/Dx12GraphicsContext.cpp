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
	
	static Dx12RendererAPI* s_RendererAPI;

	Dx12GraphicsContext::Dx12GraphicsContext(const Window& window)
		: m_Window(window), m_Vsync(true)
	{
		OPTICK_EVENT();

	}

	void Dx12GraphicsContext::Init()
	{
		OPTICK_EVENT();

		CreateDeviceAndSwapChain();
		CreateAllocatorsAndCommandLists();
		CreateViewportScissor();

		s_RendererAPI = reinterpret_cast<Dx12RendererAPI*>(RenderCommand::s_RendererAPI.get());
		s_RendererAPI->m_Device = m_Device;
		s_RendererAPI->m_CommandQueue = m_CommandQueue;
		s_RendererAPI->m_CommandList = m_CommandLists[m_CurrentBackBuffer];

		WaitForFence(m_Fences[m_CurrentBackBuffer], m_FenceValues[m_CurrentBackBuffer], m_FenceEvents[m_CurrentBackBuffer]);
		PrepareRender();
	}

	void Dx12GraphicsContext::SwapBuffers()
	{
		OPTICK_EVENT();

		{
			OPTICK_EVENT("Finalize Render");

			// Transition the swap chain back to present
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = m_RenderTargets[m_CurrentBackBuffer];
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

		{
			OPTICK_EVENT("Present");

			uint32_t syncInterval = m_Vsync ? 1 : 0;
			uint32_t presentFlags = m_Vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
			HRESULT hr = m_SwapChain->Present(syncInterval, presentFlags);
			ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to present the swapchain");

			const uint64_t fenceValue = m_CurrentFenceValue;
			m_CommandQueue->Signal(m_Fences[m_CurrentBackBuffer], fenceValue);
			m_FenceValues[m_CurrentBackBuffer] = fenceValue;
			++m_CurrentFenceValue;
			m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % s_QueueSlotCount;
		}

		WaitForFence(m_Fences[m_CurrentBackBuffer], m_FenceValues[m_CurrentBackBuffer], m_FenceEvents[m_CurrentBackBuffer]);
		PrepareRender();
	}

	void Dx12GraphicsContext::Shutdown()
	{
		for (size_t i = 0; i < s_QueueSlotCount; ++i)
			WaitForFence(m_Fences[i], m_FenceValues[i], m_FenceEvents[i]);

		for (auto e : m_FenceEvents)
			CloseHandle(e);

#ifdef ILLUMINO_DEBUG
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
			m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue->Release();
		}
#endif // ILLUMINO_DEBUG

		for (size_t i = 0; i < s_QueueSlotCount; ++i)
		{
			m_CommandLists[i]->Release();
			m_CommandAllocators[i]->Release();
			m_RenderTargets[i]->Release();

			m_Fences[i]->Release();
		}

		m_RenderTargetDescriptorHeap->Release();

		m_SwapChain->Release();
		m_CommandQueue->Release();
		m_Device->Release();
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

	void Dx12GraphicsContext::CreateDeviceAndSwapChain()
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

		D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = s_QueueSlotCount;
		swapChainDesc.Width = m_Window.GetWidth();
		swapChainDesc.Height = m_Window.GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;


		IDXGIFactory7* dxgiFactory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create DXGI Factory");
		
		IDXGIAdapter1* adapter;
		GetHardwareAdapter(dxgiFactory, &adapter, minFeatureLevel);
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
		
		hr = D3D12CreateDevice(adapter, minFeatureLevel, IID_PPV_ARGS(&m_Device));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to find a compatible device");

#ifdef ILLUMINO_DEBUG
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
			ID3D12InfoQueue* infoQueue;
			m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->Release();
		}
		
#endif // ILLUMINO_DEBUG

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create command queue");

		DXGI_SWAP_CHAIN_DESC1 swapChainDescCopy = swapChainDesc;
		hr = dxgiFactory->CreateSwapChainForHwnd(m_CommandQueue, m_Window
			.GetHwnd(), &swapChainDescCopy, nullptr, nullptr, &m_SwapChain);
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create SwapChain");

		m_RenderTargetViewDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Setting up Fences and SwapChain
		m_CurrentFenceValue = 1;
		for (size_t i = 0; i < s_QueueSlotCount; ++i)
		{
			m_FenceEvents[i] = CreateEvent(nullptr, false, false, nullptr);
			m_FenceValues[i] = 0;
			m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[i]));
		}

		for (UINT i = 0; i < s_QueueSlotCount; ++i)
		{
			m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]));
		}

		// Setup Render Targets
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = s_QueueSlotCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RenderTargetDescriptorHeap));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle { m_RenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

		for (size_t i = 0; i < s_QueueSlotCount; ++i)
		{
			D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			m_Device->CreateRenderTargetView(m_RenderTargets[i], &viewDesc, rtvHandle);
			rtvHandle.Offset(m_RenderTargetViewDescriptorSize);
		}

		adapter->Release();
		dxgiFactory->Release();
	}

	void Dx12GraphicsContext::CreateAllocatorsAndCommandLists()
	{
		for (size_t i = 0; i < s_QueueSlotCount; ++i)
		{
			m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i]));
			m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[i], nullptr, IID_PPV_ARGS(&m_CommandLists[i]));
			_bstr_t wc(i);
			m_CommandLists[i]->SetName(wc);
			m_CommandLists[i]->Close();
		}
	}

	void Dx12GraphicsContext::CreateViewportScissor()
	{
		OPTICK_EVENT();

		int width = m_Window.GetWidth();
		int height = m_Window.GetHeight();
		m_RectScissor = { 0, 0, width, height };
		m_Viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
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

	void Dx12GraphicsContext::WaitForFence(
		void* fence,
		uint64_t completionValue,
		HANDLE waitEvent)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(fence != nullptr, "Fence is null");

		ID3D12Fence* d3d12Fence = reinterpret_cast<ID3D12Fence*>(fence);
		if (d3d12Fence->GetCompletedValue() < completionValue)
		{
			d3d12Fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void Dx12GraphicsContext::PrepareRender()
	{
		OPTICK_EVENT();

		m_CommandAllocators[m_CurrentBackBuffer]->Reset();

		auto commandList = m_CommandLists[m_CurrentBackBuffer];
		commandList->Reset(m_CommandAllocators[m_CurrentBackBuffer], nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(renderTargetHandle,
				m_RenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				m_CurrentBackBuffer, m_RenderTargetViewDescriptorSize);

		commandList->OMSetRenderTargets(1, &renderTargetHandle, true, nullptr);
		commandList->RSSetViewports(1, &m_Viewport);
		commandList->RSSetScissorRects(1, &m_RectScissor);

		// Transition back buffer
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = m_RenderTargets[m_CurrentBackBuffer];
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		commandList->ResourceBarrier(1, &barrier);

		s_RendererAPI->m_BackBufferIndex = m_CurrentBackBuffer;
		s_RendererAPI->m_CommandList = m_CommandLists[m_CurrentBackBuffer];
		s_RendererAPI->m_RenderTarget = renderTargetHandle;
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
