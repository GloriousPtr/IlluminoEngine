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

		Dx12GraphicsContext::s_Context->WaitForAllFrames();

		m_PipelineState->Release();
		m_RootSignature->Release();

		for (auto& buffer: m_ConstantBuffers)
		{
			for (auto& [name, b] : buffer)
			{
				b.Size = 0;
				b.Resource->Release();
			}
		}
		m_ConstantBuffers->clear();

		for (auto& buffer : m_SRVBuffers)
		{
			for (auto& [name, b] : buffer)
			{
				b.Size = 0;
				b.Resource->Release();

				Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Free(b.Handle);
			}
		}

		m_SRVBuffers->clear();
	}

	void Dx12Shader::BindConstantBuffer(uint32_t slot, uint64_t handle)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->GetCommandList()->SetGraphicsRootConstantBufferView(slot, handle);
	}

	void Dx12Shader::BindStructuredBuffer(uint32_t slot, uint64_t handle)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->GetCommandList()->SetGraphicsRootDescriptorTable(slot, { handle });
	}

	void Dx12Shader::BindGlobal(uint32_t slot, uint64_t handle)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->GetCommandList()->SetGraphicsRootUnorderedAccessView(slot, handle);
	}

	void Dx12Shader::BindPipeline()
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->BindShader(m_PipelineState, m_RootSignature);
	}

	uint64_t Dx12Shader::CreateBuffer(const char* name, size_t sizeAligned)
	{
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		uint32_t backBuffer = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		auto& constantBufferMap = m_ConstantBuffers[backBuffer];

		if (constantBufferMap.find_as(name) != constantBufferMap.end())
		{
			auto& cb = constantBufferMap.at(name);
			if (cb.Size == sizeAligned)
				return cb.Resource->GetGPUVirtualAddress();
			else
				cb.Resource->Release();
		}

		ID3D12Resource* buffer = Dx12GraphicsContext::s_Context->CreateConstantBuffer(sizeAligned);
		constantBufferMap[name] = { sizeAligned, buffer };

		return buffer->GetGPUVirtualAddress();
	}

	uint64_t Dx12Shader::CreateSRV(const char* name, size_t size)
	{
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		uint32_t backBuffer = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		auto& srvBufferMap = m_SRVBuffers[backBuffer];
		if (srvBufferMap.find_as(name) != srvBufferMap.end())
		{
			auto& s = srvBufferMap.at(name);
			if (s.Size == size)
			{
				return s.Handle.GPU.ptr;
			}
			else
			{
				s.Resource->Release();
				Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Free(s.Handle);
			}
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		shaderResourceViewDesc.Buffer.FirstElement = 0;
		shaderResourceViewDesc.Buffer.NumElements = 1;
		shaderResourceViewDesc.Buffer.StructureByteStride = size;
		shaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CreationNodeMask = 1;
		heapDesc.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Width = size;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* ret = nullptr;
		HRESULT hr = Dx12GraphicsContext::s_Context->GetDevice()->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&ret));

		DescriptorHandle handle = Dx12GraphicsContext::s_Context->GetSRVDescriptorHeap().Allocate();
		Dx12GraphicsContext::s_Context->GetDevice()->CreateShaderResourceView(ret, &shaderResourceViewDesc, handle.CPU);
		
		srvBufferMap[name] = { size, ret, handle };

		return handle.GPU.ptr;
	}

	void Dx12Shader::UploadBuffer(const char* name, void* data, size_t size, size_t offsetAligned)
	{
		OPTICK_EVENT();

		ID3D12Resource* constantBuffer = GetConstantBuffer((String&&)name);
		void** p;
		constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&p));
		memcpy(p + offsetAligned, data, size);
		constantBuffer->Unmap(0, nullptr);
	}

	void Dx12Shader::UploadSRV(const char* name, void* data, size_t size, size_t offsetAligned)
	{
		OPTICK_EVENT();

		uint32_t backBuffer = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		ID3D12Resource* srvBuffer = m_SRVBuffers[backBuffer].at(name).Resource;
		void** p;
		srvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&p));
		memcpy(p + offsetAligned, data, size);
		srvBuffer->Unmap(0, nullptr);
	}

	ID3D12Resource* Dx12Shader::GetConstantBuffer(const char* name)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		uint32_t backBuffer = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		auto& constantBufferMap = m_ConstantBuffers[backBuffer];

		if (constantBufferMap.find_as(name) != constantBufferMap.end())
			return constantBufferMap.at(name).Resource;

		ILLUMINO_ASSERT("Constant buffer not found!");
		return nullptr;
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
		CD3DX12_ROOT_PARAMETER parameters[6];
		CD3DX12_DESCRIPTOR_RANGE range1 { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 };
		CD3DX12_DESCRIPTOR_RANGE range2 { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1 };
		CD3DX12_DESCRIPTOR_RANGE range3 { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2 };

		parameters[0].InitAsDescriptorTable(1, &range1);
		parameters[1].InitAsDescriptorTable(1, &range2);
		parameters[2].InitAsDescriptorTable(1, &range3);
		parameters[3].InitAsConstantBufferView(0, 0);
		parameters[4].InitAsConstantBufferView(1, 0);
		parameters[5].InitAsConstantBufferView(2, 0);

		CD3DX12_STATIC_SAMPLER_DESC samplers[1];
		samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		
		descRootSignature.Init(6, parameters, 1, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthEnable = true;
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
