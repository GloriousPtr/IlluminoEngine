#include "ipch.h"
#include "Application.h"

#include "Window.h"

namespace IlluminoEngine
{
	Application::Application()
	{
		ILLUMINO_INFO("Application Started");
		m_Window = CreateRef<Window>("Illumino Engine", 1920, 1080);
	}

	Application::~Application()
	{
		ILLUMINO_INFO("Application Ended");
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			m_Window->Update();
		}
	}
}
