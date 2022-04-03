#include "ipch.h"
#include "Dx12GraphicsContext.h"

#include <dxgi1_4.h>;
#include <comdef.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

#ifdef ILLUMINO_DEBUG
#include <dxgidebug.h>
#endif

#include "Window.h"

#include "Shaders.h"

namespace IlluminoEngine
{

#ifdef ILLUMINO_DEBUG
	static DWORD s_DebugCallbackCookie;
#endif
	
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

		CreateRootSignature();
		CreatePipelineState();

		// Create our upload fence, command list and command allocator
		// This will be only used while creating the mesh buffer and the texture
		// to upload data to the GPU.
		Microsoft::WRL::ComPtr<ID3D12Fence> uploadFence;
		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence));

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
		m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadCommandAllocator));
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
		m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
									uploadCommandAllocator.Get(), nullptr,
									IID_PPV_ARGS(&uploadCommandList));

		struct Vertex
		{
			float position[3];
			float uv[2];
		};

		// Declare upload buffer data as 'static' so it persists after returning from this function.
		// Otherwise, we would need to explicitly wait for the GPU to copy data from the upload buffer
		// to vertex/index default buffers due to how the GPU processes commands asynchronously. 
		static const Vertex vertices[4] =
		{
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },		// Upper Left
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },		// Upper Right
			{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },		// Bottom right
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }		// Bottom left
		};

		static const int indices[6] =
		{
			0, 1, 2,
			2, 3, 0
		};

		static const int uploadBufferSize = sizeof(vertices) + sizeof(indices);
		static const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		static const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		// Create upload buffer on CPU
		m_Device->CreateCommittedResource(&uploadHeapProperties,
											D3D12_HEAP_FLAG_NONE,
											&uploadBufferDesc,
											D3D12_RESOURCE_STATE_GENERIC_READ,
											nullptr,
											IID_PPV_ARGS(&m_UploadBuffer));

		// Create vertex & index buffer on the GPU
		// HEAP_TYPE_DEFAULT is on GPU, we also initialize with COPY_DEST state
		// so we don't have to transition into this before copying into them
		static const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		static const auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
		m_Device->CreateCommittedResource(&defaultHeapProperties,
											D3D12_HEAP_FLAG_NONE,
											&vertexBufferDesc,
											D3D12_RESOURCE_STATE_COPY_DEST,
											nullptr,
											IID_PPV_ARGS(&m_VertexBuffer));

		static const auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
		m_Device->CreateCommittedResource(&defaultHeapProperties,
											D3D12_HEAP_FLAG_NONE,
											&indexBufferDesc,
											D3D12_RESOURCE_STATE_COPY_DEST,
											nullptr,
											IID_PPV_ARGS(&m_IndexBuffer));

		// Create buffer views
		m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView.SizeInBytes = sizeof(vertices);
		m_VertexBufferView.StrideInBytes = sizeof(Vertex);

		m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.SizeInBytes = sizeof(indices);
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

		// Copy data on CPU into the upload buffer
		void* p;
		m_UploadBuffer->Map(0, nullptr, &p);
		memcpy(p, vertices, sizeof(vertices));
		memcpy(static_cast<unsigned char*>(p) + sizeof(vertices), indices, sizeof(indices));
		m_UploadBuffer->Unmap(0, nullptr);

		// Copy data from upload buffer on CPU into the index/vertex buffer on 
		// the GPU
		uploadCommandList->CopyBufferRegion(m_VertexBuffer.Get(), 0,
			m_UploadBuffer.Get(), 0, sizeof(vertices));
		uploadCommandList->CopyBufferRegion(m_IndexBuffer.Get(), 0,
			m_UploadBuffer.Get(), sizeof(vertices), sizeof(indices));

		// Barriers, batch them together
		const CD3DX12_RESOURCE_BARRIER barriers[2] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer.Get(),
												D3D12_RESOURCE_STATE_COPY_DEST,
												D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
			CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer.Get(),
												D3D12_RESOURCE_STATE_COPY_DEST,
												D3D12_RESOURCE_STATE_INDEX_BUFFER)
		};

		uploadCommandList->ResourceBarrier(2, barriers);




		uploadCommandList->Close();

		// Execute the upload and finish the command list
		ID3D12CommandList* commandLists [] = { uploadCommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
		m_CommandQueue->Signal(uploadFence.Get(), 1);

		auto waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (waitEvent == NULL) {
			throw std::runtime_error("Could not create wait event.");
		}

		WaitForFence(uploadFence.Get(), 1, waitEvent);

		// Cleanup our upload handle
		uploadCommandAllocator->Reset();

		CloseHandle(waitEvent);
	}

	void Dx12GraphicsContext::SwapBuffers()
	{
		OPTICK_EVENT();

		{
			OPTICK_EVENT("Wait For Fence");
			
			WaitForFence(m_Fences[m_CurrentBackBuffer], m_FenceValues[m_CurrentBackBuffer], m_FenceEvents[m_CurrentBackBuffer]);
		}

		{
			OPTICK_EVENT("Prepare Render");

			m_CommandAllocators[m_CurrentBackBuffer]->Reset();

			auto commandList = m_CommandLists[m_CurrentBackBuffer].Get();
			commandList->Reset(m_CommandAllocators[m_CurrentBackBuffer].Get(), nullptr);

			D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(renderTargetHandle,
					m_RenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
					m_CurrentBackBuffer, m_RenderTargetViewDescriptorSize);

			commandList->OMSetRenderTargets(1, &renderTargetHandle, true, nullptr);
			commandList->RSSetViewports(1, &m_Viewport);
			commandList->RSSetScissorRects(1, &m_RectScissor);

			// Transition back buffer
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = m_RenderTargets[m_CurrentBackBuffer].Get();
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			commandList->ResourceBarrier(1, &barrier);

			static const float clearColor [] = { 0.042f, 0.042f, 0.042f, 1.0f };

			commandList->ClearRenderTargetView(renderTargetHandle, clearColor, 0, nullptr);
		}

		{
			OPTICK_EVENT("Render");

			auto commandList = m_CommandLists[m_CurrentBackBuffer].Get();
			// Set our state (shaders, etc.)
			commandList->SetPipelineState(m_PipelineStateObject.Get());
			// Set our root signature
			commandList->SetGraphicsRootSignature(m_RootSignature.Get());

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
			commandList->IASetIndexBuffer(&m_IndexBufferView);
			commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		}

		{
			OPTICK_EVENT("Finalize Render");

			// Transition the swap chain back to present
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = m_RenderTargets[m_CurrentBackBuffer].Get();
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			auto commandList = m_CommandLists[m_CurrentBackBuffer].Get();
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
			m_SwapChain->Present(syncInterval, presentFlags);

			const uint64_t fenceValue = m_CurrentFenceValue;
			m_CommandQueue->Signal(m_Fences[m_CurrentBackBuffer].Get(), fenceValue);
			m_FenceValues[m_CurrentBackBuffer] = fenceValue;
			++m_CurrentFenceValue;
			m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % s_QueueSlotCount;
		}
	}

	void Dx12GraphicsContext::Shutdown()
	{
		for (size_t i = 0; i < s_QueueSlotCount; ++i)
			WaitForFence(m_Fences[i], m_FenceValues[i], m_FenceEvents[i]);

		for (auto e : m_FenceEvents)
			CloseHandle(e);

#ifdef ILLUMINO_DEBUG
		Microsoft::WRL::ComPtr<ID3D12InfoQueue1> infoQueue;
		m_Device.As(&infoQueue);

		infoQueue->UnregisterMessageCallback(s_DebugCallbackCookie);
#endif // ILLUMINO_DEBUG
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
		swapChainDesc.Width = m_Window.GetWidth();
		swapChainDesc.Height = m_Window.GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;


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
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		infoQueue->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &s_DebugCallbackCookie);
#endif // ILLUMINO_DEBUG

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		ILLUMINO_ASSERT(SUCCEEDED(hr), "Failed to create command queue");

		DXGI_SWAP_CHAIN_DESC1 swapChainDescCopy = swapChainDesc;
		hr = dxgiFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_Window
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

		for (size_t i = 0; i < s_QueueSlotCount; ++i)
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

			m_Device->CreateRenderTargetView(m_RenderTargets[i].Get(), &viewDesc, rtvHandle);
			rtvHandle.Offset(m_RenderTargetViewDescriptorSize);
		}
	}

	void Dx12GraphicsContext::CreateAllocatorsAndCommandLists()
	{
		for (size_t i = 0; i < s_QueueSlotCount; ++i)
		{
			m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i]));
			m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[i].Get(), nullptr, IID_PPV_ARGS(&m_CommandLists[i]));
			_bstr_t wc(i);
			m_CommandLists[i]->SetName(wc);
			m_CommandLists[i]->Close();
		}
	}

	void Dx12GraphicsContext::CreateViewportScissor()
	{
		int width = m_Window.GetWidth();
		int height = m_Window.GetHeight();
		m_RectScissor = { 0, 0, width, height };
		m_Viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	}

	void Dx12GraphicsContext::CreateRootSignature()
	{
		CD3DX12_ROOT_PARAMETER parameters[1];
		parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> rootBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

		m_Device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	}

	void Dx12GraphicsContext::CreatePipelineState()
	{
		static const D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		static const D3D_SHADER_MACRO macros[] =
		{
			{ "D3D12_SAMPLE_BASIC", "1" },
			{ nullptr, nullptr }
		};

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
		D3DCompile(Shaders, sizeof(Shaders), "", macros, nullptr, "VS_main", "vs_5_0", 0, 0, &vertexShader, nullptr);

		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
		D3DCompile(Shaders, sizeof(Shaders), "", macros, nullptr, "PS_main", "ps_5_0", 0, 0, &pixelShader, nullptr);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		psoDesc.InputLayout.NumElements = std::extent<decltype(layout)>::value;
		psoDesc.InputLayout.pInputElementDescs = layout;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		auto& renderTarget = psoDesc.BlendState.RenderTarget[0];
		renderTarget.BlendEnable = true;
		renderTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTarget.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		renderTarget.BlendOp = D3D12_BLEND_OP_ADD;
		renderTarget.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTarget.DestBlendAlpha = D3D12_BLEND_ZERO;
		renderTarget.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		psoDesc.SampleDesc.Count = 1;
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DepthStencilState.StencilEnable = false;
		psoDesc.SampleMask = 0xFFFFFFFF;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineStateObject));
	}

	void Dx12GraphicsContext::WaitForFence(
		Microsoft::WRL::ComPtr<ID3D12Fence> fence,
		uint64_t completionValue,
		HANDLE waitEvent)
	{
		if (fence->GetCompletedValue() < completionValue)
		{
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}
}
