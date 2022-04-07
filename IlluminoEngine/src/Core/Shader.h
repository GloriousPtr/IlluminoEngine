#pragma once

namespace IlluminoEngine
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() = 0;

		static Ref<Shader> Create(const std::string& filepath);
	};
}
