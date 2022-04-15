#include "ipch.h"
#include "Application.h"

#include <glm/gtx/transform.hpp>

#include "Window.h"
#include "Illumino/Renderer/RenderCommand.h"
#include "Illumino/Renderer/SceneRenderer.h"

namespace IlluminoEngine
{
	static Ref<MeshBuffer> s_Mesh;

	Application::Application()
	{
		OPTICK_EVENT();

		ILLUMINO_INFO("Application Started");
		m_Window = CreateRef<Window>("Illumino Engine", 1920, 1080);
		RenderCommand::Init();
		SceneRenderer::Init();

		struct Vertex
		{
			float position[3];
			float uv[2];
		};

		Vertex vertices[4] =
		{
			{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } },		// Upper Left
			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } },		// Upper Right
			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },		// Bottom right
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } }		// Bottom left
		};

		uint32_t indices[6] =
		{
			0, 1, 2,
			2, 3, 0
		};

		s_Mesh = MeshBuffer::Create((float*) vertices, indices, sizeof(vertices), sizeof(indices), sizeof(Vertex));
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

			RenderCommand::ClearColor({ 0.042f, 0.042f, 0.042f, 1.0f });
			
			static uint32_t counter = 0;
			counter++;
			float temp = glm::abs(glm::sin(static_cast<float>(counter) / 64.0f));
			glm::mat4 t = glm::mat4(1.0f) * glm::translate(glm::vec3(0.0f, 0.0f, -temp));
			SceneRenderer::SubmitMesh(s_Mesh, t);

			SceneRenderer::EndScene();

			m_Window->Update();
		}
	}
}
