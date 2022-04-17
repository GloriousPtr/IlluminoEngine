#include <IlluminoEngine.h>

#include "EditorLayer.h"

#include <imgui/imgui.h>

namespace IlluminoEngine
{
	static std::vector<Ref<MeshBuffer>> s_Meshes;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		MeshLoader::LoadMesh("Assets/Meshes/primitives/cube.fbx", s_Meshes);
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{

	}

	void EditorLayer::OnImGuiRender()
	{
		bool b = true;
		ImGui::ShowDemoWindow(&b);

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
	}
}
