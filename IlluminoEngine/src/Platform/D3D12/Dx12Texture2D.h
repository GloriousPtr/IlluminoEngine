#pragma once

#include "Illumino/Renderer/Texture.h"
#include "Dx12Resources.h"

namespace IlluminoEngine
{
	class Dx12Texture2D : public Texture2D
	{
	public:
		Dx12Texture2D(const char* filepath);
		virtual ~Dx12Texture2D() override;

		virtual void Bind(uint32_t slot) override;
		virtual uint64_t GetRendererID() override { return m_Handle.GPU.ptr; }

	private:
		uint32_t m_Width;
		uint32_t m_Height;
		ID3D12Resource*	m_Image;
		ID3D12Resource*	m_UploadImage;
		DescriptorHandle m_Handle;
	};
}
