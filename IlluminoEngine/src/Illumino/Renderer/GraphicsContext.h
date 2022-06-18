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

		virtual bool IsVsync() = 0;

		static Scope<GraphicsContext> Create(const Window& window);
	};
}
