#pragma once

#include "Buffer.h"

namespace IlluminoEngine
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void BindConstantBuffer(uint32_t slot, uint64_t handle) = 0;
		virtual void BindStructuredBuffer(uint32_t slot, uint64_t handle) = 0;
		virtual void BindGlobal(uint32_t slot, uint64_t handle) = 0;
		virtual void BindPipeline() = 0;

		virtual uint64_t CreateBuffer(const char* name, size_t sizeAligned) = 0;
		virtual void UploadBuffer(const char* name, void* data, size_t size, size_t offsetAligned) = 0;

		virtual uint64_t CreateSRV(const char* name, size_t sizeAligned) = 0;
		virtual void UploadSRV(const char* name, void* data, size_t size, size_t offsetAligned) = 0;

		static Ref<Shader> Create(const char* filepath, const BufferLayout& layout);
	};
}
