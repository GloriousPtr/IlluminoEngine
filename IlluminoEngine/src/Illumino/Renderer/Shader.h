#pragma once

#include "Buffer.h"

namespace IlluminoEngine
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() = 0;

		virtual void* CreateBuffer(String&& name, size_t sizeAligned) = 0;
		virtual void UploadBuffer(String&& name, void* data, size_t size, size_t offsetAligned) = 0;

		static Ref<Shader> Create(const char* filepath, const BufferLayout& layout);
	};
}
