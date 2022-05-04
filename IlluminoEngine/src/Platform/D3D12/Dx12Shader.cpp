#include "ipch.h"
#include "Dx12Shader.h"

#include <fstream>
#include <d3d12.h>
#include <d3dcompiler.h>

#include "d3dx12.h"

#include "Dx12GraphicsContext.h"

namespace IlluminoEngine
{
	Dx12Shader::Dx12Shader(const char* filepath, const BufferLayout& layout)
		: m_Filepath(filepath)
	{
		OPTICK_EVENT();

		SetBufferLayout(layout);
	}

	Dx12Shader::~Dx12Shader()
	{
		OPTICK_EVENT();

		m_PipelineState->Release();
		m_RootSignature->Release();

		for (auto& buffer: m_ConstantBuffers)
		{
			for (auto& b : buffer)
			{
				b.second->Release();
			}
		}
	}

	void Dx12Shader::Bind()
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->BindShader(m_PipelineState, m_RootSignature);
	}

	uint64_t Dx12Shader::CreateBuffer(String&& name, size_t sizeAligned)
	{
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		uint32_t backBuffer = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		auto& constantBufferMap = m_ConstantBuffers[backBuffer];

		if (constantBufferMap.find(name) != constantBufferMap.end())
			return constantBufferMap.at(name)->GetGPUVirtualAddress();

		ID3D12Resource* buffer = Dx12GraphicsContext::s_Context->CreateConstantBuffer(sizeAligned);
		constantBufferMap.emplace(name, buffer);

		return buffer->GetGPUVirtualAddress();
	}

	void Dx12Shader::UploadBuffer(String&& name, void* data, size_t size, size_t offsetAligned)
	{
		OPTICK_EVENT();

		ID3D12Resource* constantBuffer = GetConstantBuffer((String&&)name, size);
		void** p;
		constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&p));
		memcpy(p + offsetAligned, data, size);
		constantBuffer->Unmap(0, nullptr);
	}

	ID3D12Resource* Dx12Shader::GetConstantBuffer(String&& name, size_t size)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		uint32_t backBuffer = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		auto& constantBufferMap = m_ConstantBuffers[backBuffer];

		if (constantBufferMap.find(name) != constantBufferMap.end())
			return constantBufferMap.at(name);

		ID3D12Resource* buffer = Dx12GraphicsContext::s_Context->CreateConstantBuffer(ALIGN(256, size));
		constantBufferMap.emplace(name, buffer);
		return buffer;
	}

	void Dx12Shader::SetBufferLayout(const BufferLayout& layout)
	{
		OPTICK_EVENT();

		std::string source = ReadFile(m_Filepath);

		ID3DBlob* errorBlob;

		ID3DBlob* vertexShader;
		HRESULT hr = D3DCompile(source.c_str(), source.size(), "", nullptr, nullptr, "VS_main", "vs_5_0", 0, 0, &vertexShader, &errorBlob);
		ILLUMINO_ASSERT(SUCCEEDED(hr), (char*) errorBlob->GetBufferPointer());
		if (errorBlob)
			errorBlob->Release();

		ID3DBlob* pixelShader;
		hr = D3DCompile(source.c_str(), source.size(), "", nullptr, nullptr, "PS_main", "ps_5_0", 0, 0, &pixelShader, &errorBlob);
		ILLUMINO_ASSERT(SUCCEEDED(hr), (char*) errorBlob->GetBufferPointer());
		if (errorBlob)
			errorBlob->Release();

		// Create root signature
		D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
		rootCBVDescriptor.RegisterSpace = 0;
		rootCBVDescriptor.ShaderRegister = 0;

		CD3DX12_ROOT_PARAMETER parameters[1];
		parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameters[0].Descriptor = rootCBVDescriptor;
		parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob* rootBlob;
		hr = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);
		ILLUMINO_ASSERT(SUCCEEDED(hr), (char*) errorBlob->GetBufferPointer());
		if (errorBlob)
			errorBlob->Release();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->CreateRootSignature(rootBlob, &m_RootSignature);


		std::vector<D3D12_INPUT_ELEMENT_DESC> d3d12BufferLayout;
		d3d12BufferLayout.reserve(layout.GetCount());
		for (const auto& element: layout)
		{
			DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
			UINT semanticCount = 0;
			switch (element.Type)
			{
				case ShaderDataType::Float:		format = DXGI_FORMAT_R32_FLOAT;
												semanticCount = 1;
												break;
				case ShaderDataType::Float2:	format = DXGI_FORMAT_R32G32_FLOAT;
												semanticCount = 1;
												break;
				case ShaderDataType::Float3:	format = DXGI_FORMAT_R32G32B32_FLOAT;
												semanticCount = 1;
												break;
				case ShaderDataType::Float4:	format = DXGI_FORMAT_R32G32B32A32_FLOAT;
												semanticCount = 1;
												break;
				case ShaderDataType::Mat3:		format = DXGI_FORMAT_R32G32B32_FLOAT;
												semanticCount = 3;
												break;
				case ShaderDataType::Mat4:		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
												semanticCount = 4;
												break;
				case ShaderDataType::Int:		format = DXGI_FORMAT_R32G32_SINT;
												semanticCount = 1;
												break;
				case ShaderDataType::Int2:		format = DXGI_FORMAT_R32G32_SINT;
												semanticCount = 1;
												break;
				case ShaderDataType::Int3:		format = DXGI_FORMAT_R32G32B32_SINT;
												semanticCount = 1;
												break;
				case ShaderDataType::Int4:		format = DXGI_FORMAT_R32G32B32A32_SINT;
												semanticCount = 1;
												break;
				case ShaderDataType::Bool:		format = DXGI_FORMAT_R32_SINT;
												semanticCount = 1;
												break;
			}

			ILLUMINO_ASSERT(format != DXGI_FORMAT_UNKNOWN, "Unknown DXGI_FORMAT type");
			ILLUMINO_ASSERT(semanticCount != 0, "semanticCount cannot be zero");

			auto classification = element.Classification == ShaderDataClassification::Vertex
									? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
									: D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;

			UINT step = static_cast<UINT>(element.Size) / semanticCount;

			for (UINT inputSemantic = 0; inputSemantic < semanticCount; ++inputSemantic)
			{
				UINT offset = static_cast<UINT>(element.Offset) + inputSemantic * step;
				d3d12BufferLayout.push_back({ element.Name, inputSemantic, format, 0, offset, classification, 0 });
			}
		}

		// Create pipeline state object
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		psoDesc.pRootSignature = m_RootSignature;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		psoDesc.InputLayout.NumElements = static_cast<UINT>(d3d12BufferLayout.size());
		psoDesc.InputLayout.pInputElementDescs = &(d3d12BufferLayout[0]);
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

		Dx12GraphicsContext::s_Context->CreatePipelineState(psoDesc, &m_PipelineState);

		rootBlob->Release();
		pixelShader->Release();
		vertexShader->Release();
	}

	std::string Dx12Shader::ReadFile(const char* filepath)
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
