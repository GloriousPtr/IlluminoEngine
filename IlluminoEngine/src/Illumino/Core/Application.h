#pragma once

namespace IlluminoEngine
{
	class Window;
	class ImGuiLayer;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		Ref<Window>& GetWindow() { return m_Window; }
		
		static Application* GetApplication() { return s_Instance; }

	private:
		static Application* s_Instance;

		Ref<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
	};

	// Should be defined in client
	Application* CreateApplication();
}
