#pragma once

#include "Core/Shader.h"

#include <wrl.h>
#include <d3d12.h>

namespace IlluminoEngine
{
	class Dx12Shader : public Shader
	{
	public:
		Dx12Shader(const std::string& filepath);

		virtual void Bind() override;

	private:
		std::string ReadFile(const std::string& filepath);

	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	};
}
