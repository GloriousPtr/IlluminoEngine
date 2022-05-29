#pragma once

#include "LayerStack.h"

namespace IlluminoEngine
{
	class Window;
	class ImGuiLayer;

	class Application
	{
	public:
		Application();
		virtual ~Application();
		
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void Run();
		Ref<Window>& GetWindow() { return m_Window; }
		
		static Application* GetApplication() { return s_Instance; }

	private:
		static Application* s_Instance;

		std::chrono::steady_clock::time_point m_LastFrameTime;
		Ref<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
	};

	// Should be defined in client
	Application* CreateApplication();
}
