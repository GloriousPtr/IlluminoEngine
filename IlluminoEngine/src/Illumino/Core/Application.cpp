#include "ipch.h"
#include "Application.h"

#include <glm/gtx/transform.hpp>

#include "Window.h"
#include "Illumino/Renderer/RenderCommand.h"
#include "Illumino/Renderer/SceneRenderer.h"
#include "Illumino/Renderer/MeshLoader.h"

#include <glm/gtx/quaternion.hpp>

namespace IlluminoEngine
{
	static std::vector<Ref<MeshBuffer>> s_Meshes;

	Application::Application()
	{
		OPTICK_EVENT();

		ILLUMINO_INFO("Application Started");
		m_Window = CreateRef<Window>("Illumino Engine", 1920, 1080);
		RenderCommand::Init();
		SceneRenderer::Init();

		MeshLoader::LoadMesh("Assets/Meshes/primitives/cube.fbx", s_Meshes);
	}

	Application::~Application()
	{
		OPTICK_EVENT();

		SceneRenderer::Shutdown();

		ILLUMINO_INFO("Application Ended");
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			OPTICK_FRAME("MainThread");

			SceneRenderer::BeginScene();

			static uint32_t counter = 0;
			counter++;
			float temp = glm::abs(glm::sin(static_cast<float>(counter) / 64.0f));
			for (size_t i = 0; i < s_Meshes.size(); ++i)
			{
				glm::mat4 t = glm::mat4(1.0f) * glm::translate(glm::vec3(0.0f, 0.0f, -10.0f))
					* glm::rotate(counter * glm::radians(90.0f) / 60, glm::vec3(temp, 1.0 - temp, temp));
				SceneRenderer::SubmitMesh(s_Meshes[i], t);
			}

			SceneRenderer::EndScene();

			m_Window->Update();
		}
	}
}
