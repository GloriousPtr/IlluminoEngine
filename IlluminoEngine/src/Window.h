#pragma once

#include <Windows.h>
#include <Core/GraphicsContext.h>

namespace IlluminoEngine
{
	class Window
	{
	public:
		Window(const char* name, uint32_t width, uint32_t height);
		virtual ~Window();

		void Update();
		void OnClosed();

		bool ShouldClose() { return m_Closed; }
		uint32_t GetWidth() { return m_Width; }
		uint32_t GetHeight() { return m_Height; }
		HWND GetHwnd() { return m_Hwnd; }

	private:
		std::string m_Name;
		uint32_t m_Width;
		uint32_t m_Height;
		bool m_Closed;
		HINSTANCE m_HInstance;
		HWND m_Hwnd;
		Scope<GraphicsContext> m_Context;
	};
}
