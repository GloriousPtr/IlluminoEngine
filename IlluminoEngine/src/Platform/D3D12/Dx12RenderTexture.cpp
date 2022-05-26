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
			data.State = D3D12_RESOURCE_STATE_COMMON;
			data.SRVHandle = Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Allocate();
			data.RTVHandle = Dx12GraphicsContext::s_Context->GetRTVDescriptorHeap().Allocate();
		}

		Resize(m_Specification.Width, m_Specification.Height);
	}

	Dx12RenderTexture::~Dx12RenderTexture()
	{
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
			m_RenderTargets[i].Resource->Release();
	}

	void Dx12RenderTexture::Resize(size_t width, size_t height)
	{
		Dx12GraphicsContext::s_Context->WaitForAllFrames();

		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			auto& data = m_RenderTargets[i];
			data.State = D3D12_RESOURCE_STATE_COMMON;
			if (data.Resource)
				data.Resource->Release();
		}

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
			m_Format,
			(UINT)width, (UINT)height,
			1, 1, m_Specification.Samples, 0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		D3D12_CLEAR_VALUE clearValue = { m_Format, {} };
		memcpy(clearValue.Color, glm::value_ptr(m_ClearColor), sizeof(clearValue.Color));

		ID3D12Device* device = Dx12GraphicsContext::s_Context->GetDevice();

		// Create a render target
		for (size_t i = 0; i < g_QueueSlotCount; ++i)
		{
			auto& data = m_RenderTargets[i];
			data.State = D3D12_RESOURCE_STATE_RENDER_TARGET;
			device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &desc, data.State, &clearValue, IID_PPV_ARGS(&data.Resource));
			device->CreateRenderTargetView(data.Resource, nullptr, data.RTVHandle.CPU);
			device->CreateShaderResourceView(data.Resource, nullptr, data.SRVHandle.CPU);

			data.Resource->SetName(L"RenderTexture");
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
	}

	void Dx12RenderTexture::Bind()
	{
		TransitionTo(D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto& data = m_RenderTargets[Dx12GraphicsContext::s_Context->m_CurrentBackBuffer];

		auto commandList = Dx12GraphicsContext::s_Context->GetCommandList();
		commandList->OMSetRenderTargets(1, &data.RTVHandle.CPU, false, nullptr);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_Specification.Width, (float)m_Specification.Height, 0.0f, 1.0f };
		D3D12_RECT scissorRect = { 0, 0, m_Specification.Width, m_Specification.Height };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
	}

	void Dx12RenderTexture::Unbind()
	{
		TransitionTo(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		auto commandList = Dx12GraphicsContext::s_Context->GetCommandList();
		auto rtv = Dx12GraphicsContext::s_Context->m_RenderSurface->GetRTV();
		auto width = Dx12GraphicsContext::s_Context->m_RenderSurface->GetWidth();
		auto height = Dx12GraphicsContext::s_Context->m_RenderSurface->GetHeight();

		commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
		D3D12_RECT scissorRect = { 0, 0, width, height };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		TransitionTo(D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	uint64_t Dx12RenderTexture::GetRendererID()
	{
		return m_RenderTargets[Dx12GraphicsContext::s_Context->m_CurrentBackBuffer].SRVHandle.GPU.ptr;
	}

	void Dx12RenderTexture::TransitionTo(D3D12_RESOURCE_STATES afterState)
	{
		auto& data = m_RenderTargets[Dx12GraphicsContext::s_Context->m_CurrentBackBuffer];

		if (data.State == afterState)
			return;

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = data.Resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = data.State;
		barrier.Transition.StateAfter = afterState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Dx12GraphicsContext::s_Context->GetCommandList()->ResourceBarrier(1, &barrier);

		data.State = afterState;
	}
}
