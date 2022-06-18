#pragma once

#include <d3d12.h>

#include "Illumino/Core/Core.h"
#include "Illumino/Renderer/GraphicsContext.h"

namespace IlluminoEngine
{
	class DescriptorHeap;

	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CPU;
		D3D12_GPU_DESCRIPTOR_HANDLE GPU;

		const bool IsValid() const { return CPU.ptr != 0; }
		const bool IsShaderVisible() const { return GPU.ptr != 0; }

#ifdef ILLUMINO_DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* m_Container = nullptr;
		uint32_t m_Index = 0;
#endif // ILLUMINO_DEBUG

	};

	class DescriptorHeap
	{
	public:
		DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
			: m_Type(type)
		{
		}

		~DescriptorHeap()
		{
		}

		DescriptorHeap(const DescriptorHeap& other) = delete;
		DescriptorHeap(DescriptorHeap&& other) = delete;
		
		bool Init(uint32_t capacity, bool isShaderVisible);
		void ProcessDeferredFree(uint32_t frameIndex);
		void Release();

		DescriptorHandle Allocate();
		void Free(DescriptorHandle handle);

		const D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_Type; }
		const D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStart() const { return m_CPUStart; }
		const D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStart() const { return m_GPUStart; }
		ID3D12DescriptorHeap* const GetHeap() const { return m_Heap; }
		const uint32_t GetCapacity() const { return m_Capacity; }
		const uint32_t GetSize() const { return m_Size; }
		const uint32_t GetDescriptorSize() const { return m_DescriptorSize; }
		const bool IsShaderVisible() const { return m_GPUStart.ptr != 0; }

	private:
		ID3D12DescriptorHeap* m_Heap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUStart;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUStart;
		Scope<uint32_t[]> m_FreeHandles;
		std::vector<uint32_t> m_DeferredFreeIndices[g_QueueSlotCount];
		uint32_t m_Capacity = 0;
		uint32_t m_Size = 0;
		uint32_t m_DescriptorSize = 0;
		const D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	};
}
