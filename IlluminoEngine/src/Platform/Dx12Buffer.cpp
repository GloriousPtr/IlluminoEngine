#include "ipch.h"
#include "Dx12Buffer.h"

#include "Window.h"
#include "Core/GraphicsContext.h"

namespace IlluminoEngine
{
	Dx12MeshBuffer::Dx12MeshBuffer(float* vertexData, uint32_t* indexData, size_t verticesSize, size_t indicesSize, size_t strideSize)
	{
		auto& context = Window::GetGraphicsContext();
		
		ID3D12Device* device = (ID3D12Device*) context->GetDevice();
		ID3D12CommandQueue* commandQueue = (ID3D12CommandQueue*) context->GetCommandQueue();

		// Create our upload fence, command list and command allocator
		// This will be only used while creating the mesh buffer and the texture
		// to upload data to the GPU.
		ID3D12Fence* uploadFence;
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence));

		ID3D12CommandAllocator* uploadCommandAllocator;
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadCommandAllocator));
		ID3D12GraphicsCommandList* uploadCommandList;
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
									uploadCommandAllocator, nullptr,
									IID_PPV_ARGS(&uploadCommandList));

		const size_t uploadBufferSize = verticesSize + indicesSize;
		const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		// Create upload buffer on CPU
		device->CreateCommittedResource(&uploadHeapProperties,
											D3D12_HEAP_FLAG_NONE,
											&uploadBufferDesc,
											D3D12_RESOURCE_STATE_GENERIC_READ,
											nullptr,
											IID_PPV_ARGS(&m_UploadBuffer));

		// Create vertex & index buffer on the GPU
		// HEAP_TYPE_DEFAULT is on GPU, we also initialize with COPY_DEST state
		// so we don't have to transition into this before copying into them
		const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		const auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesSize);
		device->CreateCommittedResource(&defaultHeapProperties,
											D3D12_HEAP_FLAG_NONE,
											&vertexBufferDesc,
											D3D12_RESOURCE_STATE_COPY_DEST,
											nullptr,
											IID_PPV_ARGS(&m_VertexBuffer));

		const auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indicesSize);
		device->CreateCommittedResource(&defaultHeapProperties,
											D3D12_HEAP_FLAG_NONE,
											&indexBufferDesc,
											D3D12_RESOURCE_STATE_COPY_DEST,
											nullptr,
											IID_PPV_ARGS(&m_IndexBuffer));

		// Create buffer views
		m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView.SizeInBytes = static_cast<UINT>(verticesSize);
		m_VertexBufferView.StrideInBytes = static_cast<UINT>(strideSize);

		m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.SizeInBytes = static_cast<UINT>(indicesSize);
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

		// Copy data on CPU into the upload buffer
		void* p;
		m_UploadBuffer->Map(0, nullptr, &p);
		memcpy(p, vertexData, verticesSize);
		memcpy(static_cast<unsigned char*>(p) + verticesSize, indexData, indicesSize);
		m_UploadBuffer->Unmap(0, nullptr);

		// Copy data from upload buffer on CPU into the index/vertex buffer on 
		// the GPU
		uploadCommandList->CopyBufferRegion(m_VertexBuffer, 0, m_UploadBuffer, 0, verticesSize);
		uploadCommandList->CopyBufferRegion(m_IndexBuffer, 0, m_UploadBuffer, verticesSize, indicesSize);

		// Barriers, batch them together
		const CD3DX12_RESOURCE_BARRIER barriers[2] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer,
												D3D12_RESOURCE_STATE_COPY_DEST,
												D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
			CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer,
												D3D12_RESOURCE_STATE_COPY_DEST,
												D3D12_RESOURCE_STATE_INDEX_BUFFER)
		};

		uploadCommandList->ResourceBarrier(2, barriers);

		uploadCommandList->Close();

		// Execute the upload and finish the command list
		ID3D12CommandList* commandLists [] = { uploadCommandList };
		commandQueue->ExecuteCommandLists(1, commandLists);
		commandQueue->Signal(uploadFence, 1);

		auto waitEvent = CreateEvent(nullptr, false, false, nullptr);

		ILLUMINO_ASSERT(waitEvent != nullptr, "Could not create wait event");

		context->WaitForFence(uploadFence, 1, waitEvent);

		// Cleanup our upload handle
		uploadCommandAllocator->Reset();

		CloseHandle(waitEvent);

		uploadCommandList->Release();
		uploadCommandAllocator->Release();
		uploadFence->Release();
	}

	Dx12MeshBuffer::~Dx12MeshBuffer()
	{
		m_IndexBuffer->Release();
		m_VertexBuffer->Release();
		m_UploadBuffer->Release();
	}

	void Dx12MeshBuffer::Bind()
	{
		Window::GetGraphicsContext()->BindMeshBuffer(*this);
	}
}
