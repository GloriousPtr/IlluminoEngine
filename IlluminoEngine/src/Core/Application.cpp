#include "ipch.h"
#include "Application.h"

#include "Window.h"

namespace IlluminoEngine
{
	Application::Application()
	{
		OPTICK_EVENT();

		ILLUMINO_INFO("Application Started");
		m_Window = CreateRef<Window>("Illumino Engine", 1920, 1080);
	}

	Application::~Application()
	{
		OPTICK_EVENT();

		ILLUMINO_INFO("Application Ended");
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			OPTICK_FRAME("MainThread");

			m_Window->Update();
		}
	}
}
