#include "ipch.h"
#include "Dx12RenderTexture.h"

#include "d3dx12.h"
#include "Dx12GraphicsContext.h"
#include <glm/gtc/type_ptr.hpp>

namespace IlluminoEngine
{
	Dx12RenderTexture::Dx12RenderTexture(RenderTextureSpec spec) :
		m_Format(DXGI_FORMAT_R8G8B8A8_UNORM),
		m_Specification(spec)
	{
		ID3D12Device* device = Dx12GraphicsContext::s_Context->GetDevice();

		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
			if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
			{
				ILLUMINO_ERROR("CheckFeatureSupport Failed");
				return;
			}

			UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
			if ((formatSupport.Support1 & required) != required)
			{
				#ifdef _DEBUG
				char buff[128] = {};
				sprintf_s(buff, "RenderTexture: Device does not support the requested format (%u)!\n", m_Format);
				ILLUMINO_ERROR(buff);
				#endif
				ILLUMINO_ERROR("RenderTexture Failed");
				return;
			}
		}

		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			auto& data = m_RenderTargets[i];
			data.ColorState = D3D12_RESOURCE_STATE_COMMON;
			data.DepthState = D3D12_RESOURCE_STATE_COMMON;
			data.SRVHandle = Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Allocate();
			data.DSVHandle = Dx12GraphicsContext::s_Context->GetDSVDescriptorHeap().Allocate();
			data.RTVHandle = Dx12GraphicsContext::s_Context->GetRTVDescriptorHeap().Allocate();
		}

		Resize(m_Specification.Width, m_Specification.Height);
	}

	Dx12RenderTexture::~Dx12RenderTexture()
	{
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			m_RenderTargets[i].ColorResource->Release();
			m_RenderTargets[i].DepthResource->Release();
		}
	}

	void Dx12RenderTexture::Resize(size_t width, size_t height)
	{
		Dx12GraphicsContext::s_Context->WaitForAllFrames();

		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			auto& data = m_RenderTargets[i];
			data.ColorState = D3D12_RESOURCE_STATE_COMMON;
			data.DepthState = D3D12_RESOURCE_STATE_COMMON;
			if (data.ColorResource)
				data.ColorResource->Release();
			if (data.DepthResource)
				data.DepthResource->Release();
		}

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			m_Format,
			(UINT)width, (UINT)height,
			1, 1, m_Specification.Samples, 0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		D3D12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT,
			(UINT)width, (UINT)height,
			1, 1, m_Specification.Samples, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv_desc.Texture2D.MipSlice = 0;
		dsv_desc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE clearValue = { m_Format, {} };
		memcpy(clearValue.Color, glm::value_ptr(m_ClearColor), sizeof(clearValue.Color));

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = m_ClearDepth.r;
		depthOptimizedClearValue.DepthStencil.Stencil = m_ClearDepth.g;

		ID3D12Device* device = Dx12GraphicsContext::s_Context->GetDevice();

		// Create a render target
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			auto& data = m_RenderTargets[i];

			data.ColorState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &colorDesc, data.ColorState, &clearValue, IID_PPV_ARGS(&data.ColorResource));
			device->CreateRenderTargetView(data.ColorResource, nullptr, data.RTVHandle.CPU);
			device->CreateShaderResourceView(data.ColorResource, nullptr, data.SRVHandle.CPU);

			data.DepthState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &depthDesc, data.DepthState, &depthOptimizedClearValue, IID_PPV_ARGS(&data.DepthResource));
			device->CreateDepthStencilView(data.DepthResource, &dsv_desc, data.DSVHandle.CPU);

			data.ColorResource->SetName(L"RenderTexture");
			data.DepthResource->SetName(L"DepthTexture");
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
	}

	static void TransitionTo(D3D12_RESOURCE_STATES& state, const D3D12_RESOURCE_STATES afterState, ID3D12Resource* resource, ID3D12GraphicsCommandList* commandList)
	{
		if (state == afterState)
			return;

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = state;
		barrier.Transition.StateAfter = afterState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);

		state = afterState;
	}

	void Dx12RenderTexture::Bind()
	{
		auto& data = m_RenderTargets[Dx12GraphicsContext::s_Context->m_CurrentBackBuffer];

		auto commandList = Dx12GraphicsContext::s_Context->GetCommandList();
		commandList->OMSetRenderTargets(1, &data.RTVHandle.CPU, true, &data.DSVHandle.CPU);
		commandList->ClearRenderTargetView(data.RTVHandle.CPU, glm::value_ptr(m_ClearColor), 0, nullptr);

		D3D12_CLEAR_VALUE depthClearValue = { DXGI_FORMAT_D32_FLOAT, {} };
		memset(depthClearValue.Color, 1.0f, sizeof(depthClearValue.Color));
		commandList->ClearDepthStencilView(data.DSVHandle.CPU, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, m_ClearDepth.r, m_ClearDepth.g, 0, nullptr);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_Specification.Width, (float)m_Specification.Height, 0.0f, 1.0f };
		D3D12_RECT scissorRect = { 0, 0, m_Specification.Width, m_Specification.Height };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		TransitionTo(data.ColorState, D3D12_RESOURCE_STATE_RENDER_TARGET, data.ColorResource, commandList);
		TransitionTo(data.DepthState, D3D12_RESOURCE_STATE_DEPTH_WRITE, data.DepthResource, commandList);
	}

	void Dx12RenderTexture::Unbind()
	{
		auto& data = m_RenderTargets[Dx12GraphicsContext::s_Context->m_CurrentBackBuffer];
		auto commandList = Dx12GraphicsContext::s_Context->GetCommandList();
		
		TransitionTo(data.ColorState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, data.ColorResource, commandList);
		TransitionTo(data.DepthState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, data.DepthResource, commandList);
		
		auto rtv = Dx12GraphicsContext::s_Context->m_RenderSurface->GetRTV();
		auto width = Dx12GraphicsContext::s_Context->m_RenderSurface->GetWidth();
		auto height = Dx12GraphicsContext::s_Context->m_RenderSurface->GetHeight();

		commandList->OMSetRenderTargets(1, &rtv, true, nullptr);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
		D3D12_RECT scissorRect = { 0, 0, width, height };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		TransitionTo(data.ColorState, D3D12_RESOURCE_STATE_RENDER_TARGET, data.ColorResource, commandList);
		TransitionTo(data.DepthState, D3D12_RESOURCE_STATE_DEPTH_WRITE, data.DepthResource, commandList);
	}

	uint64_t Dx12RenderTexture::GetRendererID()
	{
		return m_RenderTargets[Dx12GraphicsContext::s_Context->m_CurrentBackBuffer].SRVHandle.GPU.ptr;
	}
}
