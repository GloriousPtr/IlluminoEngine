#pragma once

#include <Windows.h>

#include "Illumino/Renderer/GraphicsContext.h"

namespace IlluminoEngine
{
	class GraphicsContext;

	class Window
	{
	public:
		Window(const char* name, uint32_t width, uint32_t height);
		virtual ~Window();

		void Init();
		void Update();

		bool Minimized() const { return m_Minimized; }
		bool ShouldClose() const { return m_Closed; }
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		HWND GetHwnd() const { return m_Hwnd; }

		const Scope<GraphicsContext>& GetGraphicsContext() { return m_Context; }

	private:
		void ProcessInput();

		static LRESULT HandleInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		std::string m_Name;
		uint32_t m_Width;
		uint32_t m_Height;
		bool m_Minimized;
		bool m_Closed;
		HINSTANCE m_HInstance;
		HWND m_Hwnd;
		Scope<GraphicsContext> m_Context;
	};
}
