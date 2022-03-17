#include "ipch.h"
#include "Window.h"

namespace IlluminoEngine
{
	LRESULT HandleInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		OPTICK_EVENT();

		auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		auto window = reinterpret_cast<Window*>(ptr);

		switch (uMsg)
		{
			case WM_CLOSE:
				window->OnClosed();
				return 0;
		}

		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}

	Window::Window(const char* name, uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_Closed(false)
	{
		OPTICK_EVENT();

		WNDCLASSA wc = {};
		wc.style = 0;
		wc.lpfnWndProc = &(IlluminoEngine::HandleInput);
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = nullptr;
		wc.hCursor = nullptr;
		wc.hInstance = nullptr;
		wc.hIcon = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = name;
		RegisterClassA(&wc);

		DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER;
		m_Hwnd = CreateWindowA(name, name, style, 0, 0, width, height, nullptr, nullptr, nullptr, nullptr);

		SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		ShowWindow(m_Hwnd, SW_SHOWDEFAULT);
		UpdateWindow(m_Hwnd);
	}

	Window::~Window()
	{
		OPTICK_EVENT();

	}

	void Window::Update()
	{
		OPTICK_EVENT();

		MSG msg = {};
		if (GetMessage(&msg, m_Hwnd, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UpdateWindow(m_Hwnd);
	}

	void Window::OnClosed()
	{
		OPTICK_EVENT();

		m_Closed = true;
	}
}
