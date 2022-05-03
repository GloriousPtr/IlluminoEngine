#pragma once

namespace IlluminoEngine
{
	class Window;
	class MeshBuffer;

	constexpr static uint32_t g_QueueSlotCount = 3;

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void Shutdown() = 0;
		virtual void SetVsync(bool state) = 0;
		virtual void* GetDevice() = 0;
		virtual void* GetCommandQueue() = 0;
		virtual void* GetCommandList() = 0;
		virtual uint32_t GetCurrentBackBufferIndex() = 0;
		virtual void SetDeferredReleasesFlag() = 0;
		virtual void WaitForFence(void* fence, uint64_t completionValue, HANDLE waitEvent) = 0;
		virtual void BindMeshBuffer(MeshBuffer& mesh) = 0;

		static Scope<GraphicsContext> Create(const Window& window);
	};
}
