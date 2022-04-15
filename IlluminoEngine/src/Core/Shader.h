#pragma once

#include "Buffer.h"

namespace IlluminoEngine
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() = 0;

		virtual void UploadFloat(String&& name, float value) = 0;

		static Ref<Shader> Create(const char* filepath, const BufferLayout& layout);
	};
}