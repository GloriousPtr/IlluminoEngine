#include "ipch.h"
#include "Application.h"

#include "Window.h"

namespace IlluminoEngine
{
	Application::Application()
	{
		m_Window = new Window("Illumino Engine", 1920, 1080);
	}

	Application::~Application()
	{
		delete m_Window;
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			m_Window->Update();
		}
	}
}
