#include "ipch.h"
#include "Dx12Resources.h"

#include "Dx12GraphicsContext.h"

namespace IlluminoEngine
{
	bool DescriptorHeap::Init(uint32_t capacity, bool isShaderVisible)
	{
		OPTICK_EVENT();

		std::lock_guard lock{ m_Mutex };

		ILLUMINO_ASSERT((capacity & capacity) < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2, "Capacity is either 0 or too high!");
		ILLUMINO_ASSERT(!(m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		Dx12GraphicsContext* context = Dx12GraphicsContext::s_Context;
		ILLUMINO_ASSERT(context);

		if (m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
			m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			isShaderVisible = false;

		if (m_Heap)
			Release();

		ID3D12Device* device = context->GetDevice();
		ILLUMINO_ASSERT(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible
			? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = m_Type;
		desc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));
		if (FAILED(hr))
			return false;

		m_FreeHandles = CreateScope<uint32_t[]>(capacity);
		m_Capacity = capacity;
		m_Size = 0;

		for (uint32_t i = 0; i < capacity; ++i)
			m_FreeHandles[i] = i;

#ifdef ILLUMINO_DEBUG
		for (uint32_t i = 0; i < g_QueueSlotCount; ++i)
			ILLUMINO_ASSERT(m_DeferredFreeIndices[i].empty());
#endif // ILLUMINO_DEBUG

		m_DescriptorSize = device->GetDescriptorHandleIncrementSize(m_Type);
		m_CPUStart = m_Heap->GetCPUDescriptorHandleForHeapStart();
		m_GPUStart = isShaderVisible ?
			m_Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void DescriptorHeap::ProcessDeferredFree(uint32_t frameIndex)
	{
		std::lock_guard lock{ m_Mutex };

		ILLUMINO_ASSERT(frameIndex < g_QueueSlotCount);

		std::vector<uint32_t>& indices = m_DeferredFreeIndices[frameIndex];
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				--m_Size;
				m_FreeHandles[m_Size] = index;
			}
			indices.clear();
		}
	}

	void DescriptorHeap::Release()
	{
		OPTICK_EVENT();

		// Releasing the buffer of size 0 -> releasing the heap memory aquired by the buffer
		ILLUMINO_ASSERT(m_Size >= 0);
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);

		Dx12GraphicsContext::s_Context->DeferredRelease(m_Heap);
	}

	DescriptorHandle DescriptorHeap::Allocate()
	{
		OPTICK_EVENT();

		std::lock_guard lock{ m_Mutex };

		ILLUMINO_ASSERT(m_Heap);
		ILLUMINO_ASSERT(m_Size < m_Capacity);

		const uint32_t index = m_FreeHandles[m_Size];
		const uint32_t offset = index * m_DescriptorSize;
		++m_Size;

		DescriptorHandle handle;
		handle.CPU.ptr = m_CPUStart.ptr + offset;
		if(IsShaderVisible())
			handle.GPU.ptr = m_GPUStart.ptr + offset;

#ifdef ILLUMINO_DEBUG
		handle.m_Container = this;
		handle.m_Index = index;
#endif // ILLUMINO_DEBUG

		return handle;
	}

	void DescriptorHeap::Free(DescriptorHandle handle)
	{
		OPTICK_EVENT();

		if (!handle.IsValid())
			return;

		std::lock_guard lock{ m_Mutex };

		ILLUMINO_ASSERT(m_Heap && m_Size);
		ILLUMINO_ASSERT(handle.m_Container == this);
		ILLUMINO_ASSERT(handle.CPU.ptr >= m_CPUStart.ptr);
		ILLUMINO_ASSERT((handle.CPU.ptr - m_CPUStart.ptr) % m_DescriptorSize == 0);
		ILLUMINO_ASSERT(handle.m_Index < m_Capacity);
		
		const uint32_t index = (uint32_t) (handle.CPU.ptr - m_CPUStart.ptr) / m_DescriptorSize;
		
		ILLUMINO_ASSERT(handle.m_Index == index);
		ILLUMINO_ASSERT(Dx12GraphicsContext::s_Context);
		
		const uint32_t frameIndex = Dx12GraphicsContext::s_Context->GetCurrentBackBufferIndex();
		m_DeferredFreeIndices[frameIndex].push_back(index);
		Dx12GraphicsContext::s_Context->SetDeferredReleasesFlag();

		handle = {};
	}
}
