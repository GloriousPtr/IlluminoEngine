#pragma once

#include <d3d12.h>
#include <EASTL/unordered_map.h>
#include <EASTL/string.h>

#include "Illumino/Renderer/Shader.h"
#include "Illumino/Renderer/GraphicsContext.h"
#include "Dx12Resources.h"

namespace IlluminoEngine
{
	class Dx12Shader : public Shader
	{
	public:
		Dx12Shader(const char* filepath, const BufferLayout& layout);
		virtual ~Dx12Shader() override;

		virtual void BindConstantBuffer(uint32_t slot, uint64_t handle) override;
		virtual void BindStructuredBuffer(uint32_t slot, uint64_t handle) override;
		virtual void BindGlobal(uint32_t slot, uint64_t handle) override;
		virtual void BindPipeline() override;

		virtual uint64_t CreateBuffer(const char* name, size_t sizeAligned) override;
		virtual void UploadBuffer(const char* name, void* data, size_t size, size_t offsetAligned) override;

		virtual uint64_t CreateSRV(const char* name, size_t sizeAligned) override;
		virtual void UploadSRV(const char* name, void* data, size_t size, size_t offsetAligned) override;

	private:
		void SetBufferLayout(const BufferLayout& layout);
		std::string ReadFile(const char* filepath);
		ID3D12Resource* GetConstantBuffer(const char* name);

	private:
		struct BufferData
		{
			size_t Size = 0;
			ID3D12Resource* Resource = nullptr;
			DescriptorHandle Handle = {};
		};

		String m_Filepath;
		ID3D12RootSignature* m_RootSignature;
		ID3D12PipelineState* m_PipelineState;
		eastl::unordered_map<eastl::string, BufferData> m_ConstantBuffers[g_QueueSlotCount];
		eastl::unordered_map<eastl::string, BufferData> m_SRVBuffers[g_QueueSlotCount];
	};
}
