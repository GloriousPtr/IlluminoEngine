#include "ipch.h"
#include "Application.h"

#include "Window.h"
#include "Timestep.h"
#include "Illumino/ImGui/ImGuiLayer.h"
#include "Illumino/Renderer/RenderCommand.h"
#include "Illumino/Renderer/SceneRenderer.h"

namespace IlluminoEngine
{
	Application* Application::s_Instance;

	Application::Application()
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		ILLUMINO_INFO("Application Started");
		m_Window = CreateRef<Window>("Illumino Engine", 1920, 1080);
		m_Window->Init();
		RenderCommand::Init();
		SceneRenderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		OPTICK_EVENT();

		m_Window->Update();

		SceneRenderer::Shutdown();
		m_LayerStack.PopOverlay(m_ImGuiLayer);
		delete m_ImGuiLayer;

		ILLUMINO_INFO("Application Ended");

		OPTICK_SHUTDOWN();
	}

	void Application::PushLayer(Layer* layer)
	{
		OPTICK_EVENT();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		OPTICK_EVENT();

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Run()
	{
		while (true)
		{
			OPTICK_FRAME("MainThread");

			auto time = std::chrono::high_resolution_clock::now();
			Timestep timestep = std::chrono::duration<float>(time - m_LastFrameTime).count();
			m_LastFrameTime = time;
			
			m_Window->ProcessInput();
			if (m_Window->ShouldClose())
				break;

			{
				OPTICK_EVENT("LayerStack OnUpdate");

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}

			m_ImGuiLayer->Begin();
			{
				OPTICK_EVENT("LayerStack OnImGuiRender");

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->Update();	
		}
	}
}
