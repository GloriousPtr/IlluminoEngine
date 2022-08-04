#include "ipch.h"
#include "Dx12Texture2D.h"

#include <d3d12.h>
#include <dxgi.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "d3dx12.h"

#include "Dx12GraphicsContext.h"
#include "Illumino/Utils/StringUtils.h"

namespace IlluminoEngine
{
	Dx12Texture2D::Dx12Texture2D(const char* filepath)
	{
		OPTICK_EVENT();

		int width, height, channels;
		
		stbi_uc* data = nullptr;
		{
			OPTICK_EVENT("stbi_load Texture");

			data = stbi_load(filepath, &width, &height, &channels, 4);
		}
		ILLUMINO_ASSERT(data, "Failed to load image!");

		LoadTexture(width, height, data);
		
		stbi_image_free(data);
	}

	Dx12Texture2D::Dx12Texture2D(uint32_t width, uint32_t height, void* data)
	{
		OPTICK_EVENT();

		LoadTexture(width, height, data);
	}

	Dx12Texture2D::~Dx12Texture2D()
	{
		OPTICK_EVENT();

		Dx12GraphicsContext::s_Context->WaitForAllFrames();

		m_Image->Release();
		m_UploadImage->Release();

		Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Free(m_Handle);
	}

	void Dx12Texture2D::Bind(uint32_t slot)
	{
		OPTICK_EVENT();

		Dx12GraphicsContext::s_Context->GetCommandList()->SetGraphicsRootDescriptorTable(slot, m_Handle.GPU);
	}

	void Dx12Texture2D::LoadTexture(uint32_t width, uint32_t height, void* data)
	{
		OPTICK_EVENT();

		m_Width = width;
		m_Height = height;

		static const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);

		Dx12GraphicsContext::s_Context->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Image));

		static const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto uploadBufferSize = GetRequiredIntermediateSize(m_Image, 0, 1);
		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		Dx12GraphicsContext::s_Context->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadImage));

		D3D12_SUBRESOURCE_DATA srcData;
		srcData.pData = data;
		srcData.RowPitch = width * 4;
		srcData.SlicePitch = width * height * 4;

		ID3D12GraphicsCommandList* commandList = Dx12GraphicsContext::s_Context->GetCommandList();
		UpdateSubresources(commandList, m_Image, m_UploadImage, 0, 0, 1, &srcData);
		const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_Image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		m_Handle = Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Allocate();
		Dx12GraphicsContext::s_Context->GetDevice()->CreateShaderResourceView(m_Image, &shaderResourceViewDesc, m_Handle.CPU);
	}
}
