#include "ipch.h"
#include "Window.h"

#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace IlluminoEngine
{
	LRESULT HandleInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		OPTICK_EVENT();

		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		auto window = reinterpret_cast<Window*>(ptr);

		switch (uMsg)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
		}

		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}

	Window::Window(const char* name, uint32_t width, uint32_t height)
		: m_Name(name), m_Width(width), m_Height(height), m_Closed(false), m_HInstance(GetModuleHandle(nullptr))
	{
		OPTICK_EVENT();

		WNDCLASSA wc = {};
		wc.style = 0;
		wc.lpfnWndProc = &(IlluminoEngine::HandleInput);
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_HInstance;
		wc.lpszClassName = name;
		wc.lpszMenuName = nullptr;
		wc.hbrBackground = nullptr;
		wc.hCursor = nullptr;
		wc.hIcon = nullptr;
		bool success = RegisterClassA(&wc);
		ILLUMINO_ASSERT(success, "Failed to register window class");

		DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER;

		RECT rect;
		rect.left = 100;
		rect.right = width + rect.left;
		rect.top = 100;
		rect.bottom = height + rect.top;
		AdjustWindowRect(&rect, style, false);

		m_Hwnd = CreateWindowA(name, name, style,
			100, 100, rect.right - rect.left, rect.bottom - rect.top,
			nullptr, nullptr, m_HInstance, nullptr);

		SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		ShowWindow(m_Hwnd, SW_SHOWDEFAULT);
		UpdateWindow(m_Hwnd);

		m_Context = GraphicsContext::Create(*this);
	}

	Window::~Window()
	{
		OPTICK_EVENT();

		m_Context->Shutdown();
		UnregisterClassA(m_Name.c_str(), m_HInstance);
	}

	void Window::Init()
	{
		m_Context->Init();
	}

	void Window::Update()
	{
		OPTICK_EVENT();

		m_Context->SwapBuffers();
	}

	void Window::ProcessInput()
	{
		OPTICK_EVENT();

		MSG msg = {};
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				OnClosed();
		}
	}

	void Window::OnClosed()
	{
		OPTICK_EVENT();

		if (m_Closed)
			return;

		m_Closed = true;
	}
}
