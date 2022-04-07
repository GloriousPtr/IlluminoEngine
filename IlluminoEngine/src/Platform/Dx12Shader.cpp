#include "ipch.h"
#include "Dx12Shader.h"

#include <fstream>
#include <wrl.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include "d3dx12.h"

#include "Window.h"
#include "Dx12GraphicsContext.h"

namespace IlluminoEngine
{
	static Dx12GraphicsContext* s_Context = nullptr;

	Dx12Shader::Dx12Shader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
		HRESULT hr = D3DCompile(source.c_str(), source.size(), "", nullptr, nullptr, "VS_main", "vs_5_0", 0, 0, &vertexShader, &errorBlob);
		ILLUMINO_ASSERT(SUCCEEDED(hr), (char*) errorBlob->GetBufferPointer());

		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
		hr = D3DCompile(source.c_str(), source.size(), "", nullptr, nullptr, "PS_main", "ps_5_0", 0, 0, &pixelShader, &errorBlob);
		ILLUMINO_ASSERT(SUCCEEDED(hr), (char*) errorBlob->GetBufferPointer());

		if (!s_Context)
		{
			auto context = Window::GetGraphicsContext().get();
			s_Context = reinterpret_cast<Dx12GraphicsContext*>(context);
		}

		// Create root signature
		CD3DX12_ROOT_PARAMETER parameters[1];
		parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> rootBlob;
		hr = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);
		ILLUMINO_ASSERT(SUCCEEDED(hr), (char*) errorBlob->GetBufferPointer());

		s_Context->CreateRootSignature(rootBlob.Get(), &m_RootSignature);

		// Create buffer layout
		static const D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Create pipeline state object
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		psoDesc.InputLayout.NumElements = std::extent<decltype(layout)>::value;
		psoDesc.InputLayout.pInputElementDescs = layout;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		auto& renderTarget = psoDesc.BlendState.RenderTarget[0];
		renderTarget.BlendEnable = true;
		renderTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTarget.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		renderTarget.BlendOp = D3D12_BLEND_OP_ADD;
		renderTarget.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTarget.DestBlendAlpha = D3D12_BLEND_ZERO;
		renderTarget.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		psoDesc.SampleDesc.Count = 1;
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DepthStencilState.StencilEnable = false;
		psoDesc.SampleMask = 0xFFFFFFFF;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		s_Context->CreatePipelineState(psoDesc, &m_PipelineState);
	}

	void Dx12Shader::Bind()
	{
		s_Context->BindShader(m_PipelineState.Get(), m_RootSignature.Get());
	}

	std::string Dx12Shader::ReadFile(const std::string& filepath)
	{
		OPTICK_EVENT();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			const size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				ILLUMINO_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			ILLUMINO_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}
}
